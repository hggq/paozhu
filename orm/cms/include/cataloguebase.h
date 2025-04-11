#ifndef ORM_CMS_CATALOGUEBASEMATA_H
#define ORM_CMS_CATALOGUEBASEMATA_H
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

struct cataloguebase
{
    struct meta{
     unsigned  int  cid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  level = 0; ///*深度*/
 unsigned  int  parentid = 0; ///*父id[cid tree]*/
 std::string  name = ""; ///*分类名称*/
 char  isview = 0; ///*是否显示*/
 int  sortid = 0; ///*排序*/
 std::string  imgurl = ""; ///*图片*/
 } data;
  
        struct meta_tree{
         unsigned  int  cid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  level = 0; ///*深度*/
 unsigned  int  parentid = 0; ///*父id[cid tree]*/
 std::string  name = ""; ///*分类名称*/
 char  isview = 0; ///*是否显示*/
 int  sortid = 0; ///*排序*/
 std::string  imgurl = ""; ///*图片*/

	std::vector<meta_tree> children;
 };
 std::vector<cataloguebase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<cataloguebase::meta>::iterator begin(){     return record.begin(); }
std::vector<cataloguebase::meta>::iterator end(){     return record.end(); }
std::vector<cataloguebase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<cataloguebase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,8> col_names={"cid","userid","level","parentid","name","isview","sortid","imgurl"};
static constexpr std::array<unsigned char,8> col_types={3,3,3,3,253,1,3,253};
static constexpr std::array<unsigned char,8> col_length={0,0,0,0,64,0,0,254};
static constexpr std::array<unsigned char,8> col_decimals={0,0,0,0,0,0,0,0};
std::string tablename="catalogue";
static constexpr std::string_view modelname="Catalogue";

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
   	 return 0;
break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='l'){ return 7; }
 if(colpospppc=='w'){ return 5; }
   	 break;
 }
 break;
case 'l':
   	 return 2;
break;
case 'n':
   	 return 4;
break;
case 'p':
   	 return 3;
break;
case 's':
   	 return 6;
break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "cid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     cataloguebase::meta metatemp;    
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

        if(data.cid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.cid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.level==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.level);
}
if(data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.parentid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
if(data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isview);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.imgurl)<<"'";
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

        if(insert_data.cid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.cid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.level==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.level);
}
if(insert_data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.parentid);
}
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
if(insert_data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isview);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.imgurl)<<"'";
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


            	if(insert_data[i].cid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].cid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].level);
	}
	if(insert_data[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].parentid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
	if(insert_data[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isview);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].imgurl)<<"'";
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

        if(data.cid==0){
	tempsql<<"`cid`=0";
 }else{ 
	tempsql<<"`cid`="<<std::to_string(data.cid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.level==0){
	tempsql<<",`level`=0";
 }else{ 
	tempsql<<",`level`="<<std::to_string(data.level);
}
if(data.parentid==0){
	tempsql<<",`parentid`=0";
 }else{ 
	tempsql<<",`parentid`="<<std::to_string(data.parentid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
if(data.isview==0){
	tempsql<<",`isview`=0";
 }else{ 
	tempsql<<",`isview`="<<std::to_string(data.isview);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
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
if(data.cid==0){
	tempsql<<"`cid`=0";
 }else{ 
	tempsql<<"`cid`="<<std::to_string(data.cid);
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
if(data.level==0){
	tempsql<<"`level`=0";
 }else{ 
	tempsql<<"`level`="<<std::to_string(data.level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"`parentid`=0";
 }else{ 
	tempsql<<"`parentid`="<<std::to_string(data.parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"`isview`=0";
 }else{ 
	tempsql<<"`isview`="<<std::to_string(data.isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
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
            	if(record[i].cid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].cid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].level);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
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
            	if(record[i].cid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].cid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].level==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].level);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
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
if(data.cid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.cid));
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
if(data.level==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.level));
}
 break;
 case 3:
if(data.parentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.parentid));
}
 break;
 case 4:
	temparray.push_back(data.name);
 break;
 case 5:
if(data.isview==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isview));
}
 break;
 case 6:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 7:
	temparray.push_back(data.imgurl);
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
if(data.cid==0){
	tempsql.insert({"cid","0"});
 }else{ 
	tempsql.insert({"cid",std::to_string(data.cid)});
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
if(data.level==0){
	tempsql.insert({"level","0"});
 }else{ 
	tempsql.insert({"level",std::to_string(data.level)});
}
 break;
 case 3:
if(data.parentid==0){
	tempsql.insert({"parentid","0"});
 }else{ 
	tempsql.insert({"parentid",std::to_string(data.parentid)});
}
 break;
 case 4:
	tempsql.insert({"name",data.name});
 break;
 case 5:
if(data.isview==0){
	tempsql.insert({"isview","0"});
 }else{ 
	tempsql.insert({"isview",std::to_string(data.isview)});
}
 break;
 case 6:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 7:
	tempsql.insert({"imgurl",data.imgurl});
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
if(data.cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(data.cid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.level==0){
	tempsql<<",\"level\":0";
 }else{ 
	tempsql<<",\"level\":"<<std::to_string(data.level);
}
if(data.parentid==0){
	tempsql<<",\"parentid\":0";
 }else{ 
	tempsql<<",\"parentid\":"<<std::to_string(data.parentid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
if(data.isview==0){
	tempsql<<",\"isview\":0";
 }else{ 
	tempsql<<",\"isview\":"<<std::to_string(data.isview);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl);
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
if(data.cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(data.cid);
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
if(data.level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(data.level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(data.parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(data.isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl)<<"\"";
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
        cataloguebase::meta metatemp; 
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
			data.cid=std::stoul(set_value_name);
		}catch (...) { 
			data.cid=0;
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
			data.level=std::stoul(set_value_name);
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 3:
		 try{
			data.parentid=std::stoul(set_value_name);
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 4:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 5:
		 try{
			data.isview=std::stoi(set_value_name);
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 6:
		 try{
			data.sortid=std::stoi(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 7:
		 try{
			data.imgurl.append(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
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
			data.cid=set_value_name;
		}catch (...) { 
			data.cid=0;
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
			data.level=set_value_name;
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 3:
		 try{
			data.parentid=set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 4:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 5:
		 try{
			data.isview=set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 6:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 7:
		 try{
			data.imgurl=std::to_string(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
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
			data.cid=(unsigned int)set_value_name;
		}catch (...) { 
			data.cid=0;
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
			data.level=(unsigned int)set_value_name;
		}catch (...) { 
			data.level=0;
			 }
			break;
		case 3:
		 try{
			data.parentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 4:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 5:
		 try{
			data.isview=(int)set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 6:
		 try{
			data.sortid=(int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 7:
		 try{
			data.imgurl=std::to_string(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
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
if(record[n].cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(record[n].cid);
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
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
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
if(record[n].cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(record[n].cid);
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
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
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
   long long getPK(){  return data.cid; } 
 void setPK(long long val){  data.cid=val;} 
 unsigned  int  getCid(){  return data.cid; } 
 void setCid( unsigned  int  val){  data.cid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getLevel(){  return data.level; } 
 void setLevel( unsigned  int  val){  data.level=val;} 

 unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid( unsigned  int  val){  data.parentid=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 char  getIsview(){  return data.isview; } 
 void setIsview( char  val){  data.isview=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

 std::string  getImgurl(){  return data.imgurl; } 
 std::string & getRefImgurl(){  return std::ref(data.imgurl); } 
 void setImgurl( std::string  &val){  data.imgurl=val;} 
 void setImgurl(std::string_view val){  data.imgurl=val;} 

cataloguebase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
cataloguebase::meta getData(){
 	 return data; 
} 
std::vector<cataloguebase::meta> getRecord(){
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
if(tree_data[n].cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(tree_data[n].cid);
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
if(tree_data[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(tree_data[n].level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
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
if(tree_data[n].cid==0){
	tempsql<<"\"cid\":0";
 }else{ 
	tempsql<<"\"cid\":"<<std::to_string(tree_data[n].cid);
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
if(tree_data[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(tree_data[n].level);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
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
   
    meta_tree treedata_from_record(unsigned int i=0)
    {
        meta_tree temp_obja;
        if(i>=record.size())
        {
           return  temp_obja;   
        }
        	temp_obja.cid=record[i].cid;
	temp_obja.userid=record[i].userid;
	temp_obja.level=record[i].level;
	temp_obja.parentid=record[i].parentid;
	temp_obja.name=record[i].name;
	temp_obja.isview=record[i].isview;
	temp_obja.sortid=record[i].sortid;
	temp_obja.imgurl=record[i].imgurl;

        return  temp_obja;   
    }
    meta_tree treedata_from_data()
    {
        meta_tree temp_obja;

        	temp_obja.cid=data.cid;
	temp_obja.userid=data.userid;
	temp_obja.level=data.level;
	temp_obja.parentid=data.parentid;
	temp_obja.name=data.name;
	temp_obja.isview=data.isview;
	temp_obja.sortid=data.sortid;
	temp_obja.imgurl=data.imgurl;

        return  temp_obja;   
    }      
    meta_tree treedata_from_data(const meta &tempdata)
    {
        meta_tree temp_obja;
        	temp_obja.cid=tempdata.cid;
	temp_obja.userid=tempdata.userid;
	temp_obja.level=tempdata.level;
	temp_obja.parentid=tempdata.parentid;
	temp_obja.name=tempdata.name;
	temp_obja.isview=tempdata.isview;
	temp_obja.sortid=tempdata.sortid;
	temp_obja.imgurl=tempdata.imgurl;

        return  temp_obja;   
    }     
    std::vector<meta_tree> to_tree(unsigned int beginid=0)
    {
       std::vector<meta_tree> temp;
       unsigned int level=0; 
       if(beginid==0)
       {
            for (unsigned int i = 0; i < record.size(); i++)
            {
                if (record[i].parentid == 0)
                {
                    		meta_tree temp_obja;
						temp_obja.cid=record[i].cid;
						temp_obja.userid=record[i].userid;
						temp_obja.level=level;
						temp_obja.parentid=record[i].parentid;
						temp_obja.name=record[i].name;
						temp_obja.isview=record[i].isview;
						temp_obja.sortid=record[i].sortid;
						temp_obja.imgurl=record[i].imgurl;

                    temp.push_back(temp_obja);
                }
            }
       }
       else
       {
           for (unsigned int i = 0; i < record.size(); i++)
            {
                if (record[i].cid == beginid)
                {
                    		meta_tree temp_obja;
						temp_obja.cid=record[i].cid;
						temp_obja.userid=record[i].userid;
						temp_obja.level=level;
						temp_obja.parentid=record[i].parentid;
						temp_obja.name=record[i].name;
						temp_obja.isview=record[i].isview;
						temp_obja.sortid=record[i].sortid;
						temp_obja.imgurl=record[i].imgurl;

                    temp.push_back(temp_obja);
                    break;
                }
            }
       }

       if(temp.size()==0)
       {
          return temp; 
       }
       level+=1;
       for (unsigned int j = 0; j < temp.size(); j++)
       {
         record_to_tree(temp[j].children,temp[j].cid,level);
       }
       return temp; 
    }    
    void record_to_tree(std::vector<meta_tree> &targetdata,long long t_vid,unsigned int level=0)
    {
        for (unsigned int i = 0; i < record.size(); i++)
        {
            if (record[i].parentid== t_vid)
            {
                		meta_tree temp_obja;
						temp_obja.cid=record[i].cid;
						temp_obja.userid=record[i].userid;
						temp_obja.level=level;
						temp_obja.parentid=record[i].parentid;
						temp_obja.name=record[i].name;
						temp_obja.isview=record[i].isview;
						temp_obja.sortid=record[i].sortid;
						temp_obja.imgurl=record[i].imgurl;

                targetdata.push_back(temp_obja);
            }
        }
        level+=1;
        for (unsigned int j = 0; j < targetdata.size(); j++)
        {
         record_to_tree(targetdata[j].children,targetdata[j].cid,level);
        }
    }
    void tree_torecord(const std::vector<meta_tree> &sourcedata,unsigned int level=0)
    {
        for (unsigned int i = 0; i < sourcedata.size(); i++)
        {
		meta temp_obja;
			temp_obja.cid=sourcedata[i].cid;
			temp_obja.userid=sourcedata[i].userid;
			temp_obja.level=level;
			temp_obja.parentid=sourcedata[i].parentid;
			temp_obja.name=sourcedata[i].name;
			temp_obja.isview=sourcedata[i].isview;
			temp_obja.sortid=sourcedata[i].sortid;
			temp_obja.imgurl=sourcedata[i].imgurl;

            record.push_back(temp_obja);
            if(sourcedata[i].children.size()>0)
            {
                tree_torecord(sourcedata[i].children,level+1);
            }
        }
    }      
   

    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="name")
		{
			return data.name;
		}
		 if(key_name=="imgurl")
		{
			return data.imgurl;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="cid")
		{
			return data.cid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="level")
		{
			return data.level;
		}
		 if(key_name=="parentid")
		{
			return data.parentid;
		}
		 if(key_name=="isview")
		{
			return data.isview;
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
 				 a.emplace_back(iter.cid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.level);
				 break;
			case 3: 
 				 a.emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a.emplace_back(iter.isview);
				 break;
			case 6: 
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
 				 return data.cid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.level;
				 break;
			case 3: 
 				 return data.parentid;
				 break;
			case 5: 
 				 return data.isview;
				 break;
			case 6: 
 				 return data.sortid;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] cataloguebase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.cid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.level;
				 break;
			case 3: 
 				 return iter.parentid;
				 break;
			case 5: 
 				 return iter.isview;
				 break;
			case 6: 
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
            T getVal([[maybe_unused]] cataloguebase::meta & iter,std::string keyname)
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

   			case 4: 
 				 return data.name;
				 break;
			case 7: 
 				 return data.imgurl;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] cataloguebase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 4: 
 				 return iter.name;
				 break;
			case 7: 
 				 return iter.imgurl;
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

    			case 4: 
 				 a.emplace_back(iter.name);
					 break;
			case 7: 
 				 a.emplace_back(iter.imgurl);
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
 				 a<<std::to_string(iter.cid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.level);
				 break;
			case 3: 
 				 a<<std::to_string(iter.parentid);
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 5: 
 				 a<<std::to_string(iter.isview);
				 break;
			case 6: 
 				 a<<std::to_string(iter.sortid);
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.imgurl); 
				 }else{
				 a<<iter.imgurl;
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
				 } 
			switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
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
 
       			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
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
 	 ktemp=iter.cid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.level;
	 break;
case 3: 
 	 ktemp=iter.parentid;
	 break;
case 5: 
 	 ktemp=iter.isview;
	 break;
case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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
 				 a.emplace(iter.cid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.level,iter);
				 break;
			case 3: 
 				 a.emplace(iter.parentid,iter);
				 break;
			case 5: 
 				 a.emplace(iter.isview,iter);
				 break;
			case 6: 
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

   			case 4: 
 				 a.emplace(iter.name,iter);
			 break;
			case 7: 
 				 a.emplace(iter.imgurl,iter);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
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
 				 ktemp=iter.cid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.level;
			 break;
			case 3: 
 				 ktemp=iter.parentid;
			 break;
			case 5: 
 				 ktemp=iter.isview;
			 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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

   case 4: 
 	 ktemp=iter.name;
	 break;
case 7: 
 	 ktemp=iter.imgurl;
	 break;
	  }
 switch(vpos){
case 4: 
 	 vtemp=iter.name;
	 break;
case 7: 
 	 vtemp=iter.imgurl;
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
 	 a.emplace_back(iter.cid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.level,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.parentid,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.isview,iter);
	 break;
case 6: 
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

   case 4: 
 	 a.emplace_back(iter.name,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.imgurl,iter);
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
 				 vtemp=iter.sortid;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
				 break;
			  }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 6: 
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.cid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.level;
				 break;
			case 3: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.isview;
				 break;
			case 6: 
 				 vtemp=iter.sortid;
				 break;
			 }

			switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 6: 
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.name;
				 break;
			case 7: 
 				 vtemp=iter.imgurl;
				 break;
			  }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.imgurl);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.imgurl);
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.cid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.level);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 6: 
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
 				 a[iter.cid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.level].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.isview].emplace_back(iter);
				 break;
			case 6: 
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

   			case 4: 
 				 a[iter.name].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.imgurl].emplace_back(iter);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.imgurl].emplace_back(iter);
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

   			case 4: 
 				 ktemp=iter.name;
				 break;
			case 7: 
 				 ktemp=iter.imgurl;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.cid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.cid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 6: 
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
 				 ktemp=iter.cid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.level;
				 break;
			case 3: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.isview;
				 break;
			case 6: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.imgurl].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   