#ifndef ORM_CMS_USERBASEMATA_H
#define ORM_CMS_USERBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Mon, 09 Jan 2023 15:30:05 GMT
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
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mysql.h"
namespace orm { 
     namespace cms { 

struct userbase
{
    struct meta{
    unsigned  int userid= 0; //
 std::string name=""; //
 std::string password=""; //
 char isopen=0; //
 int level= 0; //
 } data;
 std::vector<userbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<unsigned char> _keypos{0x00};
MYSQL_ROW _row;
std::vector<userbase::meta>::iterator begin(){     return record.begin(); }
std::vector<userbase::meta>::iterator end(){     return record.end(); }
std::vector<userbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<userbase::meta>::const_iterator end() const{     return record.end(); }
const std::array<std::string,5> colnames={"userid","name","password","isopen","level"};
const std::array<unsigned char,5> colnamestype= {3,253,253,1,3};
std::string tablename="user";
std::string modelname="User";

	  unsigned char findcolpos(std::string coln){
		    unsigned char  bi=coln[0];
         

	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         case 'i':
   	 return 3;
break;
case 'l':
   	 return 4;
break;
case 'n':
   	 return 1;
break;
case 'p':
   	 return 2;
break;
case 'u':
   	 return 0;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "userid";
}

      void _setColnamevalue(){
          userbase::meta metatemp;   
         for(unsigned char i=0;i<_keypos.size();i++){
                 switch(_keypos[i]){
        	case 0:
		 try{
			metatemp.userid=std::stoul(_row[0]);
		}catch (...) { 
			metatemp.userid=0;
			 }
			break;
	case 1:
		 try{
			metatemp.name.append(_row[1]);
		}catch (...) { 
			metatemp.name.clear();
			 }
			break;
	case 2:
		 try{
			metatemp.password.append(_row[2]);
		}catch (...) { 
			metatemp.password.clear();
			 }
			break;
	case 3:
		 try{
			metatemp.isopen=std::stoi(_row[3]);
		}catch (...) { 
			metatemp.isopen=0;
			 }
			break;
	case 4:
		 try{
			metatemp.level=std::stoi(_row[4]);
		}catch (...) { 
			metatemp.level=0;
			 }
			break;
	default:
		 { }
			

                 }

                 if(i>210){
                     break;
                 }
          }
          data=metatemp;
          record.emplace_back(metatemp);   
   } 
         void _addnewrowvalue(){
           userbase::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        	case 0:
		 try{
			metatemp.userid=std::stoul(_row[0]);
		}catch (...) { 
			metatemp.userid=0;
			 }
			break;
	case 1:
		 try{
			metatemp.name.append(_row[1]);
		}catch (...) { 
			metatemp.name.clear();
			 }
			break;
	case 2:
		 try{
			metatemp.password.append(_row[2]);
		}catch (...) { 
			metatemp.password.clear();
			 }
			break;
	case 3:
		 try{
			metatemp.isopen=std::stoi(_row[3]);
		}catch (...) { 
			metatemp.isopen=0;
			 }
			break;
	case 4:
		 try{
			metatemp.level=std::stoi(_row[4]);
		}catch (...) { 
			metatemp.level=0;
			 }
			break;
	default:
		 { }
			

                  }
                 if(i>210){
                     break;
                 }
          }
           record.emplace_back(std::move(metatemp)); 
   } 

  inline  std::string stringaddslash(std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='\''){
                temp.append("\\'");
                continue;
            }else if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            temp.push_back(content[i]);
        }
        return temp;
   }  
  inline  std::string jsonaddslash(std::string &content){
        std::string temp;
        for(unsigned int i=0;i<content.size();i++){
            if(content[i]=='"'){
                temp.append("\\\"");
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
                    for(;j<colnames.size();j++){
                            if(j>0){
                                tempsql<<"`,`";
                            }else{
                                tempsql<<"`";
                            }
                            tempsql<<colnames[j];
                    }
                    if(j>0){
                        tempsql<<"`";
                    }
            tempsql<<") VALUES (";

        if(data.userid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.password)<<"'";
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
tempsql<<")";

     
       return tempsql.str();
   }     
    std::string _makeupdatesql(std::string fileld){
       //int j=0;
            std::ostringstream tempsql;
                 tempsql<<"UPDATE ";
                 tempsql<<tablename;
                 tempsql<<" SET ";

            bool isall=false;
            if(fileld.empty()){
                isall=true;
            }
            if(isall){

        if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`password`='"<<stringaddslash(data.password)<<"'";
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
 }else{ 

     
  unsigned int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
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
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){

         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
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
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.level==0){
	tempsql<<"`level`=0";
 }else{ 
	tempsql<<"`level`="<<std::to_string(data.level);
}
 break;

     
                  default:
                                ;
                     }
                 }   

            }        

        return tempsql.str();
   } 
   std::string data_tojson(){
       std::ostringstream tempsql;

        tempsql<<"{";
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"password\":\""<<http::utf8_to_jsonstring(data.password);
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                 tempsql<<"{";
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
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
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(data.level);
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
        userbase::meta metatemp; 
        data=metatemp;
        unsigned int json_offset=0;
        bool isarray=false;
        std::vector<std::string> list_content;
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
                                    json_key_name=http::jsonstring_to_utf8(json_content.substr(json_offset),temp_offset);
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
                                        json_value_name=http::jsonstring_to_utf8(json_content.substr(json_offset),temp_offset);
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
                                 json_key_name=http::jsonstring_to_utf8(json_content.substr(json_offset),temp_offset);
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
                                    json_value_name=http::jsonstring_to_utf8(json_content.substr(json_offset),temp_offset);
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
			data.userid=std::stoul(set_value_name);
		}catch (...) { 
			data.userid=0;
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
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 4:
		 try{
			data.level=std::stoi(set_value_name);
		}catch (...) { 
			data.level=0;
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
			data.userid=set_value_name;
		}catch (...) { 
			data.userid=0;
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
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 4:
		 try{
			data.level=set_value_name;
		}catch (...) { 
			data.level=0;
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
			data.userid=(unsigned int)set_value_name;
		}catch (...) { 
			data.userid=0;
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
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 4:
		 try{
			data.level=(int)set_value_name;
		}catch (...) { 
			data.level=0;
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
                     for(jj=0;jj<colnames.size();jj++){
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
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
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
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
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
                     for(jj=0;jj<colnames.size();jj++){
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
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
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
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].level==0){
	tempsql<<"\"level\":0";
 }else{ 
	tempsql<<"\"level\":"<<std::to_string(record[n].level);
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
   long long getPK(){  return data.userid; } 
 void setPK(long long val){  data.userid=val;} 
unsigned  int  getUserid(){  return data.userid; } 
 void setUserid(unsigned  int  val){  data.userid=val;} 

std::string getName(){  return data.name; } 
std::string& getRefName(){  return std::ref(data.name); } 
 void setName(std::string &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

std::string getPassword(){  return data.password; } 
std::string& getRefPassword(){  return std::ref(data.password); } 
 void setPassword(std::string &val){  data.password=val;} 
 void setPassword(std::string_view val){  data.password=val;} 

 int  getIsopen(){  return data.isopen; } 
 void setIsopen( int  val){  data.isopen=val;} 

 int  getLevel(){  return data.level; } 
 void setLevel( int  val){  data.level=val;} 

userbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
userbase::meta getData(){
 	 return data; 
} 
std::vector<userbase::meta> getRecord(){
 	 return record; 
} 

            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >  
            std::vector<T> getCol(std::string keyname)
            {
                std::vector<T> a;
                unsigned char kpos;
                kpos=findcolpos(keyname);
   
                for(auto &iter:record)
                {
                    switch(kpos)
                    {
   			case 0: 
 				 a.emplace_back(iter.userid);
				 break;
			case 3: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a.emplace_back(iter.level);
				 break;

                    }
                }
    
                return a;
            }
    
            template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
			std::vector<T> getCol(std::string keyname)
			{
				std::vector<T> a;
				unsigned char kpos;
				kpos = findcolpos(keyname);

                return a;
            }
    
            template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
            T getVal(std::string keyname)
            {
                unsigned char kpos;
                kpos=findcolpos(keyname);
                      
                    switch(kpos)
                    {

   			case 0: 
 				 return data.userid;
				 break;
			case 3: 
 				 return data.isopen;
				 break;
			case 4: 
 				 return data.level;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal(userbase::meta & iter,std::string keyname)
        {
            unsigned char kpos;
            kpos=findcolpos(keyname);
             
            switch(kpos)
            {
   			case 0: 
 				 return iter.userid;
				 break;
			case 3: 
 				 return iter.isopen;
				 break;
			case 4: 
 				 return iter.level;
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
            T getVal(userbase::meta & iter,std::string keyname)
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

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal(userbase::meta & iter,std::string keyname)
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

                }
                return "";
            }  
     
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
            std::vector<std::string> getCol(std::string keyname)
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
					}
				}

        return a;
    }
     
        std::string getstrCol(std::string keyname,bool isyinhao=false)
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
 				 a<<std::to_string(iter.userid);
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
 				 a<<std::to_string(iter.isopen);
				 break;
			case 4: 
 				 a<<std::to_string(iter.level);
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
    std::map<std::string,std::string> getCols(std::string keyname,std::string valname)
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
				 } 
			 switch(vpos){ 
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;

                }
                a.emplace(ktemp,vtemp);
            }       

        
            return a;
        } 
    

        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<std::string,U> getCols(std::string keyname,std::string valname)
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
			 } 
 		    switch(vpos){ 

                    }
                    a.emplace(ktemp,vtemp);
                }       
        
            return a;
        } 
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>       
        std::map<T,U> getCols(std::string keyname,std::string valname)
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
 	 ktemp=iter.userid;
	 break;
case 3: 
 	 ktemp=iter.isopen;
	 break;
case 4: 
 	 ktemp=iter.level;
	 break;
	 } 
 		   switch(vpos){ 

                }
                a.emplace(ktemp,vtemp);
            }       
     
        return a;
    }  
            template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>      
            std::map<T,std::string> getCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }
 			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;

                    }
                    a.emplace(ktemp,vtemp);
                } 
         
                return a;
            }     
        
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>       
        std::map<std::string,U> getCols(std::string keyname,std::string valname)
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
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;

                }
                a.emplace(ktemp,vtemp);
            }       
      
        return a;
    }  
    
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>   
        std::map<T,U> getCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;

                }
                a.emplace(ktemp,vtemp);
            }       
    
            return a;
        }   
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >         
        std::map<T,meta> getmapRows(std::string keyname)
        {
            std::map<T,meta> a;
            unsigned char kpos;
            kpos=findcolpos(keyname);
                
            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 4: 
 				 a.emplace(iter.level,iter);
				 break;

                }
            }       
     
            return a;
        }     
    
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
        std::map<std::string,meta> getmapRows(std::string keyname)
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

                }
                //a.emplace(ktemp,iter);
            }       
    

        return a;
    }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
        std::vector<std::pair<std::string,U>> getvecCols(std::string keyname,std::string valname)
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
	 		 }
 			switch(vpos){

                   }

                 a.emplace_back(ktemp,vtemp);
            }       

     

            return a;
        }   
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
			 break;
			case 3: 
 				 ktemp=iter.isopen;
			 break;
			case 4: 
 				 ktemp=iter.level;
			 break;
			  }
			 switch(vpos){

                   }

                   a.emplace_back(ktemp,vtemp);
                }       

    

            return a;
        }   
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }
 			switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;

                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname)
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
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;

                   }
                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;

                   }
                    a.emplace_back(ktemp,vtemp);
                }       
      
            return a;
        }   
    
        template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>     
        std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname)
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
	  }
 switch(vpos){
case 1: 
 	 vtemp=iter.name;
	 break;
case 2: 
 	 vtemp=iter.password;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
                }       
    
            return a;
        }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
        std::vector<std::pair<T,meta>> getvecRows(std::string keyname)
        {
            std::vector<std::pair<T,meta>> a;
            unsigned char kpos;
            kpos=findcolpos(keyname);
           
            for(auto &iter:record)
            { 
                switch(kpos)
                {

   case 0: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.level,iter);
	 break;

                }
            }       
    
        return a;
    }
        template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
        std::vector<std::pair<std::string,meta>> getvecRows(std::string keyname)
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

                }
            }       
    

        return a;
    }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			  }

			 switch(dpos){

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			 }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			  }

			 switch(dpos){

                   }
                }       
    
            return a;
        }
    
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;

            }
         }       
    
        return a;
    }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			  }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			  }

			 switch(dpos){

                    }
                }       
    

            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;

                }
            }       
    
            return a;
        }
    
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
    std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.userid;
				 break;
			case 3: 
 				 vtemp=iter.isopen;
				 break;
			case 4: 
 				 vtemp=iter.level;
				 break;
			 }

			switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;

            }
         }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			 }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			  }

			 switch(dpos){

                }
            }       

    
            return a;
        }
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.level);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname)
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
			  }

			 switch(vpos){
			case 1: 
 				 vtemp=iter.name;
				 break;
			case 2: 
 				 vtemp=iter.password;
				 break;
			  }

			 switch(dpos){
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.password);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
			  }

			 switch(vpos){
			case 1: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.password);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
			 }

			 switch(vpos){

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.level);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			 }

			 switch(vpos){
			case 1: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.password);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }

			 switch(vpos){

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.level);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename std::enable_if<std::is_integral_v<T>,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows(std::string keyname)
        {
            std::map<T,std::vector<meta>> a;

            unsigned char kpos;
            kpos=findcolpos(keyname);
   
            //T ktemp;
            //U vtemp;

            for(auto &iter:record)
            {
                switch(kpos)
                {

   			case 0: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.level].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
        std::map<T,std::vector<meta>> getgroupRows(std::string keyname)
        {
            std::map<T,std::vector<meta>> a;

            unsigned char kpos;
            kpos=findcolpos(keyname);
   
            //T ktemp;
            //U vtemp;

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

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname)
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
			 }

			 switch(vpos){
			case 1: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.password].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname)
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
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;

                }
            }       

    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<U>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.level].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
        template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
        std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname)
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
 				 ktemp=iter.userid;
				 break;
			case 3: 
 				 ktemp=iter.isopen;
				 break;
			case 4: 
 				 ktemp=iter.level;
				 break;
			  }

			 switch(vpos){
			case 1: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.password].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
} 
}
#endif
   