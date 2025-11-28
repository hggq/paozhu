#ifndef ORM_CMS_TOPICSETBASEMATA_H
#define ORM_CMS_TOPICSETBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Fri, 28 Nov 2025 12:11:33 GMT
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

struct topicset_base
{
    struct meta{
     unsigned  int  topicsetid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  topicid = 0; ///**/
 unsigned  int  linktopicid = 0; ///**/
 unsigned  char  linkrownum = 0; ///**/
 unsigned  char  sidetype = 0; ///*showtype*/
 std::string  sidename = ""; ///**/
 std::string  txtcontent = ""; ///**/
 unsigned  int  sort = 0; ///**/
 } data;
 std::vector<topicset_base::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<topicset_base::meta>::iterator begin(){     return record.begin(); }
std::vector<topicset_base::meta>::iterator end(){     return record.end(); }
std::vector<topicset_base::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<topicset_base::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,9> col_names={"topicsetid","userid","topicid","linktopicid","linkrownum","sidetype","sidename","txtcontent","sort"};
static constexpr std::array<unsigned char,9> col_types={3,3,3,3,1,1,253,252,3};
static constexpr std::array<unsigned char,9> col_length={0,0,0,0,0,0,80,0,0};
static constexpr std::array<unsigned char,9> col_decimals={0,0,0,0,0,0,0,0,0};
std::string tablename="topicset";
static constexpr std::string_view modelname="Topicset";

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


         case 'l':
 switch(coln.size()){  
case 10:
   	 return 4;
break;
case 11:
   	 return 3;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 4:
   	 return 8;
break;
case 8:
 if(coln.size()>4&&(coln[4]=='n'||coln[4]=='N')){ return 6; }
 if(coln.size()>4&&(coln[4]=='t'||coln[4]=='T')){ return 5; }
   	 break;
 }
 break;
case 't':
 switch(coln.size()){  
case 7:
   	 return 2;
break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 0; }
 if(colpospppc=='t'){ return 7; }
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
       return "topicsetid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     topicset_base::meta metatemp;    
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

        if(data.topicsetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sidetype);
}
tempsql<<",'"<<stringaddslash(data.sidename)<<"'";
tempsql<<",'"<<stringaddslash(data.txtcontent)<<"'";
if(data.sort==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sort);
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

        if(insert_data.topicsetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.topicsetid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.topicid);
}
if(insert_data.linktopicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.linktopicid);
}
if(insert_data.linkrownum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.linkrownum);
}
if(insert_data.sidetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sidetype);
}
tempsql<<",'"<<stringaddslash(insert_data.sidename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.txtcontent)<<"'";
if(insert_data.sort==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sort);
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


            	if(insert_data[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].topicsetid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].linktopicid);
	}
	if(insert_data[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].linkrownum);
	}
	if(insert_data[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sidetype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sidename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].txtcontent)<<"'";
	if(insert_data[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sort);
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

        if(data.topicsetid==0){
	tempsql<<"`topicsetid`=0";
 }else{ 
	tempsql<<"`topicsetid`="<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",`topicid`=0";
 }else{ 
	tempsql<<",`topicid`="<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",`linktopicid`=0";
 }else{ 
	tempsql<<",`linktopicid`="<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",`linkrownum`=0";
 }else{ 
	tempsql<<",`linkrownum`="<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",`sidetype`=0";
 }else{ 
	tempsql<<",`sidetype`="<<std::to_string(data.sidetype);
}
tempsql<<",`sidename`='"<<stringaddslash(data.sidename)<<"'";
tempsql<<",`txtcontent`='"<<stringaddslash(data.txtcontent)<<"'";
if(data.sort==0){
	tempsql<<",`sort`=0";
 }else{ 
	tempsql<<",`sort`="<<std::to_string(data.sort);
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
if(data.topicsetid==0){
	tempsql<<"`topicsetid`=0";
 }else{ 
	tempsql<<"`topicsetid`="<<std::to_string(data.topicsetid);
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
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.linktopicid==0){
	tempsql<<"`linktopicid`=0";
 }else{ 
	tempsql<<"`linktopicid`="<<std::to_string(data.linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.linkrownum==0){
	tempsql<<"`linkrownum`=0";
 }else{ 
	tempsql<<"`linkrownum`="<<std::to_string(data.linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sidetype==0){
	tempsql<<"`sidetype`=0";
 }else{ 
	tempsql<<"`sidetype`="<<std::to_string(data.sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sidename`='"<<stringaddslash(data.sidename)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`txtcontent`='"<<stringaddslash(data.txtcontent)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sort==0){
	tempsql<<"`sort`=0";
 }else{ 
	tempsql<<"`sort`="<<std::to_string(data.sort);
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
            	if(record[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicsetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linktopicid);
	}
	if(record[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linkrownum);
	}
	if(record[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sidetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sidename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].txtcontent)<<"'";
	if(record[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sort);
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
            	if(record[i].topicsetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicsetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].linktopicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linktopicid);
	}
	if(record[i].linkrownum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].linkrownum);
	}
	if(record[i].sidetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sidetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sidename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].txtcontent)<<"'";
	if(record[i].sort==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sort);
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
if(data.topicsetid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicsetid));
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
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 3:
if(data.linktopicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.linktopicid));
}
 break;
 case 4:
if(data.linkrownum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.linkrownum));
}
 break;
 case 5:
if(data.sidetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sidetype));
}
 break;
 case 6:
	temparray.push_back(data.sidename);
 break;
 case 7:
	temparray.push_back(data.txtcontent);
 break;
 case 8:
if(data.sort==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sort));
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
if(data.topicsetid==0){
	tempsql.insert({"topicsetid","0"});
 }else{ 
	tempsql.insert({"topicsetid",std::to_string(data.topicsetid)});
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
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 3:
if(data.linktopicid==0){
	tempsql.insert({"linktopicid","0"});
 }else{ 
	tempsql.insert({"linktopicid",std::to_string(data.linktopicid)});
}
 break;
 case 4:
if(data.linkrownum==0){
	tempsql.insert({"linkrownum","0"});
 }else{ 
	tempsql.insert({"linkrownum",std::to_string(data.linkrownum)});
}
 break;
 case 5:
if(data.sidetype==0){
	tempsql.insert({"sidetype","0"});
 }else{ 
	tempsql.insert({"sidetype",std::to_string(data.sidetype)});
}
 break;
 case 6:
	tempsql.insert({"sidename",data.sidename});
 break;
 case 7:
	tempsql.insert({"txtcontent",data.txtcontent});
 break;
 case 8:
if(data.sort==0){
	tempsql.insert({"sort","0"});
 }else{ 
	tempsql.insert({"sort",std::to_string(data.sort)});
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
if(data.topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(data.topicsetid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",\"topicid\":0";
 }else{ 
	tempsql<<",\"topicid\":"<<std::to_string(data.topicid);
}
if(data.linktopicid==0){
	tempsql<<",\"linktopicid\":0";
 }else{ 
	tempsql<<",\"linktopicid\":"<<std::to_string(data.linktopicid);
}
if(data.linkrownum==0){
	tempsql<<",\"linkrownum\":0";
 }else{ 
	tempsql<<",\"linkrownum\":"<<std::to_string(data.linkrownum);
}
if(data.sidetype==0){
	tempsql<<",\"sidetype\":0";
 }else{ 
	tempsql<<",\"sidetype\":"<<std::to_string(data.sidetype);
}
tempsql<<",\"sidename\":\""<<http::utf8_to_jsonstring(data.sidename);
tempsql<<"\"";
tempsql<<",\"txtcontent\":\""<<http::utf8_to_jsonstring(data.txtcontent);
tempsql<<"\"";
if(data.sort==0){
	tempsql<<",\"sort\":0";
 }else{ 
	tempsql<<",\"sort\":"<<std::to_string(data.sort);
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
if(data.topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(data.topicsetid);
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
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(data.linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(data.linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(data.sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(data.sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(data.txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(data.sort);
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
        topicset_base::meta metatemp; 
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
			data.topicsetid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=std::stoul(set_value_name);
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=std::stoi(set_value_name);
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=std::stoi(set_value_name);
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename.append(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent.append(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=std::stoul(set_value_name);
		}catch (...) { 
			data.sort=0;
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
			data.topicsetid=set_value_name;
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=set_value_name;
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=set_value_name;
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=set_value_name;
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename=std::to_string(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=set_value_name;
		}catch (...) { 
			data.sort=0;
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
			data.topicsetid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicsetid=0;
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
			data.topicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.linktopicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.linktopicid=0;
			 }
			break;
		case 4:
		 try{
			data.linkrownum=(int)set_value_name;
		}catch (...) { 
			data.linkrownum=0;
			 }
			break;
		case 5:
		 try{
			data.sidetype=(int)set_value_name;
		}catch (...) { 
			data.sidetype=0;
			 }
			break;
		case 6:
		 try{
			data.sidename=std::to_string(set_value_name);
		}catch (...) { 
			data.sidename.clear();
			 }
			break;
		case 7:
		 try{
			data.txtcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.txtcontent.clear();
			 }
			break;
		case 8:
		 try{
			data.sort=(unsigned int)set_value_name;
		}catch (...) { 
			data.sort=0;
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
if(record[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(record[n].topicsetid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(record[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(record[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(record[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(record[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(record[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(record[n].sort);
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
if(record[n].topicsetid==0){
	tempsql<<"\"topicsetid\":0";
 }else{ 
	tempsql<<"\"topicsetid\":"<<std::to_string(record[n].topicsetid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].linktopicid==0){
	tempsql<<"\"linktopicid\":0";
 }else{ 
	tempsql<<"\"linktopicid\":"<<std::to_string(record[n].linktopicid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].linkrownum==0){
	tempsql<<"\"linkrownum\":0";
 }else{ 
	tempsql<<"\"linkrownum\":"<<std::to_string(record[n].linkrownum);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].sidetype==0){
	tempsql<<"\"sidetype\":0";
 }else{ 
	tempsql<<"\"sidetype\":"<<std::to_string(record[n].sidetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sidename\":\""<<http::utf8_to_jsonstring(record[n].sidename)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"txtcontent\":\""<<http::utf8_to_jsonstring(record[n].txtcontent)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sort==0){
	tempsql<<"\"sort\":0";
 }else{ 
	tempsql<<"\"sort\":"<<std::to_string(record[n].sort);
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
   long long getPK(){  return data.topicsetid; } 
 void setPK(long long val){  data.topicsetid=val;} 
 unsigned  int  getTopicsetid(){  return data.topicsetid; } 
 void setTopicsetid( unsigned  int  val){  data.topicsetid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getLinktopicid(){  return data.linktopicid; } 
 void setLinktopicid( unsigned  int  val){  data.linktopicid=val;} 

 unsigned  char  getLinkrownum(){  return data.linkrownum; } 
 void setLinkrownum( unsigned  char  val){  data.linkrownum=val;} 

 unsigned  char  getSidetype(){  return data.sidetype; } 
 void setSidetype( unsigned  char  val){  data.sidetype=val;} 

 std::string  getSidename(){  return data.sidename; } 
 std::string & getRefSidename(){  return std::ref(data.sidename); } 
 void setSidename( std::string  &val){  data.sidename=val;} 
 void setSidename(std::string_view val){  data.sidename=val;} 

 std::string  getTxtcontent(){  return data.txtcontent; } 
 std::string & getRefTxtcontent(){  return std::ref(data.txtcontent); } 
 void setTxtcontent( std::string  &val){  data.txtcontent=val;} 
 void setTxtcontent(std::string_view val){  data.txtcontent=val;} 

 unsigned  int  getSort(){  return data.sort; } 
 void setSort( unsigned  int  val){  data.sort=val;} 

topicset_base::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
topicset_base::meta getData(){
 	 return data; 
} 
std::vector<topicset_base::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="sidename")
		{
			return data.sidename;
		}
		 if(key_name=="txtcontent")
		{
			return data.txtcontent;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="topicsetid")
		{
			return data.topicsetid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="topicid")
		{
			return data.topicid;
		}
		 if(key_name=="linktopicid")
		{
			return data.linktopicid;
		}
		 if(key_name=="linkrownum")
		{
			return data.linkrownum;
		}
		 if(key_name=="sidetype")
		{
			return data.sidetype;
		}
		 if(key_name=="sort")
		{
			return data.sort;
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
 				 a.emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a.emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a.emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a.emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a.emplace_back(iter.sort);
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
 				 return data.topicsetid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.topicid;
				 break;
			case 3: 
 				 return data.linktopicid;
				 break;
			case 4: 
 				 return data.linkrownum;
				 break;
			case 5: 
 				 return data.sidetype;
				 break;
			case 8: 
 				 return data.sort;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] topicset_base::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.topicsetid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.topicid;
				 break;
			case 3: 
 				 return iter.linktopicid;
				 break;
			case 4: 
 				 return iter.linkrownum;
				 break;
			case 5: 
 				 return iter.sidetype;
				 break;
			case 8: 
 				 return iter.sort;
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
            T getVal([[maybe_unused]] topicset_base::meta & iter,std::string keyname)
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

   			case 6: 
 				 return data.sidename;
				 break;
			case 7: 
 				 return data.txtcontent;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] topicset_base::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 6: 
 				 return iter.sidename;
				 break;
			case 7: 
 				 return iter.txtcontent;
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

    			case 6: 
 				 a.emplace_back(iter.sidename);
					 break;
			case 7: 
 				 a.emplace_back(iter.txtcontent);
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
 				 a<<std::to_string(iter.topicsetid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.topicid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.linktopicid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.linkrownum);
				 break;
			case 5: 
 				 a<<std::to_string(iter.sidetype);
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sidename); 
				 }else{
				 a<<iter.sidename;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.txtcontent); 
				 }else{
				 a<<iter.txtcontent;
				 }
				 break;
			case 8: 
 				 a<<std::to_string(iter.sort);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
				 } 
			switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
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
 
       			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
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
 	 ktemp=iter.topicsetid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.topicid;
	 break;
case 3: 
 	 ktemp=iter.linktopicid;
	 break;
case 4: 
 	 ktemp=iter.linkrownum;
	 break;
case 5: 
 	 ktemp=iter.sidetype;
	 break;
case 8: 
 	 ktemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }
 			switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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
 				 a.emplace(iter.topicsetid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.topicid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.linktopicid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.linkrownum,iter);
				 break;
			case 5: 
 				 a.emplace(iter.sidetype,iter);
				 break;
			case 8: 
 				 a.emplace(iter.sort,iter);
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

   			case 6: 
 				 a.emplace(iter.sidename,iter);
			 break;
			case 7: 
 				 a.emplace(iter.txtcontent,iter);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
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
 				 ktemp=iter.topicsetid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.topicid;
			 break;
			case 3: 
 				 ktemp=iter.linktopicid;
			 break;
			case 4: 
 				 ktemp=iter.linkrownum;
			 break;
			case 5: 
 				 ktemp=iter.sidetype;
			 break;
			case 8: 
 				 ktemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }
 			switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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

   case 6: 
 	 ktemp=iter.sidename;
	 break;
case 7: 
 	 ktemp=iter.txtcontent;
	 break;
	  }
 switch(vpos){
case 6: 
 	 vtemp=iter.sidename;
	 break;
case 7: 
 	 vtemp=iter.txtcontent;
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
 	 a.emplace_back(iter.topicsetid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.topicid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.linktopicid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.linkrownum,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.sidetype,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.sort,iter);
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

   case 6: 
 	 a.emplace_back(iter.sidename,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.txtcontent,iter);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sort);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
				 break;
			 }

			 switch(dpos){
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.txtcontent);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			 }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			 }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sort);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
				 break;
			  }

			 switch(dpos){
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.txtcontent);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sort);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.topicsetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.linktopicid;
				 break;
			case 4: 
 				 vtemp=iter.linkrownum;
				 break;
			case 5: 
 				 vtemp=iter.sidetype;
				 break;
			case 8: 
 				 vtemp=iter.sort;
				 break;
			 }

			switch(dpos){
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.txtcontent);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			 }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sort);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }

			 switch(vpos){
			case 6: 
 				 vtemp=iter.sidename;
				 break;
			case 7: 
 				 vtemp=iter.txtcontent;
				 break;
			  }

			 switch(dpos){
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.txtcontent);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }

			 switch(vpos){
			case 6: 
 				 a[ktemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.txtcontent);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.sort);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			 }

			 switch(vpos){
			case 6: 
 				 a[ktemp].emplace_back(iter.sidename);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.txtcontent);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.topicsetid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.linktopicid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.linkrownum);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sidetype);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.sort);
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
 				 a[iter.topicsetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.linktopicid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.linkrownum].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.sidetype].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.sort].emplace_back(iter);
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

   			case 6: 
 				 a[iter.sidename].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.txtcontent].emplace_back(iter);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
			 }

			 switch(vpos){
			case 6: 
 				 a[ktemp][iter.sidename].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.txtcontent].emplace_back(iter);
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

   			case 6: 
 				 ktemp=iter.sidename;
				 break;
			case 7: 
 				 ktemp=iter.txtcontent;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.topicsetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.linktopicid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.linkrownum].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sidetype].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.sort].emplace_back(iter);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.topicsetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.linktopicid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.linkrownum].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sidetype].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.sort].emplace_back(iter);
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
 				 ktemp=iter.topicsetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.linktopicid;
				 break;
			case 4: 
 				 ktemp=iter.linkrownum;
				 break;
			case 5: 
 				 ktemp=iter.sidetype;
				 break;
			case 8: 
 				 ktemp=iter.sort;
				 break;
			  }

			 switch(vpos){
			case 6: 
 				 a[ktemp][iter.sidename].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.txtcontent].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   