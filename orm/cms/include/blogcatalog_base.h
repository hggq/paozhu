#ifndef ORM_CMS_BLOGCATALOGBASEMATA_H
#define ORM_CMS_BLOGCATALOGBASEMATA_H
/*
*This file is auto create from paozhu_cli
*本文件为自动生成 Sat, 26 Apr 2025 15:58:24 GMT
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

struct blogcatalog_base
{
    struct meta{
     unsigned  int  bid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  dateid = 0; ///*月份聚合*/
 unsigned  int  articlenum = 0; ///*月份文章数量聚合*/
 unsigned  int  languageid = 0; ///*语言版本*/
 } data;
 std::vector<blogcatalog_base::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<blogcatalog_base::meta>::iterator begin(){     return record.begin(); }
std::vector<blogcatalog_base::meta>::iterator end(){     return record.end(); }
std::vector<blogcatalog_base::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<blogcatalog_base::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,5> col_names={"bid","userid","dateid","articlenum","languageid"};
static constexpr std::array<unsigned char,5> col_types={3,3,3,3,3};
static constexpr std::array<unsigned char,5> col_length={0,0,0,0,0};
static constexpr std::array<unsigned char,5> col_decimals={0,0,0,0,0};
std::string tablename="blogcatalog";
static constexpr std::string_view modelname="Blogcatalog";

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
   	 return 3;
break;
case 'b':
   	 return 0;
break;
case 'd':
   	 return 2;
break;
case 'l':
   	 return 4;
break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "bid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     blogcatalog_base::meta metatemp;    
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

        if(data.bid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languageid);
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

        if(insert_data.bid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.bid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.dateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.dateid);
}
if(insert_data.articlenum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.articlenum);
}
if(insert_data.languageid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languageid);
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


            	if(insert_data[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].bid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].dateid);
	}
	if(insert_data[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].articlenum);
	}
	if(insert_data[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languageid);
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

        if(data.bid==0){
	tempsql<<"`bid`=0";
 }else{ 
	tempsql<<"`bid`="<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",`dateid`=0";
 }else{ 
	tempsql<<",`dateid`="<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",`articlenum`=0";
 }else{ 
	tempsql<<",`articlenum`="<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",`languageid`=0";
 }else{ 
	tempsql<<",`languageid`="<<std::to_string(data.languageid);
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
if(data.bid==0){
	tempsql<<"`bid`=0";
 }else{ 
	tempsql<<"`bid`="<<std::to_string(data.bid);
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
if(data.dateid==0){
	tempsql<<"`dateid`=0";
 }else{ 
	tempsql<<"`dateid`="<<std::to_string(data.dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.articlenum==0){
	tempsql<<"`articlenum`=0";
 }else{ 
	tempsql<<"`articlenum`="<<std::to_string(data.articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.languageid==0){
	tempsql<<"`languageid`=0";
 }else{ 
	tempsql<<"`languageid`="<<std::to_string(data.languageid);
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
            	if(record[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].bid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dateid);
	}
	if(record[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].articlenum);
	}
	if(record[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languageid);
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
            	if(record[i].bid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].bid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].dateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].dateid);
	}
	if(record[i].articlenum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].articlenum);
	}
	if(record[i].languageid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languageid);
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
if(data.bid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bid));
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
if(data.dateid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dateid));
}
 break;
 case 3:
if(data.articlenum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.articlenum));
}
 break;
 case 4:
if(data.languageid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languageid));
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
if(data.bid==0){
	tempsql.insert({"bid","0"});
 }else{ 
	tempsql.insert({"bid",std::to_string(data.bid)});
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
if(data.dateid==0){
	tempsql.insert({"dateid","0"});
 }else{ 
	tempsql.insert({"dateid",std::to_string(data.dateid)});
}
 break;
 case 3:
if(data.articlenum==0){
	tempsql.insert({"articlenum","0"});
 }else{ 
	tempsql.insert({"articlenum",std::to_string(data.articlenum)});
}
 break;
 case 4:
if(data.languageid==0){
	tempsql.insert({"languageid","0"});
 }else{ 
	tempsql.insert({"languageid",std::to_string(data.languageid)});
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
if(data.bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(data.bid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.dateid==0){
	tempsql<<",\"dateid\":0";
 }else{ 
	tempsql<<",\"dateid\":"<<std::to_string(data.dateid);
}
if(data.articlenum==0){
	tempsql<<",\"articlenum\":0";
 }else{ 
	tempsql<<",\"articlenum\":"<<std::to_string(data.articlenum);
}
if(data.languageid==0){
	tempsql<<",\"languageid\":0";
 }else{ 
	tempsql<<",\"languageid\":"<<std::to_string(data.languageid);
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
if(data.bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(data.bid);
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
if(data.dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(data.dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(data.articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(data.languageid);
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
        blogcatalog_base::meta metatemp; 
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
			data.bid=std::stoul(set_value_name);
		}catch (...) { 
			data.bid=0;
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
			data.dateid=std::stoul(set_value_name);
		}catch (...) { 
			data.dateid=0;
			 }
			break;
		case 3:
		 try{
			data.articlenum=std::stoul(set_value_name);
		}catch (...) { 
			data.articlenum=0;
			 }
			break;
		case 4:
		 try{
			data.languageid=std::stoul(set_value_name);
		}catch (...) { 
			data.languageid=0;
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
			data.bid=set_value_name;
		}catch (...) { 
			data.bid=0;
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
			data.dateid=set_value_name;
		}catch (...) { 
			data.dateid=0;
			 }
			break;
		case 3:
		 try{
			data.articlenum=set_value_name;
		}catch (...) { 
			data.articlenum=0;
			 }
			break;
		case 4:
		 try{
			data.languageid=set_value_name;
		}catch (...) { 
			data.languageid=0;
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
			data.bid=(unsigned int)set_value_name;
		}catch (...) { 
			data.bid=0;
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
			data.dateid=(unsigned int)set_value_name;
		}catch (...) { 
			data.dateid=0;
			 }
			break;
		case 3:
		 try{
			data.articlenum=(unsigned int)set_value_name;
		}catch (...) { 
			data.articlenum=0;
			 }
			break;
		case 4:
		 try{
			data.languageid=(unsigned int)set_value_name;
		}catch (...) { 
			data.languageid=0;
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
if(record[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(record[n].bid);
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
if(record[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(record[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(record[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(record[n].languageid);
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
if(record[n].bid==0){
	tempsql<<"\"bid\":0";
 }else{ 
	tempsql<<"\"bid\":"<<std::to_string(record[n].bid);
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
if(record[n].dateid==0){
	tempsql<<"\"dateid\":0";
 }else{ 
	tempsql<<"\"dateid\":"<<std::to_string(record[n].dateid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].articlenum==0){
	tempsql<<"\"articlenum\":0";
 }else{ 
	tempsql<<"\"articlenum\":"<<std::to_string(record[n].articlenum);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].languageid==0){
	tempsql<<"\"languageid\":0";
 }else{ 
	tempsql<<"\"languageid\":"<<std::to_string(record[n].languageid);
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
   long long getPK(){  return data.bid; } 
 void setPK(long long val){  data.bid=val;} 
 unsigned  int  getBid(){  return data.bid; } 
 void setBid( unsigned  int  val){  data.bid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getDateid(){  return data.dateid; } 
 void setDateid( unsigned  int  val){  data.dateid=val;} 

 unsigned  int  getArticlenum(){  return data.articlenum; } 
 void setArticlenum( unsigned  int  val){  data.articlenum=val;} 

 unsigned  int  getLanguageid(){  return data.languageid; } 
 void setLanguageid( unsigned  int  val){  data.languageid=val;} 

blogcatalog_base::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
blogcatalog_base::meta getData(){
 	 return data; 
} 
std::vector<blogcatalog_base::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="bid")
		{
			return data.bid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="dateid")
		{
			return data.dateid;
		}
		 if(key_name=="articlenum")
		{
			return data.articlenum;
		}
		 if(key_name=="languageid")
		{
			return data.languageid;
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
 				 a.emplace_back(iter.bid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a.emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a.emplace_back(iter.languageid);
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
 				 return data.bid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.dateid;
				 break;
			case 3: 
 				 return data.articlenum;
				 break;
			case 4: 
 				 return data.languageid;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] blogcatalog_base::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.bid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.dateid;
				 break;
			case 3: 
 				 return iter.articlenum;
				 break;
			case 4: 
 				 return iter.languageid;
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
            T getVal([[maybe_unused]] blogcatalog_base::meta & iter,std::string keyname)
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

   
                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] blogcatalog_base::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   
                }
                
    
                 
                return "";
            }  
     
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
            std::vector<std::string> getCol([[maybe_unused]] std::string keyname)
            {
                std::vector<std::string> a;

           

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
 				 a<<std::to_string(iter.bid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.dateid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.articlenum);
				 break;
			case 4: 
 				 a<<std::to_string(iter.languageid);
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
         
            return a;
        } 
    

        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<std::string,U> getCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
                std::map<std::string,U> a;
         
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
 	 ktemp=iter.bid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.dateid;
	 break;
case 3: 
 	 ktemp=iter.articlenum;
	 break;
case 4: 
 	 ktemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
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

   			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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
 				 a.emplace(iter.bid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.dateid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.articlenum,iter);
				 break;
			case 4: 
 				 a.emplace(iter.languageid,iter);
				 break;

                }
            }       
     
            return a;
        }     
    
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
        std::map<std::string,meta> getmapRows([[maybe_unused]] std::string keyname)
        {
            std::map<std::string,meta> a;

    

        return a;
    }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
        std::vector<std::pair<std::string,U>> getvecCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::vector<std::pair<std::string,U>> a;
   

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
 				 ktemp=iter.bid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.dateid;
			 break;
			case 3: 
 				 ktemp=iter.articlenum;
			 break;
			case 4: 
 				 ktemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }
 			switch(vpos){

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

   			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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
 	 a.emplace_back(iter.bid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.dateid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.articlenum,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.languageid,iter);
	 break;

                }
            }       
    
        return a;
    }
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
        std::vector<std::pair<std::string,meta>> getvecRows([[maybe_unused]] std::string keyname)
        {
            std::vector<std::pair<std::string,meta>> a;
      

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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.languageid);
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
				 break;
			 }

			 switch(dpos){

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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			 }

			 switch(vpos){
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			 }

			 switch(vpos){
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.languageid);
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }

			 switch(vpos){
			  }

			 switch(dpos){

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

   			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
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

   			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.languageid);
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

   			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.bid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.dateid;
				 break;
			case 3: 
 				 vtemp=iter.articlenum;
				 break;
			case 4: 
 				 vtemp=iter.languageid;
				 break;
			 }

			switch(dpos){

            }
         }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
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

   			  }

			 switch(vpos){
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.languageid);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname,[[maybe_unused]] std::string dataname)
        {
            std::map<T,std::map<U,std::vector<D>>> a;
   
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;

   
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::vector<U>> a;
   
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

   			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.languageid);
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			 }

			 switch(vpos){

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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.bid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.dateid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.articlenum);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.languageid);
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
 				 a[iter.bid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.dateid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.articlenum].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.languageid].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows([[maybe_unused]] std::string keyname)
        {
            std::map<T,std::vector<meta>> a;
   
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows([[maybe_unused]] std::string keyname,[[maybe_unused]] std::string valname)
        {
            std::map<T,std::map<U,std::vector<meta>>> a;
   
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

   	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.bid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.dateid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.articlenum].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.languageid].emplace_back(iter);
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.bid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.dateid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.articlenum].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.languageid].emplace_back(iter);
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
 				 ktemp=iter.bid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.dateid;
				 break;
			case 3: 
 				 ktemp=iter.articlenum;
				 break;
			case 4: 
 				 ktemp=iter.languageid;
				 break;
			  }

			 switch(vpos){

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   