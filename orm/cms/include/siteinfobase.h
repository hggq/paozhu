#ifndef ORM_CMS_SITEINFOBASEMATA_H
#define ORM_CMS_SITEINFOBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Tue, 10 Dec 2024 07:41:52 GMT
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
#include "mysql.h"
namespace orm { 
   
     namespace cms { 

struct siteinfobase
{
    struct meta{
     int sid= 0; //
 int userid= 0; //
unsigned  int agentid= 0; //
unsigned  int languagetype= 0; //语言类型
 std::string sitename=""; //
 std::string sitedomain=""; //
 std::string metakeys=""; //
 std::string metadesc=""; //
 std::string copyright=""; //
 std::string beiansn=""; //备案号
 std::string footscript=""; //底部脚本
 std::string headscript=""; //头部script
 std::string introduce=""; //
 std::string sitelogo=""; //logo url
 std::string contactman=""; //
 std::string phone=""; //
 std::string mobile=""; //
 std::string email=""; //
 std::string bankname=""; //
 std::string banksn=""; //
 std::string address=""; //
 std::string zipnum=""; //
 std::string taxsn=""; //
 std::string companyname=""; //
 std::string linkname=""; //
 std::string linkmobile=""; //
 std::string linkaddress=""; //
 std::string theme=""; //
 std::string sitepath=""; //
unsigned  char isopen=0; //
unsigned  int created_at= 0; //
unsigned  int enddate= 0; //
 } data;
 std::vector<siteinfobase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<unsigned char> _keypos{0x00};
MYSQL_ROW _row;
std::vector<siteinfobase::meta>::iterator begin(){     return record.begin(); }
std::vector<siteinfobase::meta>::iterator end(){     return record.end(); }
std::vector<siteinfobase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<siteinfobase::meta>::const_iterator end() const{     return record.end(); }
const std::array<std::string,32> colnames={"sid","userid","agentid","languagetype","sitename","sitedomain","metakeys","metadesc","copyright","beiansn","footscript","headscript","introduce","sitelogo","contactman","phone","mobile","email","bankname","banksn","address","zipnum","taxsn","companyname","linkname","linkmobile","linkaddress","theme","sitepath","isopen","created_at","enddate"};
const std::array<unsigned char,32> colnamestype= {3,3,3,3,253,253,252,252,252,253,252,252,252,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,1,3,3};
std::string tablename="siteinfo";
std::string modelname="Siteinfo";

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
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 2; }
 if(colpospppc=='s'){ return 20; }
   	 break;
 }
 break;
case 'b':
 switch(coln.size()){  
case 6:
   	 return 19;
break;
case 7:
   	 return 9;
break;
case 8:
   	 return 18;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 9:
   	 return 8;
break;
case 10:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='n'){ return 14; }
 if(colpospppc=='t'){ return 30; }
   	 break;
case 11:
   	 return 23;
break;
 }
 break;
case 'e':
 switch(coln.size()){  
case 5:
   	 return 17;
break;
case 7:
   	 return 31;
break;
 }
 break;
case 'f':
   	 return 10;
break;
case 'h':
   	 return 11;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 29;
break;
case 9:
   	 return 12;
break;
 }
 break;
case 'l':
 switch(coln.size()){  
case 8:
   	 return 24;
break;
case 10:
   	 return 25;
break;
case 11:
   	 return 26;
break;
case 12:
   	 return 3;
break;
 }
 break;
case 'm':
 switch(coln.size()){  
case 6:
   	 return 16;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='c'){ return 7; }
 if(colpospppc=='s'){ return 6; }
   	 break;
 }
 break;
case 'p':
   	 return 15;
break;
case 's':
 switch(coln.size()){  
case 3:
   	 return 0;
break;
case 8:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 4; }
 if(colpospppc=='h'){ return 28; }
 if(colpospppc=='o'){ return 13; }
   	 break;
case 10:
   	 return 5;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 27; }
 if(colpospppc=='n'){ return 22; }
   	 break;
 }
 break;
case 'u':
   	 return 1;
break;
case 'z':
   	 return 21;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "sid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     siteinfobase::meta metatemp;    
            data = metatemp; 
      }
      
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     
         return temp;
     }
     void _setColnamevalue()
      {
        for(unsigned char i=0;i<_keypos.size();i++)
        {
            switch(_keypos[i]){
        	case 0:
		 try{
			data.sid=std::stoi(_row[i]);
		}catch (...) { 
			data.sid=0;
			 }
			break;
	case 1:
		 try{
			data.userid=std::stoi(_row[i]);
		}catch (...) { 
			data.userid=0;
			 }
			break;
	case 2:
		 try{
			data.agentid=std::stoul(_row[i]);
		}catch (...) { 
			data.agentid=0;
			 }
			break;
	case 3:
		 try{
			data.languagetype=std::stoul(_row[i]);
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
	case 4:
		 try{
			data.sitename.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
	case 5:
		 try{
			data.sitedomain.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
	case 6:
		 try{
			data.metakeys.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
	case 7:
		 try{
			data.metadesc.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
	case 8:
		 try{
			data.copyright.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
	case 9:
		 try{
			data.beiansn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
	case 10:
		 try{
			data.footscript.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
	case 11:
		 try{
			data.headscript.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
	case 12:
		 try{
			data.introduce.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
	case 13:
		 try{
			data.sitelogo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
	case 14:
		 try{
			data.contactman.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
	case 15:
		 try{
			data.phone.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.phone.clear();
			 }
			break;
	case 16:
		 try{
			data.mobile.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
	case 17:
		 try{
			data.email.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.email.clear();
			 }
			break;
	case 18:
		 try{
			data.bankname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
	case 19:
		 try{
			data.banksn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
	case 20:
		 try{
			data.address.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.address.clear();
			 }
			break;
	case 21:
		 try{
			data.zipnum.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
	case 22:
		 try{
			data.taxsn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
	case 23:
		 try{
			data.companyname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
	case 24:
		 try{
			data.linkname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
	case 25:
		 try{
			data.linkmobile.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
	case 26:
		 try{
			data.linkaddress.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
	case 27:
		 try{
			data.theme.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.theme.clear();
			 }
			break;
	case 28:
		 try{
			data.sitepath.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
	case 29:
		 try{
			data.isopen=std::stoi(_row[i]);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
	case 30:
		 try{
			data.created_at=std::stoul(_row[i]);
		}catch (...) { 
			data.created_at=0;
			 }
			break;
	case 31:
		 try{
			data.enddate=std::stoul(_row[i]);
		}catch (...) { 
			data.enddate=0;
			 }
			break;
	default:
		 { }
			

                 }

                 if(i>210){
                     break;
                 }
          }
   } 
         void _addnewrowvalue(){
           siteinfobase::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        	case 0:
		 try{
			metatemp.sid=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.sid=0;
			 }
			break;
	case 1:
		 try{
			metatemp.userid=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.userid=0;
			 }
			break;
	case 2:
		 try{
			metatemp.agentid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.agentid=0;
			 }
			break;
	case 3:
		 try{
			metatemp.languagetype=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.languagetype=0;
			 }
			break;
	case 4:
		 try{
			metatemp.sitename.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.sitename.clear();
			 }
			break;
	case 5:
		 try{
			metatemp.sitedomain.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.sitedomain.clear();
			 }
			break;
	case 6:
		 try{
			metatemp.metakeys.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.metakeys.clear();
			 }
			break;
	case 7:
		 try{
			metatemp.metadesc.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.metadesc.clear();
			 }
			break;
	case 8:
		 try{
			metatemp.copyright.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.copyright.clear();
			 }
			break;
	case 9:
		 try{
			metatemp.beiansn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.beiansn.clear();
			 }
			break;
	case 10:
		 try{
			metatemp.footscript.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.footscript.clear();
			 }
			break;
	case 11:
		 try{
			metatemp.headscript.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.headscript.clear();
			 }
			break;
	case 12:
		 try{
			metatemp.introduce.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.introduce.clear();
			 }
			break;
	case 13:
		 try{
			metatemp.sitelogo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.sitelogo.clear();
			 }
			break;
	case 14:
		 try{
			metatemp.contactman.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.contactman.clear();
			 }
			break;
	case 15:
		 try{
			metatemp.phone.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.phone.clear();
			 }
			break;
	case 16:
		 try{
			metatemp.mobile.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.mobile.clear();
			 }
			break;
	case 17:
		 try{
			metatemp.email.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.email.clear();
			 }
			break;
	case 18:
		 try{
			metatemp.bankname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.bankname.clear();
			 }
			break;
	case 19:
		 try{
			metatemp.banksn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.banksn.clear();
			 }
			break;
	case 20:
		 try{
			metatemp.address.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.address.clear();
			 }
			break;
	case 21:
		 try{
			metatemp.zipnum.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.zipnum.clear();
			 }
			break;
	case 22:
		 try{
			metatemp.taxsn.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.taxsn.clear();
			 }
			break;
	case 23:
		 try{
			metatemp.companyname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.companyname.clear();
			 }
			break;
	case 24:
		 try{
			metatemp.linkname.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.linkname.clear();
			 }
			break;
	case 25:
		 try{
			metatemp.linkmobile.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.linkmobile.clear();
			 }
			break;
	case 26:
		 try{
			metatemp.linkaddress.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.linkaddress.clear();
			 }
			break;
	case 27:
		 try{
			metatemp.theme.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.theme.clear();
			 }
			break;
	case 28:
		 try{
			metatemp.sitepath.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.sitepath.clear();
			 }
			break;
	case 29:
		 try{
			metatemp.isopen=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.isopen=0;
			 }
			break;
	case 30:
		 try{
			metatemp.created_at=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.created_at=0;
			 }
			break;
	case 31:
		 try{
			metatemp.enddate=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.enddate=0;
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
            }else if(content[i]=='\\'){
                temp.append("\\\\");
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

        if(data.sid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languagetype);
}
tempsql<<",'"<<stringaddslash(data.sitename)<<"'";
tempsql<<",'"<<stringaddslash(data.sitedomain)<<"'";
tempsql<<",'"<<stringaddslash(data.metakeys)<<"'";
tempsql<<",'"<<stringaddslash(data.metadesc)<<"'";
tempsql<<",'"<<stringaddslash(data.copyright)<<"'";
tempsql<<",'"<<stringaddslash(data.beiansn)<<"'";
tempsql<<",'"<<stringaddslash(data.footscript)<<"'";
tempsql<<",'"<<stringaddslash(data.headscript)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.sitelogo)<<"'";
tempsql<<",'"<<stringaddslash(data.contactman)<<"'";
tempsql<<",'"<<stringaddslash(data.phone)<<"'";
tempsql<<",'"<<stringaddslash(data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(data.email)<<"'";
tempsql<<",'"<<stringaddslash(data.bankname)<<"'";
tempsql<<",'"<<stringaddslash(data.banksn)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.zipnum)<<"'";
tempsql<<",'"<<stringaddslash(data.taxsn)<<"'";
tempsql<<",'"<<stringaddslash(data.companyname)<<"'";
tempsql<<",'"<<stringaddslash(data.linkname)<<"'";
tempsql<<",'"<<stringaddslash(data.linkmobile)<<"'";
tempsql<<",'"<<stringaddslash(data.linkaddress)<<"'";
tempsql<<",'"<<stringaddslash(data.theme)<<"'";
tempsql<<",'"<<stringaddslash(data.sitepath)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
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
tempsql<<")";

     
       return tempsql.str();
   } 
      
      std::string _makerecordinsertsql( meta &insert_data){
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

        if(insert_data.sid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.sid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.agentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.agentid);
}
if(insert_data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languagetype);
}
tempsql<<",'"<<stringaddslash(insert_data.sitename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitedomain)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.metakeys)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.metadesc)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.copyright)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.beiansn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.footscript)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.headscript)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitelogo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.contactman)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.phone)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.email)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.bankname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.banksn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.zipnum)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.taxsn)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.companyname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkmobile)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.linkaddress)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.theme)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.sitepath)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
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
tempsql<<")";

     
       return tempsql.str();
   } 
       
      std::string _makerecordinsertsql( std::vector<meta> &insert_data){
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
            tempsql<<") VALUES ";

    for(unsigned int i=0;i<insert_data.size();i++)
    {
		if(i>0)
		{
			tempsql<<",";	
		}
		tempsql<<"(";


        	if(insert_data[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].sid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].agentid);
	}
	if(insert_data[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languagetype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sitename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitedomain)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].metakeys)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].metadesc)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].copyright)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].beiansn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].footscript)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].headscript)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitelogo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].contactman)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].phone)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].email)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].bankname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].banksn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].zipnum)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].taxsn)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].companyname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkmobile)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].linkaddress)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].theme)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].sitepath)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
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
		tempsql<<")";
	 } 

     
       return tempsql.str();
   } 
       
    std::string _makeupdatesql(const std::string &fileld){
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

        if(data.sid==0){
	tempsql<<"`sid`=0";
 }else{ 
	tempsql<<"`sid`="<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",`agentid`=0";
 }else{ 
	tempsql<<",`agentid`="<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",`languagetype`=0";
 }else{ 
	tempsql<<",`languagetype`="<<std::to_string(data.languagetype);
}
tempsql<<",`sitename`='"<<stringaddslash(data.sitename)<<"'";
tempsql<<",`sitedomain`='"<<stringaddslash(data.sitedomain)<<"'";
tempsql<<",`metakeys`='"<<stringaddslash(data.metakeys)<<"'";
tempsql<<",`metadesc`='"<<stringaddslash(data.metadesc)<<"'";
tempsql<<",`copyright`='"<<stringaddslash(data.copyright)<<"'";
tempsql<<",`beiansn`='"<<stringaddslash(data.beiansn)<<"'";
tempsql<<",`footscript`='"<<stringaddslash(data.footscript)<<"'";
tempsql<<",`headscript`='"<<stringaddslash(data.headscript)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`sitelogo`='"<<stringaddslash(data.sitelogo)<<"'";
tempsql<<",`contactman`='"<<stringaddslash(data.contactman)<<"'";
tempsql<<",`phone`='"<<stringaddslash(data.phone)<<"'";
tempsql<<",`mobile`='"<<stringaddslash(data.mobile)<<"'";
tempsql<<",`email`='"<<stringaddslash(data.email)<<"'";
tempsql<<",`bankname`='"<<stringaddslash(data.bankname)<<"'";
tempsql<<",`banksn`='"<<stringaddslash(data.banksn)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`zipnum`='"<<stringaddslash(data.zipnum)<<"'";
tempsql<<",`taxsn`='"<<stringaddslash(data.taxsn)<<"'";
tempsql<<",`companyname`='"<<stringaddslash(data.companyname)<<"'";
tempsql<<",`linkname`='"<<stringaddslash(data.linkname)<<"'";
tempsql<<",`linkmobile`='"<<stringaddslash(data.linkmobile)<<"'";
tempsql<<",`linkaddress`='"<<stringaddslash(data.linkaddress)<<"'";
tempsql<<",`theme`='"<<stringaddslash(data.theme)<<"'";
tempsql<<",`sitepath`='"<<stringaddslash(data.sitepath)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
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
 }else{ 

     
  unsigned int jj=0;
                  std::string keyname;
                  std::vector<unsigned char> keypos;
                  for(;jj<fileld.size();jj++){
                        if(fileld[jj]==','){
                                unsigned char bpos_i=findcolpos(keyname);
                               keypos.emplace_back(bpos_i); 
#ifdef DEBUG
                    if (bpos_i == 254)
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
                    if (bpos_i == 254)
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
if(data.sid==0){
	tempsql<<"`sid`=0";
 }else{ 
	tempsql<<"`sid`="<<std::to_string(data.sid);
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
if(data.agentid==0){
	tempsql<<"`agentid`=0";
 }else{ 
	tempsql<<"`agentid`="<<std::to_string(data.agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"`languagetype`=0";
 }else{ 
	tempsql<<"`languagetype`="<<std::to_string(data.languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitename`='"<<stringaddslash(data.sitename)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitedomain`='"<<stringaddslash(data.sitedomain)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`metakeys`='"<<stringaddslash(data.metakeys)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`metadesc`='"<<stringaddslash(data.metadesc)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`copyright`='"<<stringaddslash(data.copyright)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`beiansn`='"<<stringaddslash(data.beiansn)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`footscript`='"<<stringaddslash(data.footscript)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`headscript`='"<<stringaddslash(data.headscript)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitelogo`='"<<stringaddslash(data.sitelogo)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`contactman`='"<<stringaddslash(data.contactman)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`phone`='"<<stringaddslash(data.phone)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mobile`='"<<stringaddslash(data.mobile)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`email`='"<<stringaddslash(data.email)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`bankname`='"<<stringaddslash(data.bankname)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`banksn`='"<<stringaddslash(data.banksn)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`zipnum`='"<<stringaddslash(data.zipnum)<<"'";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`taxsn`='"<<stringaddslash(data.taxsn)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`companyname`='"<<stringaddslash(data.companyname)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkname`='"<<stringaddslash(data.linkname)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkmobile`='"<<stringaddslash(data.linkmobile)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkaddress`='"<<stringaddslash(data.linkaddress)<<"'";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`theme`='"<<stringaddslash(data.theme)<<"'";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sitepath`='"<<stringaddslash(data.sitepath)<<"'";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"`created_at`=0";
 }else{ 
	tempsql<<"`created_at`="<<std::to_string(data.created_at);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"`enddate`=0";
 }else{ 
	tempsql<<"`enddate`="<<std::to_string(data.enddate);
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
        for (; j < colnames.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << colnames[j];
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
            	if(record[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].sid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].agentid);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sitename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitedomain)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metakeys)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metadesc)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].copyright)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].beiansn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].footscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].headscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitelogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].contactman)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].phone)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bankname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].banksn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zipnum)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].taxsn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].companyname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkmobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].theme)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitepath)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
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
        for (; j < colnames.size(); j++)
        {
            if (j > 0)
            {
                tempsql << "`,`";
            }
            else
            {
                tempsql << "`";
            }
            tempsql << colnames[j];
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
            	if(record[i].sid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].sid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].agentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].agentid);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sitename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitedomain)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metakeys)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].metadesc)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].copyright)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].beiansn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].footscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].headscript)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitelogo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].contactman)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].phone)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].email)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bankname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].banksn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zipnum)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].taxsn)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].companyname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkmobile)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].linkaddress)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].theme)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].sitepath)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
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
            for(jj=0;jj<colnames.size();jj++){
                keypos.emplace_back(jj); 
            }
        }
               
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
         case 0:
if(data.sid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sid));
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
if(data.agentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.agentid));
}
 break;
 case 3:
if(data.languagetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languagetype));
}
 break;
 case 4:
	temparray.push_back(data.sitename);
 break;
 case 5:
	temparray.push_back(data.sitedomain);
 break;
 case 6:
	temparray.push_back(data.metakeys);
 break;
 case 7:
	temparray.push_back(data.metadesc);
 break;
 case 8:
	temparray.push_back(data.copyright);
 break;
 case 9:
	temparray.push_back(data.beiansn);
 break;
 case 10:
	temparray.push_back(data.footscript);
 break;
 case 11:
	temparray.push_back(data.headscript);
 break;
 case 12:
	temparray.push_back(data.introduce);
 break;
 case 13:
	temparray.push_back(data.sitelogo);
 break;
 case 14:
	temparray.push_back(data.contactman);
 break;
 case 15:
	temparray.push_back(data.phone);
 break;
 case 16:
	temparray.push_back(data.mobile);
 break;
 case 17:
	temparray.push_back(data.email);
 break;
 case 18:
	temparray.push_back(data.bankname);
 break;
 case 19:
	temparray.push_back(data.banksn);
 break;
 case 20:
	temparray.push_back(data.address);
 break;
 case 21:
	temparray.push_back(data.zipnum);
 break;
 case 22:
	temparray.push_back(data.taxsn);
 break;
 case 23:
	temparray.push_back(data.companyname);
 break;
 case 24:
	temparray.push_back(data.linkname);
 break;
 case 25:
	temparray.push_back(data.linkmobile);
 break;
 case 26:
	temparray.push_back(data.linkaddress);
 break;
 case 27:
	temparray.push_back(data.theme);
 break;
 case 28:
	temparray.push_back(data.sitepath);
 break;
 case 29:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 30:
if(data.created_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.created_at));
}
 break;
 case 31:
if(data.enddate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.enddate));
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
         case 0:
if(data.sid==0){
	tempsql.insert({"sid","0"});
 }else{ 
	tempsql.insert({"sid",std::to_string(data.sid)});
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
if(data.agentid==0){
	tempsql.insert({"agentid","0"});
 }else{ 
	tempsql.insert({"agentid",std::to_string(data.agentid)});
}
 break;
 case 3:
if(data.languagetype==0){
	tempsql.insert({"languagetype","0"});
 }else{ 
	tempsql.insert({"languagetype",std::to_string(data.languagetype)});
}
 break;
 case 4:
	tempsql.insert({"sitename",data.sitename});
 break;
 case 5:
	tempsql.insert({"sitedomain",data.sitedomain});
 break;
 case 6:
	tempsql.insert({"metakeys",data.metakeys});
 break;
 case 7:
	tempsql.insert({"metadesc",data.metadesc});
 break;
 case 8:
	tempsql.insert({"copyright",data.copyright});
 break;
 case 9:
	tempsql.insert({"beiansn",data.beiansn});
 break;
 case 10:
	tempsql.insert({"footscript",data.footscript});
 break;
 case 11:
	tempsql.insert({"headscript",data.headscript});
 break;
 case 12:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 13:
	tempsql.insert({"sitelogo",data.sitelogo});
 break;
 case 14:
	tempsql.insert({"contactman",data.contactman});
 break;
 case 15:
	tempsql.insert({"phone",data.phone});
 break;
 case 16:
	tempsql.insert({"mobile",data.mobile});
 break;
 case 17:
	tempsql.insert({"email",data.email});
 break;
 case 18:
	tempsql.insert({"bankname",data.bankname});
 break;
 case 19:
	tempsql.insert({"banksn",data.banksn});
 break;
 case 20:
	tempsql.insert({"address",data.address});
 break;
 case 21:
	tempsql.insert({"zipnum",data.zipnum});
 break;
 case 22:
	tempsql.insert({"taxsn",data.taxsn});
 break;
 case 23:
	tempsql.insert({"companyname",data.companyname});
 break;
 case 24:
	tempsql.insert({"linkname",data.linkname});
 break;
 case 25:
	tempsql.insert({"linkmobile",data.linkmobile});
 break;
 case 26:
	tempsql.insert({"linkaddress",data.linkaddress});
 break;
 case 27:
	tempsql.insert({"theme",data.theme});
 break;
 case 28:
	tempsql.insert({"sitepath",data.sitepath});
 break;
 case 29:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 30:
if(data.created_at==0){
	tempsql.insert({"created_at","0"});
 }else{ 
	tempsql.insert({"created_at",std::to_string(data.created_at)});
}
 break;
 case 31:
if(data.enddate==0){
	tempsql.insert({"enddate","0"});
 }else{ 
	tempsql.insert({"enddate",std::to_string(data.enddate)});
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
if(data.sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(data.sid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.agentid==0){
	tempsql<<",\"agentid\":0";
 }else{ 
	tempsql<<",\"agentid\":"<<std::to_string(data.agentid);
}
if(data.languagetype==0){
	tempsql<<",\"languagetype\":0";
 }else{ 
	tempsql<<",\"languagetype\":"<<std::to_string(data.languagetype);
}
tempsql<<",\"sitename\":\""<<http::utf8_to_jsonstring(data.sitename);
tempsql<<"\"";
tempsql<<",\"sitedomain\":\""<<http::utf8_to_jsonstring(data.sitedomain);
tempsql<<"\"";
tempsql<<",\"metakeys\":\""<<http::utf8_to_jsonstring(data.metakeys);
tempsql<<"\"";
tempsql<<",\"metadesc\":\""<<http::utf8_to_jsonstring(data.metadesc);
tempsql<<"\"";
tempsql<<",\"copyright\":\""<<http::utf8_to_jsonstring(data.copyright);
tempsql<<"\"";
tempsql<<",\"beiansn\":\""<<http::utf8_to_jsonstring(data.beiansn);
tempsql<<"\"";
tempsql<<",\"footscript\":\""<<http::utf8_to_jsonstring(data.footscript);
tempsql<<"\"";
tempsql<<",\"headscript\":\""<<http::utf8_to_jsonstring(data.headscript);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"sitelogo\":\""<<http::utf8_to_jsonstring(data.sitelogo);
tempsql<<"\"";
tempsql<<",\"contactman\":\""<<http::utf8_to_jsonstring(data.contactman);
tempsql<<"\"";
tempsql<<",\"phone\":\""<<http::utf8_to_jsonstring(data.phone);
tempsql<<"\"";
tempsql<<",\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile);
tempsql<<"\"";
tempsql<<",\"email\":\""<<http::utf8_to_jsonstring(data.email);
tempsql<<"\"";
tempsql<<",\"bankname\":\""<<http::utf8_to_jsonstring(data.bankname);
tempsql<<"\"";
tempsql<<",\"banksn\":\""<<http::utf8_to_jsonstring(data.banksn);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"zipnum\":\""<<http::utf8_to_jsonstring(data.zipnum);
tempsql<<"\"";
tempsql<<",\"taxsn\":\""<<http::utf8_to_jsonstring(data.taxsn);
tempsql<<"\"";
tempsql<<",\"companyname\":\""<<http::utf8_to_jsonstring(data.companyname);
tempsql<<"\"";
tempsql<<",\"linkname\":\""<<http::utf8_to_jsonstring(data.linkname);
tempsql<<"\"";
tempsql<<",\"linkmobile\":\""<<http::utf8_to_jsonstring(data.linkmobile);
tempsql<<"\"";
tempsql<<",\"linkaddress\":\""<<http::utf8_to_jsonstring(data.linkaddress);
tempsql<<"\"";
tempsql<<",\"theme\":\""<<http::utf8_to_jsonstring(data.theme);
tempsql<<"\"";
tempsql<<",\"sitepath\":\""<<http::utf8_to_jsonstring(data.sitepath);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
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
if(data.sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(data.sid);
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
if(data.agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(data.agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(data.languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(data.sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(data.sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(data.metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(data.metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(data.copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(data.beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(data.footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(data.headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(data.sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(data.contactman)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(data.phone)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(data.mobile)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(data.email)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(data.bankname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(data.banksn)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(data.zipnum)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(data.taxsn)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(data.companyname)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(data.linkname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(data.linkmobile)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(data.linkaddress)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(data.theme)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(data.sitepath)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(data.created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(data.created_at);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(data.enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(data.enddate);
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
        siteinfobase::meta metatemp; 
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
			data.sid=std::stoi(set_value_name);
		}catch (...) { 
			data.sid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=std::stoi(set_value_name);
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.agentid=std::stoul(set_value_name);
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=std::stoul(set_value_name);
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename.append(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain.append(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys.append(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc.append(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright.append(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn.append(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript.append(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript.append(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo.append(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.contactman.append(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 15:
		 try{
			data.phone.append(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 16:
		 try{
			data.mobile.append(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 17:
		 try{
			data.email.append(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 18:
		 try{
			data.bankname.append(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 19:
		 try{
			data.banksn.append(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 20:
		 try{
			data.address.append(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 21:
		 try{
			data.zipnum.append(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 22:
		 try{
			data.taxsn.append(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 23:
		 try{
			data.companyname.append(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 24:
		 try{
			data.linkname.append(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkmobile.append(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 26:
		 try{
			data.linkaddress.append(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 27:
		 try{
			data.theme.append(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 28:
		 try{
			data.sitepath.append(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 29:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 30:
		 try{
			data.created_at=std::stoul(set_value_name);
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 31:
		 try{
			data.enddate=std::stoul(set_value_name);
		}catch (...) { 
			data.enddate=0;
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
			data.sid=set_value_name;
		}catch (...) { 
			data.sid=0;
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
			data.agentid=set_value_name;
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename=std::to_string(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain=std::to_string(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys=std::to_string(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc=std::to_string(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright=std::to_string(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn=std::to_string(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript=std::to_string(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript=std::to_string(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo=std::to_string(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.contactman=std::to_string(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 15:
		 try{
			data.phone=std::to_string(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 16:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 17:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 18:
		 try{
			data.bankname=std::to_string(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 19:
		 try{
			data.banksn=std::to_string(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 20:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 21:
		 try{
			data.zipnum=std::to_string(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 22:
		 try{
			data.taxsn=std::to_string(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 23:
		 try{
			data.companyname=std::to_string(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 24:
		 try{
			data.linkname=std::to_string(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkmobile=std::to_string(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 26:
		 try{
			data.linkaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 27:
		 try{
			data.theme=std::to_string(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 28:
		 try{
			data.sitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 29:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 30:
		 try{
			data.created_at=set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 31:
		 try{
			data.enddate=set_value_name;
		}catch (...) { 
			data.enddate=0;
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
			data.sid=(int)set_value_name;
		}catch (...) { 
			data.sid=0;
			 }
			break;
		case 1:
		 try{
			data.userid=(int)set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 2:
		 try{
			data.agentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.agentid=0;
			 }
			break;
		case 3:
		 try{
			data.languagetype=(unsigned int)set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 4:
		 try{
			data.sitename=std::to_string(set_value_name);
		}catch (...) { 
			data.sitename.clear();
			 }
			break;
		case 5:
		 try{
			data.sitedomain=std::to_string(set_value_name);
		}catch (...) { 
			data.sitedomain.clear();
			 }
			break;
		case 6:
		 try{
			data.metakeys=std::to_string(set_value_name);
		}catch (...) { 
			data.metakeys.clear();
			 }
			break;
		case 7:
		 try{
			data.metadesc=std::to_string(set_value_name);
		}catch (...) { 
			data.metadesc.clear();
			 }
			break;
		case 8:
		 try{
			data.copyright=std::to_string(set_value_name);
		}catch (...) { 
			data.copyright.clear();
			 }
			break;
		case 9:
		 try{
			data.beiansn=std::to_string(set_value_name);
		}catch (...) { 
			data.beiansn.clear();
			 }
			break;
		case 10:
		 try{
			data.footscript=std::to_string(set_value_name);
		}catch (...) { 
			data.footscript.clear();
			 }
			break;
		case 11:
		 try{
			data.headscript=std::to_string(set_value_name);
		}catch (...) { 
			data.headscript.clear();
			 }
			break;
		case 12:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 13:
		 try{
			data.sitelogo=std::to_string(set_value_name);
		}catch (...) { 
			data.sitelogo.clear();
			 }
			break;
		case 14:
		 try{
			data.contactman=std::to_string(set_value_name);
		}catch (...) { 
			data.contactman.clear();
			 }
			break;
		case 15:
		 try{
			data.phone=std::to_string(set_value_name);
		}catch (...) { 
			data.phone.clear();
			 }
			break;
		case 16:
		 try{
			data.mobile=std::to_string(set_value_name);
		}catch (...) { 
			data.mobile.clear();
			 }
			break;
		case 17:
		 try{
			data.email=std::to_string(set_value_name);
		}catch (...) { 
			data.email.clear();
			 }
			break;
		case 18:
		 try{
			data.bankname=std::to_string(set_value_name);
		}catch (...) { 
			data.bankname.clear();
			 }
			break;
		case 19:
		 try{
			data.banksn=std::to_string(set_value_name);
		}catch (...) { 
			data.banksn.clear();
			 }
			break;
		case 20:
		 try{
			data.address=std::to_string(set_value_name);
		}catch (...) { 
			data.address.clear();
			 }
			break;
		case 21:
		 try{
			data.zipnum=std::to_string(set_value_name);
		}catch (...) { 
			data.zipnum.clear();
			 }
			break;
		case 22:
		 try{
			data.taxsn=std::to_string(set_value_name);
		}catch (...) { 
			data.taxsn.clear();
			 }
			break;
		case 23:
		 try{
			data.companyname=std::to_string(set_value_name);
		}catch (...) { 
			data.companyname.clear();
			 }
			break;
		case 24:
		 try{
			data.linkname=std::to_string(set_value_name);
		}catch (...) { 
			data.linkname.clear();
			 }
			break;
		case 25:
		 try{
			data.linkmobile=std::to_string(set_value_name);
		}catch (...) { 
			data.linkmobile.clear();
			 }
			break;
		case 26:
		 try{
			data.linkaddress=std::to_string(set_value_name);
		}catch (...) { 
			data.linkaddress.clear();
			 }
			break;
		case 27:
		 try{
			data.theme=std::to_string(set_value_name);
		}catch (...) { 
			data.theme.clear();
			 }
			break;
		case 28:
		 try{
			data.sitepath=std::to_string(set_value_name);
		}catch (...) { 
			data.sitepath.clear();
			 }
			break;
		case 29:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 30:
		 try{
			data.created_at=(unsigned int)set_value_name;
		}catch (...) { 
			data.created_at=0;
			 }
			break;
		case 31:
		 try{
			data.enddate=(unsigned int)set_value_name;
		}catch (...) { 
			data.enddate=0;
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
if(record[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(record[n].sid);
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
if(record[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(record[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(record[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(record[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(record[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(record[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(record[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(record[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(record[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(record[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(record[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(record[n].contactman)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(record[n].phone)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(record[n].bankname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(record[n].banksn)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(record[n].zipnum)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(record[n].taxsn)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(record[n].companyname)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(record[n].linkname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(record[n].linkmobile)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(record[n].linkaddress)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(record[n].theme)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(record[n].sitepath)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
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
if(record[n].sid==0){
	tempsql<<"\"sid\":0";
 }else{ 
	tempsql<<"\"sid\":"<<std::to_string(record[n].sid);
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
if(record[n].agentid==0){
	tempsql<<"\"agentid\":0";
 }else{ 
	tempsql<<"\"agentid\":"<<std::to_string(record[n].agentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitename\":\""<<http::utf8_to_jsonstring(record[n].sitename)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitedomain\":\""<<http::utf8_to_jsonstring(record[n].sitedomain)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metakeys\":\""<<http::utf8_to_jsonstring(record[n].metakeys)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"metadesc\":\""<<http::utf8_to_jsonstring(record[n].metadesc)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"copyright\":\""<<http::utf8_to_jsonstring(record[n].copyright)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"beiansn\":\""<<http::utf8_to_jsonstring(record[n].beiansn)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"footscript\":\""<<http::utf8_to_jsonstring(record[n].footscript)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"headscript\":\""<<http::utf8_to_jsonstring(record[n].headscript)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitelogo\":\""<<http::utf8_to_jsonstring(record[n].sitelogo)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"contactman\":\""<<http::utf8_to_jsonstring(record[n].contactman)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"phone\":\""<<http::utf8_to_jsonstring(record[n].phone)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mobile\":\""<<http::utf8_to_jsonstring(record[n].mobile)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"email\":\""<<http::utf8_to_jsonstring(record[n].email)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bankname\":\""<<http::utf8_to_jsonstring(record[n].bankname)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"banksn\":\""<<http::utf8_to_jsonstring(record[n].banksn)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zipnum\":\""<<http::utf8_to_jsonstring(record[n].zipnum)<<"\"";
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"taxsn\":\""<<http::utf8_to_jsonstring(record[n].taxsn)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"companyname\":\""<<http::utf8_to_jsonstring(record[n].companyname)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkname\":\""<<http::utf8_to_jsonstring(record[n].linkname)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkmobile\":\""<<http::utf8_to_jsonstring(record[n].linkmobile)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkaddress\":\""<<http::utf8_to_jsonstring(record[n].linkaddress)<<"\"";
 break;
 case 27:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"theme\":\""<<http::utf8_to_jsonstring(record[n].theme)<<"\"";
 break;
 case 28:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sitepath\":\""<<http::utf8_to_jsonstring(record[n].sitepath)<<"\"";
 break;
 case 29:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 30:
 if(jj>0){ tempsql<<","; } 
if(record[n].created_at==0){
	tempsql<<"\"created_at\":0";
 }else{ 
	tempsql<<"\"created_at\":"<<std::to_string(record[n].created_at);
}
 break;
 case 31:
 if(jj>0){ tempsql<<","; } 
if(record[n].enddate==0){
	tempsql<<"\"enddate\":0";
 }else{ 
	tempsql<<"\"enddate\":"<<std::to_string(record[n].enddate);
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
   long long getPK(){  return data.sid; } 
 void setPK(long long val){  data.sid=val;} 
 int  getSid(){  return data.sid; } 
 void setSid( int  val){  data.sid=val;} 

 int  getUserid(){  return data.userid; } 
 void setUserid( int  val){  data.userid=val;} 

unsigned  int  getAgentid(){  return data.agentid; } 
 void setAgentid(unsigned  int  val){  data.agentid=val;} 

unsigned  int  getLanguagetype(){  return data.languagetype; } 
 void setLanguagetype(unsigned  int  val){  data.languagetype=val;} 

std::string getSitename(){  return data.sitename; } 
std::string& getRefSitename(){  return std::ref(data.sitename); } 
 void setSitename(std::string &val){  data.sitename=val;} 
 void setSitename(std::string_view val){  data.sitename=val;} 

std::string getSitedomain(){  return data.sitedomain; } 
std::string& getRefSitedomain(){  return std::ref(data.sitedomain); } 
 void setSitedomain(std::string &val){  data.sitedomain=val;} 
 void setSitedomain(std::string_view val){  data.sitedomain=val;} 

std::string getMetakeys(){  return data.metakeys; } 
std::string& getRefMetakeys(){  return std::ref(data.metakeys); } 
 void setMetakeys(std::string &val){  data.metakeys=val;} 
 void setMetakeys(std::string_view val){  data.metakeys=val;} 

std::string getMetadesc(){  return data.metadesc; } 
std::string& getRefMetadesc(){  return std::ref(data.metadesc); } 
 void setMetadesc(std::string &val){  data.metadesc=val;} 
 void setMetadesc(std::string_view val){  data.metadesc=val;} 

std::string getCopyright(){  return data.copyright; } 
std::string& getRefCopyright(){  return std::ref(data.copyright); } 
 void setCopyright(std::string &val){  data.copyright=val;} 
 void setCopyright(std::string_view val){  data.copyright=val;} 

std::string getBeiansn(){  return data.beiansn; } 
std::string& getRefBeiansn(){  return std::ref(data.beiansn); } 
 void setBeiansn(std::string &val){  data.beiansn=val;} 
 void setBeiansn(std::string_view val){  data.beiansn=val;} 

std::string getFootscript(){  return data.footscript; } 
std::string& getRefFootscript(){  return std::ref(data.footscript); } 
 void setFootscript(std::string &val){  data.footscript=val;} 
 void setFootscript(std::string_view val){  data.footscript=val;} 

std::string getHeadscript(){  return data.headscript; } 
std::string& getRefHeadscript(){  return std::ref(data.headscript); } 
 void setHeadscript(std::string &val){  data.headscript=val;} 
 void setHeadscript(std::string_view val){  data.headscript=val;} 

std::string getIntroduce(){  return data.introduce; } 
std::string& getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce(std::string &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

std::string getSitelogo(){  return data.sitelogo; } 
std::string& getRefSitelogo(){  return std::ref(data.sitelogo); } 
 void setSitelogo(std::string &val){  data.sitelogo=val;} 
 void setSitelogo(std::string_view val){  data.sitelogo=val;} 

std::string getContactman(){  return data.contactman; } 
std::string& getRefContactman(){  return std::ref(data.contactman); } 
 void setContactman(std::string &val){  data.contactman=val;} 
 void setContactman(std::string_view val){  data.contactman=val;} 

std::string getPhone(){  return data.phone; } 
std::string& getRefPhone(){  return std::ref(data.phone); } 
 void setPhone(std::string &val){  data.phone=val;} 
 void setPhone(std::string_view val){  data.phone=val;} 

std::string getMobile(){  return data.mobile; } 
std::string& getRefMobile(){  return std::ref(data.mobile); } 
 void setMobile(std::string &val){  data.mobile=val;} 
 void setMobile(std::string_view val){  data.mobile=val;} 

std::string getEmail(){  return data.email; } 
std::string& getRefEmail(){  return std::ref(data.email); } 
 void setEmail(std::string &val){  data.email=val;} 
 void setEmail(std::string_view val){  data.email=val;} 

std::string getBankname(){  return data.bankname; } 
std::string& getRefBankname(){  return std::ref(data.bankname); } 
 void setBankname(std::string &val){  data.bankname=val;} 
 void setBankname(std::string_view val){  data.bankname=val;} 

std::string getBanksn(){  return data.banksn; } 
std::string& getRefBanksn(){  return std::ref(data.banksn); } 
 void setBanksn(std::string &val){  data.banksn=val;} 
 void setBanksn(std::string_view val){  data.banksn=val;} 

std::string getAddress(){  return data.address; } 
std::string& getRefAddress(){  return std::ref(data.address); } 
 void setAddress(std::string &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

std::string getZipnum(){  return data.zipnum; } 
std::string& getRefZipnum(){  return std::ref(data.zipnum); } 
 void setZipnum(std::string &val){  data.zipnum=val;} 
 void setZipnum(std::string_view val){  data.zipnum=val;} 

std::string getTaxsn(){  return data.taxsn; } 
std::string& getRefTaxsn(){  return std::ref(data.taxsn); } 
 void setTaxsn(std::string &val){  data.taxsn=val;} 
 void setTaxsn(std::string_view val){  data.taxsn=val;} 

std::string getCompanyname(){  return data.companyname; } 
std::string& getRefCompanyname(){  return std::ref(data.companyname); } 
 void setCompanyname(std::string &val){  data.companyname=val;} 
 void setCompanyname(std::string_view val){  data.companyname=val;} 

std::string getLinkname(){  return data.linkname; } 
std::string& getRefLinkname(){  return std::ref(data.linkname); } 
 void setLinkname(std::string &val){  data.linkname=val;} 
 void setLinkname(std::string_view val){  data.linkname=val;} 

std::string getLinkmobile(){  return data.linkmobile; } 
std::string& getRefLinkmobile(){  return std::ref(data.linkmobile); } 
 void setLinkmobile(std::string &val){  data.linkmobile=val;} 
 void setLinkmobile(std::string_view val){  data.linkmobile=val;} 

std::string getLinkaddress(){  return data.linkaddress; } 
std::string& getRefLinkaddress(){  return std::ref(data.linkaddress); } 
 void setLinkaddress(std::string &val){  data.linkaddress=val;} 
 void setLinkaddress(std::string_view val){  data.linkaddress=val;} 

std::string getTheme(){  return data.theme; } 
std::string& getRefTheme(){  return std::ref(data.theme); } 
 void setTheme(std::string &val){  data.theme=val;} 
 void setTheme(std::string_view val){  data.theme=val;} 

std::string getSitepath(){  return data.sitepath; } 
std::string& getRefSitepath(){  return std::ref(data.sitepath); } 
 void setSitepath(std::string &val){  data.sitepath=val;} 
 void setSitepath(std::string_view val){  data.sitepath=val;} 

unsigned  int  getIsopen(){  return data.isopen; } 
 void setIsopen(unsigned  int  val){  data.isopen=val;} 

unsigned  int  getCreatedAt(){  return data.created_at; } 
 void setCreatedAt(unsigned  int  val){  data.created_at=val;} 

unsigned  int  getEnddate(){  return data.enddate; } 
 void setEnddate(unsigned  int  val){  data.enddate=val;} 

siteinfobase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
siteinfobase::meta getData(){
 	 return data; 
} 
std::vector<siteinfobase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="sitename")
		{
			return data.sitename;
		}
		 if(key_name=="sitedomain")
		{
			return data.sitedomain;
		}
		 if(key_name=="metakeys")
		{
			return data.metakeys;
		}
		 if(key_name=="metadesc")
		{
			return data.metadesc;
		}
		 if(key_name=="copyright")
		{
			return data.copyright;
		}
		 if(key_name=="beiansn")
		{
			return data.beiansn;
		}
		 if(key_name=="footscript")
		{
			return data.footscript;
		}
		 if(key_name=="headscript")
		{
			return data.headscript;
		}
		 if(key_name=="introduce")
		{
			return data.introduce;
		}
		 if(key_name=="sitelogo")
		{
			return data.sitelogo;
		}
		 if(key_name=="contactman")
		{
			return data.contactman;
		}
		 if(key_name=="phone")
		{
			return data.phone;
		}
		 if(key_name=="mobile")
		{
			return data.mobile;
		}
		 if(key_name=="email")
		{
			return data.email;
		}
		 if(key_name=="bankname")
		{
			return data.bankname;
		}
		 if(key_name=="banksn")
		{
			return data.banksn;
		}
		 if(key_name=="address")
		{
			return data.address;
		}
		 if(key_name=="zipnum")
		{
			return data.zipnum;
		}
		 if(key_name=="taxsn")
		{
			return data.taxsn;
		}
		 if(key_name=="companyname")
		{
			return data.companyname;
		}
		 if(key_name=="linkname")
		{
			return data.linkname;
		}
		 if(key_name=="linkmobile")
		{
			return data.linkmobile;
		}
		 if(key_name=="linkaddress")
		{
			return data.linkaddress;
		}
		 if(key_name=="theme")
		{
			return data.theme;
		}
		 if(key_name=="sitepath")
		{
			return data.sitepath;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="sid")
		{
			return data.sid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="agentid")
		{
			return data.agentid;
		}
		 if(key_name=="languagetype")
		{
			return data.languagetype;
		}
		 if(key_name=="isopen")
		{
			return data.isopen;
		}
		 if(key_name=="created_at")
		{
			return data.created_at;
		}
		 if(key_name=="enddate")
		{
			return data.enddate;
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
 				 a.emplace_back(iter.sid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a.emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a.emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a.emplace_back(iter.enddate);
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
 				 return data.sid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.agentid;
				 break;
			case 3: 
 				 return data.languagetype;
				 break;
			case 29: 
 				 return data.isopen;
				 break;
			case 30: 
 				 return data.created_at;
				 break;
			case 31: 
 				 return data.enddate;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] siteinfobase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.sid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.agentid;
				 break;
			case 3: 
 				 return iter.languagetype;
				 break;
			case 29: 
 				 return iter.isopen;
				 break;
			case 30: 
 				 return iter.created_at;
				 break;
			case 31: 
 				 return iter.enddate;
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
            T getVal([[maybe_unused]] siteinfobase::meta & iter,std::string keyname)
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
 				 return data.sitename;
				 break;
			case 5: 
 				 return data.sitedomain;
				 break;
			case 6: 
 				 return data.metakeys;
				 break;
			case 7: 
 				 return data.metadesc;
				 break;
			case 8: 
 				 return data.copyright;
				 break;
			case 9: 
 				 return data.beiansn;
				 break;
			case 10: 
 				 return data.footscript;
				 break;
			case 11: 
 				 return data.headscript;
				 break;
			case 12: 
 				 return data.introduce;
				 break;
			case 13: 
 				 return data.sitelogo;
				 break;
			case 14: 
 				 return data.contactman;
				 break;
			case 15: 
 				 return data.phone;
				 break;
			case 16: 
 				 return data.mobile;
				 break;
			case 17: 
 				 return data.email;
				 break;
			case 18: 
 				 return data.bankname;
				 break;
			case 19: 
 				 return data.banksn;
				 break;
			case 20: 
 				 return data.address;
				 break;
			case 21: 
 				 return data.zipnum;
				 break;
			case 22: 
 				 return data.taxsn;
				 break;
			case 23: 
 				 return data.companyname;
				 break;
			case 24: 
 				 return data.linkname;
				 break;
			case 25: 
 				 return data.linkmobile;
				 break;
			case 26: 
 				 return data.linkaddress;
				 break;
			case 27: 
 				 return data.theme;
				 break;
			case 28: 
 				 return data.sitepath;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] siteinfobase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 4: 
 				 return iter.sitename;
				 break;
			case 5: 
 				 return iter.sitedomain;
				 break;
			case 6: 
 				 return iter.metakeys;
				 break;
			case 7: 
 				 return iter.metadesc;
				 break;
			case 8: 
 				 return iter.copyright;
				 break;
			case 9: 
 				 return iter.beiansn;
				 break;
			case 10: 
 				 return iter.footscript;
				 break;
			case 11: 
 				 return iter.headscript;
				 break;
			case 12: 
 				 return iter.introduce;
				 break;
			case 13: 
 				 return iter.sitelogo;
				 break;
			case 14: 
 				 return iter.contactman;
				 break;
			case 15: 
 				 return iter.phone;
				 break;
			case 16: 
 				 return iter.mobile;
				 break;
			case 17: 
 				 return iter.email;
				 break;
			case 18: 
 				 return iter.bankname;
				 break;
			case 19: 
 				 return iter.banksn;
				 break;
			case 20: 
 				 return iter.address;
				 break;
			case 21: 
 				 return iter.zipnum;
				 break;
			case 22: 
 				 return iter.taxsn;
				 break;
			case 23: 
 				 return iter.companyname;
				 break;
			case 24: 
 				 return iter.linkname;
				 break;
			case 25: 
 				 return iter.linkmobile;
				 break;
			case 26: 
 				 return iter.linkaddress;
				 break;
			case 27: 
 				 return iter.theme;
				 break;
			case 28: 
 				 return iter.sitepath;
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
 				 a.emplace_back(iter.sitename);
					 break;
			case 5: 
 				 a.emplace_back(iter.sitedomain);
					 break;
			case 6: 
 				 a.emplace_back(iter.metakeys);
					 break;
			case 7: 
 				 a.emplace_back(iter.metadesc);
					 break;
			case 8: 
 				 a.emplace_back(iter.copyright);
					 break;
			case 9: 
 				 a.emplace_back(iter.beiansn);
					 break;
			case 10: 
 				 a.emplace_back(iter.footscript);
					 break;
			case 11: 
 				 a.emplace_back(iter.headscript);
					 break;
			case 12: 
 				 a.emplace_back(iter.introduce);
					 break;
			case 13: 
 				 a.emplace_back(iter.sitelogo);
					 break;
			case 14: 
 				 a.emplace_back(iter.contactman);
					 break;
			case 15: 
 				 a.emplace_back(iter.phone);
					 break;
			case 16: 
 				 a.emplace_back(iter.mobile);
					 break;
			case 17: 
 				 a.emplace_back(iter.email);
					 break;
			case 18: 
 				 a.emplace_back(iter.bankname);
					 break;
			case 19: 
 				 a.emplace_back(iter.banksn);
					 break;
			case 20: 
 				 a.emplace_back(iter.address);
					 break;
			case 21: 
 				 a.emplace_back(iter.zipnum);
					 break;
			case 22: 
 				 a.emplace_back(iter.taxsn);
					 break;
			case 23: 
 				 a.emplace_back(iter.companyname);
					 break;
			case 24: 
 				 a.emplace_back(iter.linkname);
					 break;
			case 25: 
 				 a.emplace_back(iter.linkmobile);
					 break;
			case 26: 
 				 a.emplace_back(iter.linkaddress);
					 break;
			case 27: 
 				 a.emplace_back(iter.theme);
					 break;
			case 28: 
 				 a.emplace_back(iter.sitepath);
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
 				 a<<std::to_string(iter.sid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.agentid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.languagetype);
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sitename); 
				 }else{
				 a<<iter.sitename;
				 }
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sitedomain); 
				 }else{
				 a<<iter.sitedomain;
				 }
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.metakeys); 
				 }else{
				 a<<iter.metakeys;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.metadesc); 
				 }else{
				 a<<iter.metadesc;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.copyright); 
				 }else{
				 a<<iter.copyright;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.beiansn); 
				 }else{
				 a<<iter.beiansn;
				 }
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.footscript); 
				 }else{
				 a<<iter.footscript;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.headscript); 
				 }else{
				 a<<iter.headscript;
				 }
				 break;
			case 12: 
 				 if(isyinhao){ a<<jsonaddslash(iter.introduce); 
				 }else{
				 a<<iter.introduce;
				 }
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sitelogo); 
				 }else{
				 a<<iter.sitelogo;
				 }
				 break;
			case 14: 
 				 if(isyinhao){ a<<jsonaddslash(iter.contactman); 
				 }else{
				 a<<iter.contactman;
				 }
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.phone); 
				 }else{
				 a<<iter.phone;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.mobile); 
				 }else{
				 a<<iter.mobile;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.email); 
				 }else{
				 a<<iter.email;
				 }
				 break;
			case 18: 
 				 if(isyinhao){ a<<jsonaddslash(iter.bankname); 
				 }else{
				 a<<iter.bankname;
				 }
				 break;
			case 19: 
 				 if(isyinhao){ a<<jsonaddslash(iter.banksn); 
				 }else{
				 a<<iter.banksn;
				 }
				 break;
			case 20: 
 				 if(isyinhao){ a<<jsonaddslash(iter.address); 
				 }else{
				 a<<iter.address;
				 }
				 break;
			case 21: 
 				 if(isyinhao){ a<<jsonaddslash(iter.zipnum); 
				 }else{
				 a<<iter.zipnum;
				 }
				 break;
			case 22: 
 				 if(isyinhao){ a<<jsonaddslash(iter.taxsn); 
				 }else{
				 a<<iter.taxsn;
				 }
				 break;
			case 23: 
 				 if(isyinhao){ a<<jsonaddslash(iter.companyname); 
				 }else{
				 a<<iter.companyname;
				 }
				 break;
			case 24: 
 				 if(isyinhao){ a<<jsonaddslash(iter.linkname); 
				 }else{
				 a<<iter.linkname;
				 }
				 break;
			case 25: 
 				 if(isyinhao){ a<<jsonaddslash(iter.linkmobile); 
				 }else{
				 a<<iter.linkmobile;
				 }
				 break;
			case 26: 
 				 if(isyinhao){ a<<jsonaddslash(iter.linkaddress); 
				 }else{
				 a<<iter.linkaddress;
				 }
				 break;
			case 27: 
 				 if(isyinhao){ a<<jsonaddslash(iter.theme); 
				 }else{
				 a<<iter.theme;
				 }
				 break;
			case 28: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sitepath); 
				 }else{
				 a<<iter.sitepath;
				 }
				 break;
			case 29: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 30: 
 				 a<<std::to_string(iter.created_at);
				 break;
			case 31: 
 				 a<<std::to_string(iter.enddate);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
				 } 
			switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
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
 	 ktemp=iter.sid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.agentid;
	 break;
case 3: 
 	 ktemp=iter.languagetype;
	 break;
case 29: 
 	 ktemp=iter.isopen;
	 break;
case 30: 
 	 ktemp=iter.created_at;
	 break;
case 31: 
 	 ktemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }
 			switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 				 a.emplace(iter.sid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.agentid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.languagetype,iter);
				 break;
			case 29: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 30: 
 				 a.emplace(iter.created_at,iter);
				 break;
			case 31: 
 				 a.emplace(iter.enddate,iter);
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
 				 a.emplace(iter.sitename,iter);
			 break;
			case 5: 
 				 a.emplace(iter.sitedomain,iter);
			 break;
			case 6: 
 				 a.emplace(iter.metakeys,iter);
			 break;
			case 7: 
 				 a.emplace(iter.metadesc,iter);
			 break;
			case 8: 
 				 a.emplace(iter.copyright,iter);
			 break;
			case 9: 
 				 a.emplace(iter.beiansn,iter);
			 break;
			case 10: 
 				 a.emplace(iter.footscript,iter);
			 break;
			case 11: 
 				 a.emplace(iter.headscript,iter);
			 break;
			case 12: 
 				 a.emplace(iter.introduce,iter);
			 break;
			case 13: 
 				 a.emplace(iter.sitelogo,iter);
			 break;
			case 14: 
 				 a.emplace(iter.contactman,iter);
			 break;
			case 15: 
 				 a.emplace(iter.phone,iter);
			 break;
			case 16: 
 				 a.emplace(iter.mobile,iter);
			 break;
			case 17: 
 				 a.emplace(iter.email,iter);
			 break;
			case 18: 
 				 a.emplace(iter.bankname,iter);
			 break;
			case 19: 
 				 a.emplace(iter.banksn,iter);
			 break;
			case 20: 
 				 a.emplace(iter.address,iter);
			 break;
			case 21: 
 				 a.emplace(iter.zipnum,iter);
			 break;
			case 22: 
 				 a.emplace(iter.taxsn,iter);
			 break;
			case 23: 
 				 a.emplace(iter.companyname,iter);
			 break;
			case 24: 
 				 a.emplace(iter.linkname,iter);
			 break;
			case 25: 
 				 a.emplace(iter.linkmobile,iter);
			 break;
			case 26: 
 				 a.emplace(iter.linkaddress,iter);
			 break;
			case 27: 
 				 a.emplace(iter.theme,iter);
			 break;
			case 28: 
 				 a.emplace(iter.sitepath,iter);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
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
 				 ktemp=iter.sid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.agentid;
			 break;
			case 3: 
 				 ktemp=iter.languagetype;
			 break;
			case 29: 
 				 ktemp=iter.isopen;
			 break;
			case 30: 
 				 ktemp=iter.created_at;
			 break;
			case 31: 
 				 ktemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }
 			switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 	 ktemp=iter.sitename;
	 break;
case 5: 
 	 ktemp=iter.sitedomain;
	 break;
case 6: 
 	 ktemp=iter.metakeys;
	 break;
case 7: 
 	 ktemp=iter.metadesc;
	 break;
case 8: 
 	 ktemp=iter.copyright;
	 break;
case 9: 
 	 ktemp=iter.beiansn;
	 break;
case 10: 
 	 ktemp=iter.footscript;
	 break;
case 11: 
 	 ktemp=iter.headscript;
	 break;
case 12: 
 	 ktemp=iter.introduce;
	 break;
case 13: 
 	 ktemp=iter.sitelogo;
	 break;
case 14: 
 	 ktemp=iter.contactman;
	 break;
case 15: 
 	 ktemp=iter.phone;
	 break;
case 16: 
 	 ktemp=iter.mobile;
	 break;
case 17: 
 	 ktemp=iter.email;
	 break;
case 18: 
 	 ktemp=iter.bankname;
	 break;
case 19: 
 	 ktemp=iter.banksn;
	 break;
case 20: 
 	 ktemp=iter.address;
	 break;
case 21: 
 	 ktemp=iter.zipnum;
	 break;
case 22: 
 	 ktemp=iter.taxsn;
	 break;
case 23: 
 	 ktemp=iter.companyname;
	 break;
case 24: 
 	 ktemp=iter.linkname;
	 break;
case 25: 
 	 ktemp=iter.linkmobile;
	 break;
case 26: 
 	 ktemp=iter.linkaddress;
	 break;
case 27: 
 	 ktemp=iter.theme;
	 break;
case 28: 
 	 ktemp=iter.sitepath;
	 break;
	  }
 switch(vpos){
case 4: 
 	 vtemp=iter.sitename;
	 break;
case 5: 
 	 vtemp=iter.sitedomain;
	 break;
case 6: 
 	 vtemp=iter.metakeys;
	 break;
case 7: 
 	 vtemp=iter.metadesc;
	 break;
case 8: 
 	 vtemp=iter.copyright;
	 break;
case 9: 
 	 vtemp=iter.beiansn;
	 break;
case 10: 
 	 vtemp=iter.footscript;
	 break;
case 11: 
 	 vtemp=iter.headscript;
	 break;
case 12: 
 	 vtemp=iter.introduce;
	 break;
case 13: 
 	 vtemp=iter.sitelogo;
	 break;
case 14: 
 	 vtemp=iter.contactman;
	 break;
case 15: 
 	 vtemp=iter.phone;
	 break;
case 16: 
 	 vtemp=iter.mobile;
	 break;
case 17: 
 	 vtemp=iter.email;
	 break;
case 18: 
 	 vtemp=iter.bankname;
	 break;
case 19: 
 	 vtemp=iter.banksn;
	 break;
case 20: 
 	 vtemp=iter.address;
	 break;
case 21: 
 	 vtemp=iter.zipnum;
	 break;
case 22: 
 	 vtemp=iter.taxsn;
	 break;
case 23: 
 	 vtemp=iter.companyname;
	 break;
case 24: 
 	 vtemp=iter.linkname;
	 break;
case 25: 
 	 vtemp=iter.linkmobile;
	 break;
case 26: 
 	 vtemp=iter.linkaddress;
	 break;
case 27: 
 	 vtemp=iter.theme;
	 break;
case 28: 
 	 vtemp=iter.sitepath;
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
 	 a.emplace_back(iter.sid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.agentid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.languagetype,iter);
	 break;
case 29: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 30: 
 	 a.emplace_back(iter.created_at,iter);
	 break;
case 31: 
 	 a.emplace_back(iter.enddate,iter);
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
 	 a.emplace_back(iter.sitename,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.sitedomain,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.metakeys,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.metadesc,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.copyright,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.beiansn,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.footscript,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.headscript,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.introduce,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.sitelogo,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.contactman,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.phone,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.mobile,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.email,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.bankname,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.banksn,iter);
	 break;
case 20: 
 	 a.emplace_back(iter.address,iter);
	 break;
case 21: 
 	 a.emplace_back(iter.zipnum,iter);
	 break;
case 22: 
 	 a.emplace_back(iter.taxsn,iter);
	 break;
case 23: 
 	 a.emplace_back(iter.companyname,iter);
	 break;
case 24: 
 	 a.emplace_back(iter.linkname,iter);
	 break;
case 25: 
 	 a.emplace_back(iter.linkmobile,iter);
	 break;
case 26: 
 	 a.emplace_back(iter.linkaddress,iter);
	 break;
case 27: 
 	 a.emplace_back(iter.theme,iter);
	 break;
case 28: 
 	 a.emplace_back(iter.sitepath,iter);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
				 break;
			 }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
				 break;
			  }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.sid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.agentid;
				 break;
			case 3: 
 				 vtemp=iter.languagetype;
				 break;
			case 29: 
 				 vtemp=iter.isopen;
				 break;
			case 30: 
 				 vtemp=iter.created_at;
				 break;
			case 31: 
 				 vtemp=iter.enddate;
				 break;
			 }

			switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp][vtemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp][vtemp].emplace_back(iter.enddate);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 vtemp=iter.sitename;
				 break;
			case 5: 
 				 vtemp=iter.sitedomain;
				 break;
			case 6: 
 				 vtemp=iter.metakeys;
				 break;
			case 7: 
 				 vtemp=iter.metadesc;
				 break;
			case 8: 
 				 vtemp=iter.copyright;
				 break;
			case 9: 
 				 vtemp=iter.beiansn;
				 break;
			case 10: 
 				 vtemp=iter.footscript;
				 break;
			case 11: 
 				 vtemp=iter.headscript;
				 break;
			case 12: 
 				 vtemp=iter.introduce;
				 break;
			case 13: 
 				 vtemp=iter.sitelogo;
				 break;
			case 14: 
 				 vtemp=iter.contactman;
				 break;
			case 15: 
 				 vtemp=iter.phone;
				 break;
			case 16: 
 				 vtemp=iter.mobile;
				 break;
			case 17: 
 				 vtemp=iter.email;
				 break;
			case 18: 
 				 vtemp=iter.bankname;
				 break;
			case 19: 
 				 vtemp=iter.banksn;
				 break;
			case 20: 
 				 vtemp=iter.address;
				 break;
			case 21: 
 				 vtemp=iter.zipnum;
				 break;
			case 22: 
 				 vtemp=iter.taxsn;
				 break;
			case 23: 
 				 vtemp=iter.companyname;
				 break;
			case 24: 
 				 vtemp=iter.linkname;
				 break;
			case 25: 
 				 vtemp=iter.linkmobile;
				 break;
			case 26: 
 				 vtemp=iter.linkaddress;
				 break;
			case 27: 
 				 vtemp=iter.theme;
				 break;
			case 28: 
 				 vtemp=iter.sitepath;
				 break;
			  }

			 switch(dpos){
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp][vtemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp][vtemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 a[ktemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp].emplace_back(iter.enddate);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 a[ktemp].emplace_back(iter.sitename);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.sitedomain);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.metakeys);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.metadesc);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.copyright);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.beiansn);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.footscript);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.headscript);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.sitelogo);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.contactman);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.phone);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.mobile);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.email);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.bankname);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.banksn);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.zipnum);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.taxsn);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.companyname);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.linkname);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.linkmobile);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.linkaddress);
				 break;
			case 27: 
 				 a[ktemp].emplace_back(iter.theme);
				 break;
			case 28: 
 				 a[ktemp].emplace_back(iter.sitepath);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.sid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.agentid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.languagetype);
				 break;
			case 29: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 30: 
 				 a[ktemp].emplace_back(iter.created_at);
				 break;
			case 31: 
 				 a[ktemp].emplace_back(iter.enddate);
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
 				 a[iter.sid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.agentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.languagetype].emplace_back(iter);
				 break;
			case 29: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 30: 
 				 a[iter.created_at].emplace_back(iter);
				 break;
			case 31: 
 				 a[iter.enddate].emplace_back(iter);
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
 				 a[iter.sitename].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.sitedomain].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.metakeys].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.metadesc].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.copyright].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.beiansn].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.footscript].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.headscript].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.introduce].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.sitelogo].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.contactman].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.phone].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.mobile].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.email].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.bankname].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.banksn].emplace_back(iter);
				 break;
			case 20: 
 				 a[iter.address].emplace_back(iter);
				 break;
			case 21: 
 				 a[iter.zipnum].emplace_back(iter);
				 break;
			case 22: 
 				 a[iter.taxsn].emplace_back(iter);
				 break;
			case 23: 
 				 a[iter.companyname].emplace_back(iter);
				 break;
			case 24: 
 				 a[iter.linkname].emplace_back(iter);
				 break;
			case 25: 
 				 a[iter.linkmobile].emplace_back(iter);
				 break;
			case 26: 
 				 a[iter.linkaddress].emplace_back(iter);
				 break;
			case 27: 
 				 a[iter.theme].emplace_back(iter);
				 break;
			case 28: 
 				 a[iter.sitepath].emplace_back(iter);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
			 }

			 switch(vpos){
			case 4: 
 				 a[ktemp][iter.sitename].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sitedomain].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.metakeys].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.metadesc].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.copyright].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.beiansn].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.footscript].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.headscript].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.sitelogo].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.contactman].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.phone].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.bankname].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.banksn].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.zipnum].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.taxsn].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.companyname].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.linkname].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.linkmobile].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.linkaddress].emplace_back(iter);
				 break;
			case 27: 
 				 a[ktemp][iter.theme].emplace_back(iter);
				 break;
			case 28: 
 				 a[ktemp][iter.sitepath].emplace_back(iter);
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
 				 ktemp=iter.sitename;
				 break;
			case 5: 
 				 ktemp=iter.sitedomain;
				 break;
			case 6: 
 				 ktemp=iter.metakeys;
				 break;
			case 7: 
 				 ktemp=iter.metadesc;
				 break;
			case 8: 
 				 ktemp=iter.copyright;
				 break;
			case 9: 
 				 ktemp=iter.beiansn;
				 break;
			case 10: 
 				 ktemp=iter.footscript;
				 break;
			case 11: 
 				 ktemp=iter.headscript;
				 break;
			case 12: 
 				 ktemp=iter.introduce;
				 break;
			case 13: 
 				 ktemp=iter.sitelogo;
				 break;
			case 14: 
 				 ktemp=iter.contactman;
				 break;
			case 15: 
 				 ktemp=iter.phone;
				 break;
			case 16: 
 				 ktemp=iter.mobile;
				 break;
			case 17: 
 				 ktemp=iter.email;
				 break;
			case 18: 
 				 ktemp=iter.bankname;
				 break;
			case 19: 
 				 ktemp=iter.banksn;
				 break;
			case 20: 
 				 ktemp=iter.address;
				 break;
			case 21: 
 				 ktemp=iter.zipnum;
				 break;
			case 22: 
 				 ktemp=iter.taxsn;
				 break;
			case 23: 
 				 ktemp=iter.companyname;
				 break;
			case 24: 
 				 ktemp=iter.linkname;
				 break;
			case 25: 
 				 ktemp=iter.linkmobile;
				 break;
			case 26: 
 				 ktemp=iter.linkaddress;
				 break;
			case 27: 
 				 ktemp=iter.theme;
				 break;
			case 28: 
 				 ktemp=iter.sitepath;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.sid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.agentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.languagetype].emplace_back(iter);
				 break;
			case 29: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 30: 
 				 a[ktemp][iter.created_at].emplace_back(iter);
				 break;
			case 31: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.sid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.agentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.languagetype].emplace_back(iter);
				 break;
			case 29: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 30: 
 				 a[ktemp][iter.created_at].emplace_back(iter);
				 break;
			case 31: 
 				 a[ktemp][iter.enddate].emplace_back(iter);
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
 				 ktemp=iter.sid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.agentid;
				 break;
			case 3: 
 				 ktemp=iter.languagetype;
				 break;
			case 29: 
 				 ktemp=iter.isopen;
				 break;
			case 30: 
 				 ktemp=iter.created_at;
				 break;
			case 31: 
 				 ktemp=iter.enddate;
				 break;
			  }

			 switch(vpos){
			case 4: 
 				 a[ktemp][iter.sitename].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.sitedomain].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.metakeys].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.metadesc].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.copyright].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.beiansn].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.footscript].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.headscript].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.sitelogo].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.contactman].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.phone].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.mobile].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.email].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.bankname].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.banksn].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.zipnum].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.taxsn].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.companyname].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.linkname].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.linkmobile].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.linkaddress].emplace_back(iter);
				 break;
			case 27: 
 				 a[ktemp][iter.theme].emplace_back(iter);
				 break;
			case 28: 
 				 a[ktemp][iter.sitepath].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   