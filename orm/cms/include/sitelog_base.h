#ifndef ORM_CMS_SITELOGBASEMATA_H
#define ORM_CMS_SITELOGBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Thu, 07 Aug 2025 02:38:20 GMT
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

struct sitelog_base
{
    struct meta{
     unsigned  int  logid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  memberid = 0; ///**/
 unsigned  int  ipport = 0; ///**/
 unsigned  char  httpv = 0; ///*https http2*/
 std::string  ipaddress = ""; ///**/
 std::string  visittime = ""; ///**/
 std::string  useragent = ""; ///**/
 std::string  referer = ""; ///**/
 std::string  cururl = ""; ///**/
 std::string  address = ""; ///**/
 std::string  hostname = ""; ///**/
 std::string  derefererurl = ""; ///**/
 std::string  deurl = ""; ///**/
 } data;
 std::vector<sitelog_base::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<sitelog_base::meta>::iterator begin(){     return record.begin(); }
std::vector<sitelog_base::meta>::iterator end(){     return record.end(); }
std::vector<sitelog_base::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<sitelog_base::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,14> col_names={"logid","userid","memberid","ipport","httpv","ipaddress","visittime","useragent","referer","cururl","address","hostname","derefererurl","deurl"};
static constexpr std::array<unsigned char,14> col_types={3,3,3,3,1,253,253,253,253,253,253,253,253,253};
static constexpr std::array<unsigned char,14> col_length={0,0,0,0,0,60,30,0,0,0,120,60,0,0};
static constexpr std::array<unsigned char,14> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="sitelog";
static constexpr std::string_view modelname="Sitelog";

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
   	 return 10;
break;
case 'c':
   	 return 9;
break;
case 'd':
 switch(coln.size()){  
case 5:
   	 return 13;
break;
case 12:
   	 return 12;
break;
 }
 break;
case 'h':
 switch(coln.size()){  
case 5:
   	 return 4;
break;
case 8:
   	 return 11;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 3;
break;
case 9:
   	 return 5;
break;
 }
 break;
case 'l':
   	 return 0;
break;
case 'm':
   	 return 2;
break;
case 'r':
   	 return 8;
break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 9:
   	 return 7;
break;
 }
 break;
case 'v':
   	 return 6;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "logid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     sitelog_base::meta metatemp;    
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

        if(data.logid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.httpv);
}
tempsql<<",'"<<stringaddslash(data.ipaddress)<<"'";
tempsql<<",'"<<stringaddslash(data.visittime)<<"'";
tempsql<<",'"<<stringaddslash(data.useragent)<<"'";
tempsql<<",'"<<stringaddslash(data.referer)<<"'";
tempsql<<",'"<<stringaddslash(data.cururl)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.hostname)<<"'";
tempsql<<",'"<<stringaddslash(data.derefererurl)<<"'";
tempsql<<",'"<<stringaddslash(data.deurl)<<"'";
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

        if(insert_data.logid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.logid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.memberid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.memberid);
}
if(insert_data.ipport==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ipport);
}
if(insert_data.httpv==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.httpv);
}
tempsql<<",'"<<stringaddslash(insert_data.ipaddress)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.visittime)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.useragent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.referer)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.cururl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.hostname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.derefererurl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.deurl)<<"'";
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


            	if(insert_data[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].logid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].memberid);
	}
	if(insert_data[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ipport);
	}
	if(insert_data[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].httpv);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].ipaddress)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].visittime)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].useragent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].referer)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].cururl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].hostname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].derefererurl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].deurl)<<"'";
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

        if(data.logid==0){
	tempsql<<"`logid`=0";
 }else{ 
	tempsql<<"`logid`="<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",`memberid`=0";
 }else{ 
	tempsql<<",`memberid`="<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",`ipport`=0";
 }else{ 
	tempsql<<",`ipport`="<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",`httpv`=0";
 }else{ 
	tempsql<<",`httpv`="<<std::to_string(data.httpv);
}
tempsql<<",`ipaddress`='"<<stringaddslash(data.ipaddress)<<"'";
tempsql<<",`visittime`='"<<stringaddslash(data.visittime)<<"'";
tempsql<<",`useragent`='"<<stringaddslash(data.useragent)<<"'";
tempsql<<",`referer`='"<<stringaddslash(data.referer)<<"'";
tempsql<<",`cururl`='"<<stringaddslash(data.cururl)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`hostname`='"<<stringaddslash(data.hostname)<<"'";
tempsql<<",`derefererurl`='"<<stringaddslash(data.derefererurl)<<"'";
tempsql<<",`deurl`='"<<stringaddslash(data.deurl)<<"'";
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
if(data.logid==0){
	tempsql<<"`logid`=0";
 }else{ 
	tempsql<<"`logid`="<<std::to_string(data.logid);
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
if(data.memberid==0){
	tempsql<<"`memberid`=0";
 }else{ 
	tempsql<<"`memberid`="<<std::to_string(data.memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.ipport==0){
	tempsql<<"`ipport`=0";
 }else{ 
	tempsql<<"`ipport`="<<std::to_string(data.ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.httpv==0){
	tempsql<<"`httpv`=0";
 }else{ 
	tempsql<<"`httpv`="<<std::to_string(data.httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`ipaddress`='"<<stringaddslash(data.ipaddress)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`visittime`='"<<stringaddslash(data.visittime)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`useragent`='"<<stringaddslash(data.useragent)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`referer`='"<<stringaddslash(data.referer)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`cururl`='"<<stringaddslash(data.cururl)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`hostname`='"<<stringaddslash(data.hostname)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`derefererurl`='"<<stringaddslash(data.derefererurl)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`deurl`='"<<stringaddslash(data.deurl)<<"'";
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
            	if(record[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].logid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].memberid);
	}
	if(record[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ipport);
	}
	if(record[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].httpv);
	}
	tempsql<<",'"<<stringaddslash(record[i].ipaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].visittime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].useragent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].referer)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].cururl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].hostname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].derefererurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].deurl)<<"'";
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
            	if(record[i].logid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].logid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].memberid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].memberid);
	}
	if(record[i].ipport==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ipport);
	}
	if(record[i].httpv==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].httpv);
	}
	tempsql<<",'"<<stringaddslash(record[i].ipaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].visittime)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].useragent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].referer)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].cururl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].hostname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].derefererurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].deurl)<<"'";
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
if(data.logid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.logid));
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
if(data.memberid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.memberid));
}
 break;
 case 3:
if(data.ipport==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ipport));
}
 break;
 case 4:
if(data.httpv==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.httpv));
}
 break;
 case 5:
	temparray.push_back(data.ipaddress);
 break;
 case 6:
	temparray.push_back(data.visittime);
 break;
 case 7:
	temparray.push_back(data.useragent);
 break;
 case 8:
	temparray.push_back(data.referer);
 break;
 case 9:
	temparray.push_back(data.cururl);
 break;
 case 10:
	temparray.push_back(data.address);
 break;
 case 11:
	temparray.push_back(data.hostname);
 break;
 case 12:
	temparray.push_back(data.derefererurl);
 break;
 case 13:
	temparray.push_back(data.deurl);
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
if(data.logid==0){
	tempsql.insert({"logid","0"});
 }else{ 
	tempsql.insert({"logid",std::to_string(data.logid)});
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
if(data.memberid==0){
	tempsql.insert({"memberid","0"});
 }else{ 
	tempsql.insert({"memberid",std::to_string(data.memberid)});
}
 break;
 case 3:
if(data.ipport==0){
	tempsql.insert({"ipport","0"});
 }else{ 
	tempsql.insert({"ipport",std::to_string(data.ipport)});
}
 break;
 case 4:
if(data.httpv==0){
	tempsql.insert({"httpv","0"});
 }else{ 
	tempsql.insert({"httpv",std::to_string(data.httpv)});
}
 break;
 case 5:
	tempsql.insert({"ipaddress",data.ipaddress});
 break;
 case 6:
	tempsql.insert({"visittime",data.visittime});
 break;
 case 7:
	tempsql.insert({"useragent",data.useragent});
 break;
 case 8:
	tempsql.insert({"referer",data.referer});
 break;
 case 9:
	tempsql.insert({"cururl",data.cururl});
 break;
 case 10:
	tempsql.insert({"address",data.address});
 break;
 case 11:
	tempsql.insert({"hostname",data.hostname});
 break;
 case 12:
	tempsql.insert({"derefererurl",data.derefererurl});
 break;
 case 13:
	tempsql.insert({"deurl",data.deurl});
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
if(data.logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(data.logid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.memberid==0){
	tempsql<<",\"memberid\":0";
 }else{ 
	tempsql<<",\"memberid\":"<<std::to_string(data.memberid);
}
if(data.ipport==0){
	tempsql<<",\"ipport\":0";
 }else{ 
	tempsql<<",\"ipport\":"<<std::to_string(data.ipport);
}
if(data.httpv==0){
	tempsql<<",\"httpv\":0";
 }else{ 
	tempsql<<",\"httpv\":"<<std::to_string(data.httpv);
}
tempsql<<",\"ipaddress\":\""<<http::utf8_to_jsonstring(data.ipaddress);
tempsql<<"\"";
tempsql<<",\"visittime\":\""<<http::utf8_to_jsonstring(data.visittime);
tempsql<<"\"";
tempsql<<",\"useragent\":\""<<http::utf8_to_jsonstring(data.useragent);
tempsql<<"\"";
tempsql<<",\"referer\":\""<<http::utf8_to_jsonstring(data.referer);
tempsql<<"\"";
tempsql<<",\"cururl\":\""<<http::utf8_to_jsonstring(data.cururl);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"hostname\":\""<<http::utf8_to_jsonstring(data.hostname);
tempsql<<"\"";
tempsql<<",\"derefererurl\":\""<<http::utf8_to_jsonstring(data.derefererurl);
tempsql<<"\"";
tempsql<<",\"deurl\":\""<<http::utf8_to_jsonstring(data.deurl);
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
if(data.logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(data.logid);
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
if(data.memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(data.memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(data.ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(data.httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(data.ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(data.visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(data.useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(data.referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(data.cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(data.hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(data.derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(data.deurl)<<"\"";
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
        sitelog_base::meta metatemp; 
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
			data.logid=std::stoul(set_value_name);
		}catch (...) { 
			data.logid=0;
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
			data.memberid=std::stoul(set_value_name);
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=std::stoul(set_value_name);
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=std::stoi(set_value_name);
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress.append(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime.append(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent.append(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer.append(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl.append(set_value_name);
		}catch (...) { 
			data.cururl.clear();
			 }
			break;
		case 10:
		 try{
			data.address.append(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.hostname.append(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl.append(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl.append(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
			data.logid=set_value_name;
		}catch (...) { 
			data.logid=0;
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
			data.memberid=set_value_name;
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=set_value_name;
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=set_value_name;
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime=std::to_string(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent=std::to_string(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer=std::to_string(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl=std::to_string(set_value_name);
		}catch (...) { 
			data.cururl.clear();
			 }
			break;
		case 10:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.hostname=std::to_string(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl=std::to_string(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl=std::to_string(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
			data.logid=(unsigned int)set_value_name;
		}catch (...) { 
			data.logid=0;
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
			data.memberid=(unsigned int)set_value_name;
		}catch (...) { 
			data.memberid=0;
			 }
			break;
		case 3:
		 try{
			data.ipport=(unsigned int)set_value_name;
		}catch (...) { 
			data.ipport=0;
			 }
			break;
		case 4:
		 try{
			data.httpv=(int)set_value_name;
		}catch (...) { 
			data.httpv=0;
			 }
			break;
		case 5:
		 try{
			data.ipaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.ipaddress.clear();
			 }
			break;
		case 6:
		 try{
			data.visittime=std::to_string(set_value_name);
		}catch (...) { 
			data.visittime.clear();
			 }
			break;
		case 7:
		 try{
			data.useragent=std::to_string(set_value_name);
		}catch (...) { 
			data.useragent.clear();
			 }
			break;
		case 8:
		 try{
			data.referer=std::to_string(set_value_name);
		}catch (...) { 
			data.referer.clear();
			 }
			break;
		case 9:
		 try{
			data.cururl=std::to_string(set_value_name);
		}catch (...) { 
			data.cururl.clear();
			 }
			break;
		case 10:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 11:
		 try{
			data.hostname=std::to_string(set_value_name);
		}catch (...) { 
			data.hostname.clear();
			 }
			break;
		case 12:
		 try{
			data.derefererurl=std::to_string(set_value_name);
		}catch (...) { 
			data.derefererurl.clear();
			 }
			break;
		case 13:
		 try{
			data.deurl=std::to_string(set_value_name);
		}catch (...) { 
			data.deurl.clear();
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
if(record[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(record[n].logid);
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
if(record[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(record[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(record[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(record[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(record[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(record[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(record[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(record[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(record[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(record[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(record[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(record[n].deurl)<<"\"";
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
if(record[n].logid==0){
	tempsql<<"\"logid\":0";
 }else{ 
	tempsql<<"\"logid\":"<<std::to_string(record[n].logid);
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
if(record[n].memberid==0){
	tempsql<<"\"memberid\":0";
 }else{ 
	tempsql<<"\"memberid\":"<<std::to_string(record[n].memberid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].ipport==0){
	tempsql<<"\"ipport\":0";
 }else{ 
	tempsql<<"\"ipport\":"<<std::to_string(record[n].ipport);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].httpv==0){
	tempsql<<"\"httpv\":0";
 }else{ 
	tempsql<<"\"httpv\":"<<std::to_string(record[n].httpv);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"ipaddress\":\""<<http::utf8_to_jsonstring(record[n].ipaddress)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"visittime\":\""<<http::utf8_to_jsonstring(record[n].visittime)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"useragent\":\""<<http::utf8_to_jsonstring(record[n].useragent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"referer\":\""<<http::utf8_to_jsonstring(record[n].referer)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"cururl\":\""<<http::utf8_to_jsonstring(record[n].cururl)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"hostname\":\""<<http::utf8_to_jsonstring(record[n].hostname)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"derefererurl\":\""<<http::utf8_to_jsonstring(record[n].derefererurl)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"deurl\":\""<<http::utf8_to_jsonstring(record[n].deurl)<<"\"";
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
   long long getPK(){  return data.logid; } 
 void setPK(long long val){  data.logid=val;} 
 unsigned  int  getLogid(){  return data.logid; } 
 void setLogid( unsigned  int  val){  data.logid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getMemberid(){  return data.memberid; } 
 void setMemberid( unsigned  int  val){  data.memberid=val;} 

 unsigned  int  getIpport(){  return data.ipport; } 
 void setIpport( unsigned  int  val){  data.ipport=val;} 

 unsigned  char  getHttpv(){  return data.httpv; } 
 void setHttpv( unsigned  char  val){  data.httpv=val;} 

 std::string  getIpaddress(){  return data.ipaddress; } 
 std::string & getRefIpaddress(){  return std::ref(data.ipaddress); } 
 void setIpaddress( std::string  &val){  data.ipaddress=val;} 
 void setIpaddress(std::string_view val){  data.ipaddress=val;} 

 std::string  getVisittime(){  return data.visittime; } 
 std::string & getRefVisittime(){  return std::ref(data.visittime); } 
 void setVisittime( std::string  &val){  data.visittime=val;} 
 void setVisittime(std::string_view val){  data.visittime=val;} 

 std::string  getUseragent(){  return data.useragent; } 
 std::string & getRefUseragent(){  return std::ref(data.useragent); } 
 void setUseragent( std::string  &val){  data.useragent=val;} 
 void setUseragent(std::string_view val){  data.useragent=val;} 

 std::string  getReferer(){  return data.referer; } 
 std::string & getRefReferer(){  return std::ref(data.referer); } 
 void setReferer( std::string  &val){  data.referer=val;} 
 void setReferer(std::string_view val){  data.referer=val;} 

 std::string  getCururl(){  return data.cururl; } 
 std::string & getRefCururl(){  return std::ref(data.cururl); } 
 void setCururl( std::string  &val){  data.cururl=val;} 
 void setCururl(std::string_view val){  data.cururl=val;} 

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

 std::string  getHostname(){  return data.hostname; } 
 std::string & getRefHostname(){  return std::ref(data.hostname); } 
 void setHostname( std::string  &val){  data.hostname=val;} 
 void setHostname(std::string_view val){  data.hostname=val;} 

 std::string  getDerefererurl(){  return data.derefererurl; } 
 std::string & getRefDerefererurl(){  return std::ref(data.derefererurl); } 
 void setDerefererurl( std::string  &val){  data.derefererurl=val;} 
 void setDerefererurl(std::string_view val){  data.derefererurl=val;} 

 std::string  getDeurl(){  return data.deurl; } 
 std::string & getRefDeurl(){  return std::ref(data.deurl); } 
 void setDeurl( std::string  &val){  data.deurl=val;} 
 void setDeurl(std::string_view val){  data.deurl=val;} 

sitelog_base::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
sitelog_base::meta getData(){
 	 return data; 
} 
std::vector<sitelog_base::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="ipaddress")
		{
			return data.ipaddress;
		}
		 if(key_name=="visittime")
		{
			return data.visittime;
		}
		 if(key_name=="useragent")
		{
			return data.useragent;
		}
		 if(key_name=="referer")
		{
			return data.referer;
		}
		 if(key_name=="cururl")
		{
			return data.cururl;
		}
		 if(key_name=="address")
		{
			return data.address;
		}
		 if(key_name=="hostname")
		{
			return data.hostname;
		}
		 if(key_name=="derefererurl")
		{
			return data.derefererurl;
		}
		 if(key_name=="deurl")
		{
			return data.deurl;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="logid")
		{
			return data.logid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="memberid")
		{
			return data.memberid;
		}
		 if(key_name=="ipport")
		{
			return data.ipport;
		}
		 if(key_name=="httpv")
		{
			return data.httpv;
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
 				 a.emplace_back(iter.logid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a.emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a.emplace_back(iter.httpv);
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
 				 return data.logid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.memberid;
				 break;
			case 3: 
 				 return data.ipport;
				 break;
			case 4: 
 				 return data.httpv;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] sitelog_base::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.logid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.memberid;
				 break;
			case 3: 
 				 return iter.ipport;
				 break;
			case 4: 
 				 return iter.httpv;
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
            T getVal([[maybe_unused]] sitelog_base::meta & iter,std::string keyname)
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

   			case 5: 
 				 return data.ipaddress;
				 break;
			case 6: 
 				 return data.visittime;
				 break;
			case 7: 
 				 return data.useragent;
				 break;
			case 8: 
 				 return data.referer;
				 break;
			case 9: 
 				 return data.cururl;
				 break;
			case 10: 
 				 return data.address;
				 break;
			case 11: 
 				 return data.hostname;
				 break;
			case 12: 
 				 return data.derefererurl;
				 break;
			case 13: 
 				 return data.deurl;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] sitelog_base::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 5: 
 				 return iter.ipaddress;
				 break;
			case 6: 
 				 return iter.visittime;
				 break;
			case 7: 
 				 return iter.useragent;
				 break;
			case 8: 
 				 return iter.referer;
				 break;
			case 9: 
 				 return iter.cururl;
				 break;
			case 10: 
 				 return iter.address;
				 break;
			case 11: 
 				 return iter.hostname;
				 break;
			case 12: 
 				 return iter.derefererurl;
				 break;
			case 13: 
 				 return iter.deurl;
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
 				 a.emplace_back(iter.ipaddress);
					 break;
			case 6: 
 				 a.emplace_back(iter.visittime);
					 break;
			case 7: 
 				 a.emplace_back(iter.useragent);
					 break;
			case 8: 
 				 a.emplace_back(iter.referer);
					 break;
			case 9: 
 				 a.emplace_back(iter.cururl);
					 break;
			case 10: 
 				 a.emplace_back(iter.address);
					 break;
			case 11: 
 				 a.emplace_back(iter.hostname);
					 break;
			case 12: 
 				 a.emplace_back(iter.derefererurl);
					 break;
			case 13: 
 				 a.emplace_back(iter.deurl);
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
 				 a<<std::to_string(iter.logid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.memberid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.ipport);
				 break;
			case 4: 
 				 a<<std::to_string(iter.httpv);
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.ipaddress); 
				 }else{
				 a<<iter.ipaddress;
				 }
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.visittime); 
				 }else{
				 a<<iter.visittime;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.useragent); 
				 }else{
				 a<<iter.useragent;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.referer); 
				 }else{
				 a<<iter.referer;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.cururl); 
				 }else{
				 a<<iter.cururl;
				 }
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.address); 
				 }else{
				 a<<iter.address;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.hostname); 
				 }else{
				 a<<iter.hostname;
				 }
				 break;
			case 12: 
 				 if(isyinhao){ a<<jsonaddslash(iter.derefererurl); 
				 }else{
				 a<<iter.derefererurl;
				 }
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.deurl); 
				 }else{
				 a<<iter.deurl;
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

   			case 5: 
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
				 } 
			switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
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
 	 ktemp=iter.logid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.memberid;
	 break;
case 3: 
 	 ktemp=iter.ipport;
	 break;
case 4: 
 	 ktemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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
 				 a.emplace(iter.logid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.memberid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.ipport,iter);
				 break;
			case 4: 
 				 a.emplace(iter.httpv,iter);
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
 				 a.emplace(iter.ipaddress,iter);
			 break;
			case 6: 
 				 a.emplace(iter.visittime,iter);
			 break;
			case 7: 
 				 a.emplace(iter.useragent,iter);
			 break;
			case 8: 
 				 a.emplace(iter.referer,iter);
			 break;
			case 9: 
 				 a.emplace(iter.cururl,iter);
			 break;
			case 10: 
 				 a.emplace(iter.address,iter);
			 break;
			case 11: 
 				 a.emplace(iter.hostname,iter);
			 break;
			case 12: 
 				 a.emplace(iter.derefererurl,iter);
			 break;
			case 13: 
 				 a.emplace(iter.deurl,iter);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
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
 				 ktemp=iter.logid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.memberid;
			 break;
			case 3: 
 				 ktemp=iter.ipport;
			 break;
			case 4: 
 				 ktemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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
 	 ktemp=iter.ipaddress;
	 break;
case 6: 
 	 ktemp=iter.visittime;
	 break;
case 7: 
 	 ktemp=iter.useragent;
	 break;
case 8: 
 	 ktemp=iter.referer;
	 break;
case 9: 
 	 ktemp=iter.cururl;
	 break;
case 10: 
 	 ktemp=iter.address;
	 break;
case 11: 
 	 ktemp=iter.hostname;
	 break;
case 12: 
 	 ktemp=iter.derefererurl;
	 break;
case 13: 
 	 ktemp=iter.deurl;
	 break;
	  }
 switch(vpos){
case 5: 
 	 vtemp=iter.ipaddress;
	 break;
case 6: 
 	 vtemp=iter.visittime;
	 break;
case 7: 
 	 vtemp=iter.useragent;
	 break;
case 8: 
 	 vtemp=iter.referer;
	 break;
case 9: 
 	 vtemp=iter.cururl;
	 break;
case 10: 
 	 vtemp=iter.address;
	 break;
case 11: 
 	 vtemp=iter.hostname;
	 break;
case 12: 
 	 vtemp=iter.derefererurl;
	 break;
case 13: 
 	 vtemp=iter.deurl;
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
 	 a.emplace_back(iter.logid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.memberid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.ipport,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.httpv,iter);
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
 	 a.emplace_back(iter.ipaddress,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.visittime,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.useragent,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.referer,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.cururl,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.address,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.hostname,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.derefererurl,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.deurl,iter);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.httpv);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
				 break;
			 }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.httpv);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
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

   			case 5: 
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.httpv);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.logid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.memberid;
				 break;
			case 3: 
 				 vtemp=iter.ipport;
				 break;
			case 4: 
 				 vtemp=iter.httpv;
				 break;
			 }

			switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
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

   			case 5: 
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.httpv);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.ipaddress;
				 break;
			case 6: 
 				 vtemp=iter.visittime;
				 break;
			case 7: 
 				 vtemp=iter.useragent;
				 break;
			case 8: 
 				 vtemp=iter.referer;
				 break;
			case 9: 
 				 vtemp=iter.cururl;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.hostname;
				 break;
			case 12: 
 				 vtemp=iter.derefererurl;
				 break;
			case 13: 
 				 vtemp=iter.deurl;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
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

   			case 5: 
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.httpv);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.ipaddress);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.visittime);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.useragent);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.referer);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.cururl);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.hostname);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.derefererurl);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.deurl);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.logid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.memberid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.ipport);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.httpv);
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
 				 a[iter.logid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.memberid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.ipport].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.httpv].emplace_back(iter);
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
 				 a[iter.ipaddress].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.visittime].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.useragent].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.referer].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.cururl].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.hostname].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.derefererurl].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.deurl].emplace_back(iter);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.ipaddress].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.visittime].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.useragent].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.referer].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.cururl].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.hostname].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.derefererurl].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.deurl].emplace_back(iter);
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
 				 ktemp=iter.ipaddress;
				 break;
			case 6: 
 				 ktemp=iter.visittime;
				 break;
			case 7: 
 				 ktemp=iter.useragent;
				 break;
			case 8: 
 				 ktemp=iter.referer;
				 break;
			case 9: 
 				 ktemp=iter.cururl;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.hostname;
				 break;
			case 12: 
 				 ktemp=iter.derefererurl;
				 break;
			case 13: 
 				 ktemp=iter.deurl;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.logid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.memberid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.ipport].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.httpv].emplace_back(iter);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.logid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.memberid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.ipport].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.httpv].emplace_back(iter);
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
 				 ktemp=iter.logid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.memberid;
				 break;
			case 3: 
 				 ktemp=iter.ipport;
				 break;
			case 4: 
 				 ktemp=iter.httpv;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.ipaddress].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.visittime].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.useragent].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.referer].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.cururl].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.hostname].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.derefererurl].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.deurl].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   