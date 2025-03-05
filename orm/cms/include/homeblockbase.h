#ifndef ORM_CMS_HOMEBLOCKBASEMATA_H
#define ORM_CMS_HOMEBLOCKBASEMATA_H
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

struct homeblockbase
{
    struct meta{
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
 } data;
 std::vector<homeblockbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<homeblockbase::meta>::iterator begin(){     return record.begin(); }
std::vector<homeblockbase::meta>::iterator end(){     return record.end(); }
std::vector<homeblockbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<homeblockbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,12> col_names={"hbid","userid","title","content","jsonconfig","viewtype","gettype","rownum","width","height","strlength","sortid"};
static constexpr std::array<unsigned char,12> col_types={3,3,253,252,252,1,1,3,3,3,3,3};
static constexpr std::array<unsigned char,12> col_length={0,0,120,0,0,0,0,0,0,0,0,0};
static constexpr std::array<unsigned char,12> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0};
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
     homeblockbase::meta metatemp;    
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
            for(jj=0;jj<col_names.size();jj++){
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
        homeblockbase::meta metatemp; 
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

homeblockbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
homeblockbase::meta getData(){
 	 return data; 
} 
std::vector<homeblockbase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="content")
		{
			return data.content;
		}
		 if(key_name=="jsonconfig")
		{
			return data.jsonconfig;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="hbid")
		{
			return data.hbid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="viewtype")
		{
			return data.viewtype;
		}
		 if(key_name=="gettype")
		{
			return data.gettype;
		}
		 if(key_name=="rownum")
		{
			return data.rownum;
		}
		 if(key_name=="width")
		{
			return data.width;
		}
		 if(key_name=="height")
		{
			return data.height;
		}
		 if(key_name=="strlength")
		{
			return data.strlength;
		}
		 if(key_name=="sortid")
		{
			return data.sortid;
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
 				 a.emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 5: 
 				 a.emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a.emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a.emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a.emplace_back(iter.width);
				 break;
			case 9: 
 				 a.emplace_back(iter.height);
				 break;
			case 10: 
 				 a.emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a.emplace_back(iter.sortid);
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
 				 return data.hbid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 5: 
 				 return data.viewtype;
				 break;
			case 6: 
 				 return data.gettype;
				 break;
			case 7: 
 				 return data.rownum;
				 break;
			case 8: 
 				 return data.width;
				 break;
			case 9: 
 				 return data.height;
				 break;
			case 10: 
 				 return data.strlength;
				 break;
			case 11: 
 				 return data.sortid;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] homeblockbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.hbid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 5: 
 				 return iter.viewtype;
				 break;
			case 6: 
 				 return iter.gettype;
				 break;
			case 7: 
 				 return iter.rownum;
				 break;
			case 8: 
 				 return iter.width;
				 break;
			case 9: 
 				 return iter.height;
				 break;
			case 10: 
 				 return iter.strlength;
				 break;
			case 11: 
 				 return iter.sortid;
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
            T getVal([[maybe_unused]] homeblockbase::meta & iter,std::string keyname)
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
 				 return data.title;
				 break;
			case 3: 
 				 return data.content;
				 break;
			case 4: 
 				 return data.jsonconfig;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] homeblockbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 2: 
 				 return iter.title;
				 break;
			case 3: 
 				 return iter.content;
				 break;
			case 4: 
 				 return iter.jsonconfig;
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
 				 a.emplace_back(iter.title);
					 break;
			case 3: 
 				 a.emplace_back(iter.content);
					 break;
			case 4: 
 				 a.emplace_back(iter.jsonconfig);
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
 				 a<<std::to_string(iter.hbid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 3: 
 				 if(isyinhao){ a<<jsonaddslash(iter.content); 
				 }else{
				 a<<iter.content;
				 }
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.jsonconfig); 
				 }else{
				 a<<iter.jsonconfig;
				 }
				 break;
			case 5: 
 				 a<<std::to_string(iter.viewtype);
				 break;
			case 6: 
 				 a<<std::to_string(iter.gettype);
				 break;
			case 7: 
 				 a<<std::to_string(iter.rownum);
				 break;
			case 8: 
 				 a<<std::to_string(iter.width);
				 break;
			case 9: 
 				 a<<std::to_string(iter.height);
				 break;
			case 10: 
 				 a<<std::to_string(iter.strlength);
				 break;
			case 11: 
 				 a<<std::to_string(iter.sortid);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
				 } 
			switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
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
 	 ktemp=iter.hbid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 5: 
 	 ktemp=iter.viewtype;
	 break;
case 6: 
 	 ktemp=iter.gettype;
	 break;
case 7: 
 	 ktemp=iter.rownum;
	 break;
case 8: 
 	 ktemp=iter.width;
	 break;
case 9: 
 	 ktemp=iter.height;
	 break;
case 10: 
 	 ktemp=iter.strlength;
	 break;
case 11: 
 	 ktemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 				 a.emplace(iter.hbid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 5: 
 				 a.emplace(iter.viewtype,iter);
				 break;
			case 6: 
 				 a.emplace(iter.gettype,iter);
				 break;
			case 7: 
 				 a.emplace(iter.rownum,iter);
				 break;
			case 8: 
 				 a.emplace(iter.width,iter);
				 break;
			case 9: 
 				 a.emplace(iter.height,iter);
				 break;
			case 10: 
 				 a.emplace(iter.strlength,iter);
				 break;
			case 11: 
 				 a.emplace(iter.sortid,iter);
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
 				 a.emplace(iter.title,iter);
			 break;
			case 3: 
 				 a.emplace(iter.content,iter);
			 break;
			case 4: 
 				 a.emplace(iter.jsonconfig,iter);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
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
 				 ktemp=iter.hbid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 5: 
 				 ktemp=iter.viewtype;
			 break;
			case 6: 
 				 ktemp=iter.gettype;
			 break;
			case 7: 
 				 ktemp=iter.rownum;
			 break;
			case 8: 
 				 ktemp=iter.width;
			 break;
			case 9: 
 				 ktemp=iter.height;
			 break;
			case 10: 
 				 ktemp=iter.strlength;
			 break;
			case 11: 
 				 ktemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 	 ktemp=iter.title;
	 break;
case 3: 
 	 ktemp=iter.content;
	 break;
case 4: 
 	 ktemp=iter.jsonconfig;
	 break;
	  }
 switch(vpos){
case 2: 
 	 vtemp=iter.title;
	 break;
case 3: 
 	 vtemp=iter.content;
	 break;
case 4: 
 	 vtemp=iter.jsonconfig;
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
 	 a.emplace_back(iter.hbid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.viewtype,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.gettype,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.rownum,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.width,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.height,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.strlength,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.sortid,iter);
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
 	 a.emplace_back(iter.title,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.content,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.jsonconfig,iter);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
				 break;
			  }

			 switch(dpos){
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.hbid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 5: 
 				 vtemp=iter.viewtype;
				 break;
			case 6: 
 				 vtemp=iter.gettype;
				 break;
			case 7: 
 				 vtemp=iter.rownum;
				 break;
			case 8: 
 				 vtemp=iter.width;
				 break;
			case 9: 
 				 vtemp=iter.height;
				 break;
			case 10: 
 				 vtemp=iter.strlength;
				 break;
			case 11: 
 				 vtemp=iter.sortid;
				 break;
			 }

			switch(dpos){
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 vtemp=iter.title;
				 break;
			case 3: 
 				 vtemp=iter.content;
				 break;
			case 4: 
 				 vtemp=iter.jsonconfig;
				 break;
			  }

			 switch(dpos){
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.sortid);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.jsonconfig);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.hbid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.viewtype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.gettype);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.rownum);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.width);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.height);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.strlength);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.sortid);
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
 				 a[iter.hbid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.viewtype].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.gettype].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.rownum].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.width].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.height].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.strlength].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.sortid].emplace_back(iter);
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
 				 a[iter.title].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.content].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.jsonconfig].emplace_back(iter);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
			 }

			 switch(vpos){
			case 2: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.jsonconfig].emplace_back(iter);
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
 				 ktemp=iter.title;
				 break;
			case 3: 
 				 ktemp=iter.content;
				 break;
			case 4: 
 				 ktemp=iter.jsonconfig;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.hbid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.viewtype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.gettype].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.rownum].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.width].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.height].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.strlength].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.hbid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.viewtype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.gettype].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.rownum].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.width].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.height].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.strlength].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
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
 				 ktemp=iter.hbid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 5: 
 				 ktemp=iter.viewtype;
				 break;
			case 6: 
 				 ktemp=iter.gettype;
				 break;
			case 7: 
 				 ktemp=iter.rownum;
				 break;
			case 8: 
 				 ktemp=iter.width;
				 break;
			case 9: 
 				 ktemp=iter.height;
				 break;
			case 10: 
 				 ktemp=iter.strlength;
				 break;
			case 11: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 2: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.jsonconfig].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   