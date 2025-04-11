#ifndef ORM_CMS_XMEETBASEMATA_H
#define ORM_CMS_XMEETBASEMATA_H
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

struct xmeetbase
{
    struct meta{
     unsigned  int  xmeetid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  xpjid = 0; ///**/
 unsigned  int  xtaskid = 0; ///**/
 unsigned  int  adminid = 0; ///**/
 std::string  title = ""; ///**/
 std::string  zhuchi = ""; ///**/
 std::string  jilu = ""; ///**/
 std::string  meetnotice = ""; ///**/
 std::string  meetfiles = ""; ///**/
 std::string  address = ""; ///**/
 std::string  meettime = ""; ///**/
 unsigned  int  regdate = 0; ///**/
 unsigned  int  expecttime = 0; ///**/
 unsigned  int  endtime = 0; ///**/
 std::string  presents = ""; ///**/
 std::string  content = ""; ///**/
 std::string  postresult = ""; ///**/
 std::string  postfiles = ""; ///**/
 std::string  jiluphoto = ""; ///**/
 } data;
 std::vector<xmeetbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<xmeetbase::meta>::iterator begin(){     return record.begin(); }
std::vector<xmeetbase::meta>::iterator end(){     return record.end(); }
std::vector<xmeetbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xmeetbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,20> col_names={"xmeetid","userid","xpjid","xtaskid","adminid","title","zhuchi","jilu","meetnotice","meetfiles","address","meettime","regdate","expecttime","endtime","presents","content","postresult","postfiles","jiluphoto"};
static constexpr std::array<unsigned char,20> col_types={3,3,3,3,3,253,253,253,252,253,253,253,3,3,3,253,252,252,252,253};
static constexpr std::array<unsigned char,20> col_length={0,0,0,0,0,120,120,120,0,0,120,120,0,0,0,0,0,0,0,0};
static constexpr std::array<unsigned char,20> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="xmeet";
static constexpr std::string_view modelname="Xmeet";

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
 if(colpospppc=='d'){ return 4; }
 if(colpospppc=='s'){ return 10; }
   	 break;
 }
 break;
case 'c':
   	 return 16;
break;
case 'e':
 switch(coln.size()){  
case 7:
   	 return 14;
break;
case 10:
   	 return 13;
break;
 }
 break;
case 'j':
 switch(coln.size()){  
case 4:
   	 return 7;
break;
case 9:
   	 return 19;
break;
 }
 break;
case 'm':
 switch(coln.size()){  
case 8:
   	 return 11;
break;
case 9:
   	 return 9;
break;
case 10:
   	 return 8;
break;
 }
 break;
case 'p':
 switch(coln.size()){  
case 8:
   	 return 15;
break;
case 9:
   	 return 18;
break;
case 10:
   	 return 17;
break;
 }
 break;
case 'r':
   	 return 12;
break;
case 't':
   	 return 5;
break;
case 'u':
   	 return 1;
break;
case 'x':
 switch(coln.size()){  
case 5:
   	 return 2;
break;
case 7:
 if(coln.size()>1&&(coln[1]=='m'||coln[1]=='M')){ return 0; }
 if(coln.size()>1&&(coln[1]=='t'||coln[1]=='T')){ return 3; }
   	 break;
 }
 break;
case 'z':
   	 return 6;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "xmeetid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xmeetbase::meta metatemp;    
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

        if(data.xmeetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.zhuchi)<<"'";
tempsql<<",'"<<stringaddslash(data.jilu)<<"'";
tempsql<<",'"<<stringaddslash(data.meetnotice)<<"'";
tempsql<<",'"<<stringaddslash(data.meetfiles)<<"'";
tempsql<<",'"<<stringaddslash(data.address)<<"'";
tempsql<<",'"<<stringaddslash(data.meettime)<<"'";
if(data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.endtime);
}
tempsql<<",'"<<stringaddslash(data.presents)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
tempsql<<",'"<<stringaddslash(data.postresult)<<"'";
tempsql<<",'"<<stringaddslash(data.postfiles)<<"'";
tempsql<<",'"<<stringaddslash(data.jiluphoto)<<"'";
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

        if(insert_data.xmeetid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.xmeetid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.xpjid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xpjid);
}
if(insert_data.xtaskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.xtaskid);
}
if(insert_data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminid);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.zhuchi)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.jilu)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meetnotice)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meetfiles)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.address)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.meettime)<<"'";
if(insert_data.regdate==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.regdate);
}
if(insert_data.expecttime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.expecttime);
}
if(insert_data.endtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.endtime);
}
tempsql<<",'"<<stringaddslash(insert_data.presents)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.postresult)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.postfiles)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.jiluphoto)<<"'";
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


            	if(insert_data[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].xmeetid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xpjid);
	}
	if(insert_data[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].xtaskid);
	}
	if(insert_data[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].zhuchi)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].jilu)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meetnotice)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meetfiles)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].address)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].meettime)<<"'";
	if(insert_data[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].regdate);
	}
	if(insert_data[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].expecttime);
	}
	if(insert_data[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].endtime);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].presents)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].postresult)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].postfiles)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].jiluphoto)<<"'";
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

        if(data.xmeetid==0){
	tempsql<<"`xmeetid`=0";
 }else{ 
	tempsql<<"`xmeetid`="<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",`xpjid`=0";
 }else{ 
	tempsql<<",`xpjid`="<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",`xtaskid`=0";
 }else{ 
	tempsql<<",`xtaskid`="<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",`adminid`=0";
 }else{ 
	tempsql<<",`adminid`="<<std::to_string(data.adminid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`zhuchi`='"<<stringaddslash(data.zhuchi)<<"'";
tempsql<<",`jilu`='"<<stringaddslash(data.jilu)<<"'";
tempsql<<",`meetnotice`='"<<stringaddslash(data.meetnotice)<<"'";
tempsql<<",`meetfiles`='"<<stringaddslash(data.meetfiles)<<"'";
tempsql<<",`address`='"<<stringaddslash(data.address)<<"'";
tempsql<<",`meettime`='"<<stringaddslash(data.meettime)<<"'";
if(data.regdate==0){
	tempsql<<",`regdate`=0";
 }else{ 
	tempsql<<",`regdate`="<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",`expecttime`=0";
 }else{ 
	tempsql<<",`expecttime`="<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",`endtime`=0";
 }else{ 
	tempsql<<",`endtime`="<<std::to_string(data.endtime);
}
tempsql<<",`presents`='"<<stringaddslash(data.presents)<<"'";
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
tempsql<<",`postresult`='"<<stringaddslash(data.postresult)<<"'";
tempsql<<",`postfiles`='"<<stringaddslash(data.postfiles)<<"'";
tempsql<<",`jiluphoto`='"<<stringaddslash(data.jiluphoto)<<"'";
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
if(data.xmeetid==0){
	tempsql<<"`xmeetid`=0";
 }else{ 
	tempsql<<"`xmeetid`="<<std::to_string(data.xmeetid);
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
if(data.xpjid==0){
	tempsql<<"`xpjid`=0";
 }else{ 
	tempsql<<"`xpjid`="<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.xtaskid==0){
	tempsql<<"`xtaskid`=0";
 }else{ 
	tempsql<<"`xtaskid`="<<std::to_string(data.xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`zhuchi`='"<<stringaddslash(data.zhuchi)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`jilu`='"<<stringaddslash(data.jilu)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meetnotice`='"<<stringaddslash(data.meetnotice)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meetfiles`='"<<stringaddslash(data.meetfiles)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`address`='"<<stringaddslash(data.address)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`meettime`='"<<stringaddslash(data.meettime)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"`regdate`=0";
 }else{ 
	tempsql<<"`regdate`="<<std::to_string(data.regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.expecttime==0){
	tempsql<<"`expecttime`=0";
 }else{ 
	tempsql<<"`expecttime`="<<std::to_string(data.expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.endtime==0){
	tempsql<<"`endtime`=0";
 }else{ 
	tempsql<<"`endtime`="<<std::to_string(data.endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`presents`='"<<stringaddslash(data.presents)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`postresult`='"<<stringaddslash(data.postresult)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`postfiles`='"<<stringaddslash(data.postfiles)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`jiluphoto`='"<<stringaddslash(data.jiluphoto)<<"'";
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
            	if(record[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xmeetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtaskid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zhuchi)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jilu)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetnotice)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meettime)<<"'";
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expecttime);
	}
	if(record[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].endtime);
	}
	tempsql<<",'"<<stringaddslash(record[i].presents)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postresult)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jiluphoto)<<"'";
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
            	if(record[i].xmeetid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].xmeetid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].xpjid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xpjid);
	}
	if(record[i].xtaskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].xtaskid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].zhuchi)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jilu)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetnotice)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meetfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].address)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].meettime)<<"'";
	if(record[i].regdate==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].regdate);
	}
	if(record[i].expecttime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].expecttime);
	}
	if(record[i].endtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].endtime);
	}
	tempsql<<",'"<<stringaddslash(record[i].presents)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postresult)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].postfiles)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].jiluphoto)<<"'";
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
if(data.xmeetid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xmeetid));
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
if(data.xpjid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xpjid));
}
 break;
 case 3:
if(data.xtaskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.xtaskid));
}
 break;
 case 4:
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 5:
	temparray.push_back(data.title);
 break;
 case 6:
	temparray.push_back(data.zhuchi);
 break;
 case 7:
	temparray.push_back(data.jilu);
 break;
 case 8:
	temparray.push_back(data.meetnotice);
 break;
 case 9:
	temparray.push_back(data.meetfiles);
 break;
 case 10:
	temparray.push_back(data.address);
 break;
 case 11:
	temparray.push_back(data.meettime);
 break;
 case 12:
if(data.regdate==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.regdate));
}
 break;
 case 13:
if(data.expecttime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.expecttime));
}
 break;
 case 14:
if(data.endtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.endtime));
}
 break;
 case 15:
	temparray.push_back(data.presents);
 break;
 case 16:
	temparray.push_back(data.content);
 break;
 case 17:
	temparray.push_back(data.postresult);
 break;
 case 18:
	temparray.push_back(data.postfiles);
 break;
 case 19:
	temparray.push_back(data.jiluphoto);
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
if(data.xmeetid==0){
	tempsql.insert({"xmeetid","0"});
 }else{ 
	tempsql.insert({"xmeetid",std::to_string(data.xmeetid)});
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
if(data.xpjid==0){
	tempsql.insert({"xpjid","0"});
 }else{ 
	tempsql.insert({"xpjid",std::to_string(data.xpjid)});
}
 break;
 case 3:
if(data.xtaskid==0){
	tempsql.insert({"xtaskid","0"});
 }else{ 
	tempsql.insert({"xtaskid",std::to_string(data.xtaskid)});
}
 break;
 case 4:
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 5:
	tempsql.insert({"title",data.title});
 break;
 case 6:
	tempsql.insert({"zhuchi",data.zhuchi});
 break;
 case 7:
	tempsql.insert({"jilu",data.jilu});
 break;
 case 8:
	tempsql.insert({"meetnotice",data.meetnotice});
 break;
 case 9:
	tempsql.insert({"meetfiles",data.meetfiles});
 break;
 case 10:
	tempsql.insert({"address",data.address});
 break;
 case 11:
	tempsql.insert({"meettime",data.meettime});
 break;
 case 12:
if(data.regdate==0){
	tempsql.insert({"regdate","0"});
 }else{ 
	tempsql.insert({"regdate",std::to_string(data.regdate)});
}
 break;
 case 13:
if(data.expecttime==0){
	tempsql.insert({"expecttime","0"});
 }else{ 
	tempsql.insert({"expecttime",std::to_string(data.expecttime)});
}
 break;
 case 14:
if(data.endtime==0){
	tempsql.insert({"endtime","0"});
 }else{ 
	tempsql.insert({"endtime",std::to_string(data.endtime)});
}
 break;
 case 15:
	tempsql.insert({"presents",data.presents});
 break;
 case 16:
	tempsql.insert({"content",data.content});
 break;
 case 17:
	tempsql.insert({"postresult",data.postresult});
 break;
 case 18:
	tempsql.insert({"postfiles",data.postfiles});
 break;
 case 19:
	tempsql.insert({"jiluphoto",data.jiluphoto});
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
if(data.xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(data.xmeetid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.xpjid==0){
	tempsql<<",\"xpjid\":0";
 }else{ 
	tempsql<<",\"xpjid\":"<<std::to_string(data.xpjid);
}
if(data.xtaskid==0){
	tempsql<<",\"xtaskid\":0";
 }else{ 
	tempsql<<",\"xtaskid\":"<<std::to_string(data.xtaskid);
}
if(data.adminid==0){
	tempsql<<",\"adminid\":0";
 }else{ 
	tempsql<<",\"adminid\":"<<std::to_string(data.adminid);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"zhuchi\":\""<<http::utf8_to_jsonstring(data.zhuchi);
tempsql<<"\"";
tempsql<<",\"jilu\":\""<<http::utf8_to_jsonstring(data.jilu);
tempsql<<"\"";
tempsql<<",\"meetnotice\":\""<<http::utf8_to_jsonstring(data.meetnotice);
tempsql<<"\"";
tempsql<<",\"meetfiles\":\""<<http::utf8_to_jsonstring(data.meetfiles);
tempsql<<"\"";
tempsql<<",\"address\":\""<<http::utf8_to_jsonstring(data.address);
tempsql<<"\"";
tempsql<<",\"meettime\":\""<<http::utf8_to_jsonstring(data.meettime);
tempsql<<"\"";
if(data.regdate==0){
	tempsql<<",\"regdate\":0";
 }else{ 
	tempsql<<",\"regdate\":"<<std::to_string(data.regdate);
}
if(data.expecttime==0){
	tempsql<<",\"expecttime\":0";
 }else{ 
	tempsql<<",\"expecttime\":"<<std::to_string(data.expecttime);
}
if(data.endtime==0){
	tempsql<<",\"endtime\":0";
 }else{ 
	tempsql<<",\"endtime\":"<<std::to_string(data.endtime);
}
tempsql<<",\"presents\":\""<<http::utf8_to_jsonstring(data.presents);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"postresult\":\""<<http::utf8_to_jsonstring(data.postresult);
tempsql<<"\"";
tempsql<<",\"postfiles\":\""<<http::utf8_to_jsonstring(data.postfiles);
tempsql<<"\"";
tempsql<<",\"jiluphoto\":\""<<http::utf8_to_jsonstring(data.jiluphoto);
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
if(data.xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(data.xmeetid);
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
if(data.xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(data.xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(data.xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(data.zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(data.jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(data.meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(data.meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(data.address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(data.meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(data.regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(data.expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(data.endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(data.presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(data.postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(data.postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(data.jiluphoto)<<"\"";
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
        xmeetbase::meta metatemp; 
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
			data.xmeetid=std::stoul(set_value_name);
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=std::stoul(set_value_name);
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=std::stoul(set_value_name);
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi.append(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu.append(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice.append(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles.append(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
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
			data.meettime.append(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=std::stoul(set_value_name);
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=std::stoul(set_value_name);
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=std::stoul(set_value_name);
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents.append(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content.append(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult.append(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles.append(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto.append(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
			data.xmeetid=set_value_name;
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=set_value_name;
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi=std::to_string(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu=std::to_string(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice=std::to_string(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
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
			data.meettime=std::to_string(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=set_value_name;
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=set_value_name;
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents=std::to_string(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult=std::to_string(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto=std::to_string(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
			data.xmeetid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xmeetid=0;
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
			data.xpjid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xpjid=0;
			 }
			break;
		case 3:
		 try{
			data.xtaskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.xtaskid=0;
			 }
			break;
		case 4:
		 try{
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 5:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 6:
		 try{
			data.zhuchi=std::to_string(set_value_name);
		}catch (...) { 
			data.zhuchi.clear();
			 }
			break;
		case 7:
		 try{
			data.jilu=std::to_string(set_value_name);
		}catch (...) { 
			data.jilu.clear();
			 }
			break;
		case 8:
		 try{
			data.meetnotice=std::to_string(set_value_name);
		}catch (...) { 
			data.meetnotice.clear();
			 }
			break;
		case 9:
		 try{
			data.meetfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.meetfiles.clear();
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
			data.meettime=std::to_string(set_value_name);
		}catch (...) { 
			data.meettime.clear();
			 }
			break;
		case 12:
		 try{
			data.regdate=(unsigned int)set_value_name;
		}catch (...) { 
			data.regdate=0;
			 }
			break;
		case 13:
		 try{
			data.expecttime=(unsigned int)set_value_name;
		}catch (...) { 
			data.expecttime=0;
			 }
			break;
		case 14:
		 try{
			data.endtime=(unsigned int)set_value_name;
		}catch (...) { 
			data.endtime=0;
			 }
			break;
		case 15:
		 try{
			data.presents=std::to_string(set_value_name);
		}catch (...) { 
			data.presents.clear();
			 }
			break;
		case 16:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 17:
		 try{
			data.postresult=std::to_string(set_value_name);
		}catch (...) { 
			data.postresult.clear();
			 }
			break;
		case 18:
		 try{
			data.postfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.postfiles.clear();
			 }
			break;
		case 19:
		 try{
			data.jiluphoto=std::to_string(set_value_name);
		}catch (...) { 
			data.jiluphoto.clear();
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
if(record[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(record[n].xmeetid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(record[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(record[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(record[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(record[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(record[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(record[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(record[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(record[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(record[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(record[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(record[n].jiluphoto)<<"\"";
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
if(record[n].xmeetid==0){
	tempsql<<"\"xmeetid\":0";
 }else{ 
	tempsql<<"\"xmeetid\":"<<std::to_string(record[n].xmeetid);
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
if(record[n].xpjid==0){
	tempsql<<"\"xpjid\":0";
 }else{ 
	tempsql<<"\"xpjid\":"<<std::to_string(record[n].xpjid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].xtaskid==0){
	tempsql<<"\"xtaskid\":0";
 }else{ 
	tempsql<<"\"xtaskid\":"<<std::to_string(record[n].xtaskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"zhuchi\":\""<<http::utf8_to_jsonstring(record[n].zhuchi)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jilu\":\""<<http::utf8_to_jsonstring(record[n].jilu)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetnotice\":\""<<http::utf8_to_jsonstring(record[n].meetnotice)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meetfiles\":\""<<http::utf8_to_jsonstring(record[n].meetfiles)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"address\":\""<<http::utf8_to_jsonstring(record[n].address)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"meettime\":\""<<http::utf8_to_jsonstring(record[n].meettime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].regdate==0){
	tempsql<<"\"regdate\":0";
 }else{ 
	tempsql<<"\"regdate\":"<<std::to_string(record[n].regdate);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].expecttime==0){
	tempsql<<"\"expecttime\":0";
 }else{ 
	tempsql<<"\"expecttime\":"<<std::to_string(record[n].expecttime);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].endtime==0){
	tempsql<<"\"endtime\":0";
 }else{ 
	tempsql<<"\"endtime\":"<<std::to_string(record[n].endtime);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"presents\":\""<<http::utf8_to_jsonstring(record[n].presents)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postresult\":\""<<http::utf8_to_jsonstring(record[n].postresult)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"postfiles\":\""<<http::utf8_to_jsonstring(record[n].postfiles)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"jiluphoto\":\""<<http::utf8_to_jsonstring(record[n].jiluphoto)<<"\"";
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
   long long getPK(){  return data.xmeetid; } 
 void setPK(long long val){  data.xmeetid=val;} 
 unsigned  int  getXmeetid(){  return data.xmeetid; } 
 void setXmeetid( unsigned  int  val){  data.xmeetid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getXpjid(){  return data.xpjid; } 
 void setXpjid( unsigned  int  val){  data.xpjid=val;} 

 unsigned  int  getXtaskid(){  return data.xtaskid; } 
 void setXtaskid( unsigned  int  val){  data.xtaskid=val;} 

 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getZhuchi(){  return data.zhuchi; } 
 std::string & getRefZhuchi(){  return std::ref(data.zhuchi); } 
 void setZhuchi( std::string  &val){  data.zhuchi=val;} 
 void setZhuchi(std::string_view val){  data.zhuchi=val;} 

 std::string  getJilu(){  return data.jilu; } 
 std::string & getRefJilu(){  return std::ref(data.jilu); } 
 void setJilu( std::string  &val){  data.jilu=val;} 
 void setJilu(std::string_view val){  data.jilu=val;} 

 std::string  getMeetnotice(){  return data.meetnotice; } 
 std::string & getRefMeetnotice(){  return std::ref(data.meetnotice); } 
 void setMeetnotice( std::string  &val){  data.meetnotice=val;} 
 void setMeetnotice(std::string_view val){  data.meetnotice=val;} 

 std::string  getMeetfiles(){  return data.meetfiles; } 
 std::string & getRefMeetfiles(){  return std::ref(data.meetfiles); } 
 void setMeetfiles( std::string  &val){  data.meetfiles=val;} 
 void setMeetfiles(std::string_view val){  data.meetfiles=val;} 

 std::string  getAddress(){  return data.address; } 
 std::string & getRefAddress(){  return std::ref(data.address); } 
 void setAddress( std::string  &val){  data.address=val;} 
 void setAddress(std::string_view val){  data.address=val;} 

 std::string  getMeettime(){  return data.meettime; } 
 std::string & getRefMeettime(){  return std::ref(data.meettime); } 
 void setMeettime( std::string  &val){  data.meettime=val;} 
 void setMeettime(std::string_view val){  data.meettime=val;} 

 unsigned  int  getRegdate(){  return data.regdate; } 
 void setRegdate( unsigned  int  val){  data.regdate=val;} 

 unsigned  int  getExpecttime(){  return data.expecttime; } 
 void setExpecttime( unsigned  int  val){  data.expecttime=val;} 

 unsigned  int  getEndtime(){  return data.endtime; } 
 void setEndtime( unsigned  int  val){  data.endtime=val;} 

 std::string  getPresents(){  return data.presents; } 
 std::string & getRefPresents(){  return std::ref(data.presents); } 
 void setPresents( std::string  &val){  data.presents=val;} 
 void setPresents(std::string_view val){  data.presents=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 std::string  getPostresult(){  return data.postresult; } 
 std::string & getRefPostresult(){  return std::ref(data.postresult); } 
 void setPostresult( std::string  &val){  data.postresult=val;} 
 void setPostresult(std::string_view val){  data.postresult=val;} 

 std::string  getPostfiles(){  return data.postfiles; } 
 std::string & getRefPostfiles(){  return std::ref(data.postfiles); } 
 void setPostfiles( std::string  &val){  data.postfiles=val;} 
 void setPostfiles(std::string_view val){  data.postfiles=val;} 

 std::string  getJiluphoto(){  return data.jiluphoto; } 
 std::string & getRefJiluphoto(){  return std::ref(data.jiluphoto); } 
 void setJiluphoto( std::string  &val){  data.jiluphoto=val;} 
 void setJiluphoto(std::string_view val){  data.jiluphoto=val;} 

xmeetbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
xmeetbase::meta getData(){
 	 return data; 
} 
std::vector<xmeetbase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="zhuchi")
		{
			return data.zhuchi;
		}
		 if(key_name=="jilu")
		{
			return data.jilu;
		}
		 if(key_name=="meetnotice")
		{
			return data.meetnotice;
		}
		 if(key_name=="meetfiles")
		{
			return data.meetfiles;
		}
		 if(key_name=="address")
		{
			return data.address;
		}
		 if(key_name=="meettime")
		{
			return data.meettime;
		}
		 if(key_name=="presents")
		{
			return data.presents;
		}
		 if(key_name=="content")
		{
			return data.content;
		}
		 if(key_name=="postresult")
		{
			return data.postresult;
		}
		 if(key_name=="postfiles")
		{
			return data.postfiles;
		}
		 if(key_name=="jiluphoto")
		{
			return data.jiluphoto;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="xmeetid")
		{
			return data.xmeetid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="xpjid")
		{
			return data.xpjid;
		}
		 if(key_name=="xtaskid")
		{
			return data.xtaskid;
		}
		 if(key_name=="adminid")
		{
			return data.adminid;
		}
		 if(key_name=="regdate")
		{
			return data.regdate;
		}
		 if(key_name=="expecttime")
		{
			return data.expecttime;
		}
		 if(key_name=="endtime")
		{
			return data.endtime;
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
 				 a.emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a.emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a.emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a.emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a.emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a.emplace_back(iter.endtime);
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
 				 return data.xmeetid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.xpjid;
				 break;
			case 3: 
 				 return data.xtaskid;
				 break;
			case 4: 
 				 return data.adminid;
				 break;
			case 12: 
 				 return data.regdate;
				 break;
			case 13: 
 				 return data.expecttime;
				 break;
			case 14: 
 				 return data.endtime;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] xmeetbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.xmeetid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.xpjid;
				 break;
			case 3: 
 				 return iter.xtaskid;
				 break;
			case 4: 
 				 return iter.adminid;
				 break;
			case 12: 
 				 return iter.regdate;
				 break;
			case 13: 
 				 return iter.expecttime;
				 break;
			case 14: 
 				 return iter.endtime;
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
            T getVal([[maybe_unused]] xmeetbase::meta & iter,std::string keyname)
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
 				 return data.title;
				 break;
			case 6: 
 				 return data.zhuchi;
				 break;
			case 7: 
 				 return data.jilu;
				 break;
			case 8: 
 				 return data.meetnotice;
				 break;
			case 9: 
 				 return data.meetfiles;
				 break;
			case 10: 
 				 return data.address;
				 break;
			case 11: 
 				 return data.meettime;
				 break;
			case 15: 
 				 return data.presents;
				 break;
			case 16: 
 				 return data.content;
				 break;
			case 17: 
 				 return data.postresult;
				 break;
			case 18: 
 				 return data.postfiles;
				 break;
			case 19: 
 				 return data.jiluphoto;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] xmeetbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 5: 
 				 return iter.title;
				 break;
			case 6: 
 				 return iter.zhuchi;
				 break;
			case 7: 
 				 return iter.jilu;
				 break;
			case 8: 
 				 return iter.meetnotice;
				 break;
			case 9: 
 				 return iter.meetfiles;
				 break;
			case 10: 
 				 return iter.address;
				 break;
			case 11: 
 				 return iter.meettime;
				 break;
			case 15: 
 				 return iter.presents;
				 break;
			case 16: 
 				 return iter.content;
				 break;
			case 17: 
 				 return iter.postresult;
				 break;
			case 18: 
 				 return iter.postfiles;
				 break;
			case 19: 
 				 return iter.jiluphoto;
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
 				 a.emplace_back(iter.title);
					 break;
			case 6: 
 				 a.emplace_back(iter.zhuchi);
					 break;
			case 7: 
 				 a.emplace_back(iter.jilu);
					 break;
			case 8: 
 				 a.emplace_back(iter.meetnotice);
					 break;
			case 9: 
 				 a.emplace_back(iter.meetfiles);
					 break;
			case 10: 
 				 a.emplace_back(iter.address);
					 break;
			case 11: 
 				 a.emplace_back(iter.meettime);
					 break;
			case 15: 
 				 a.emplace_back(iter.presents);
					 break;
			case 16: 
 				 a.emplace_back(iter.content);
					 break;
			case 17: 
 				 a.emplace_back(iter.postresult);
					 break;
			case 18: 
 				 a.emplace_back(iter.postfiles);
					 break;
			case 19: 
 				 a.emplace_back(iter.jiluphoto);
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
 				 a<<std::to_string(iter.xmeetid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.xpjid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.xtaskid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.adminid);
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.zhuchi); 
				 }else{
				 a<<iter.zhuchi;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.jilu); 
				 }else{
				 a<<iter.jilu;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.meetnotice); 
				 }else{
				 a<<iter.meetnotice;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.meetfiles); 
				 }else{
				 a<<iter.meetfiles;
				 }
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.address); 
				 }else{
				 a<<iter.address;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.meettime); 
				 }else{
				 a<<iter.meettime;
				 }
				 break;
			case 12: 
 				 a<<std::to_string(iter.regdate);
				 break;
			case 13: 
 				 a<<std::to_string(iter.expecttime);
				 break;
			case 14: 
 				 a<<std::to_string(iter.endtime);
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.presents); 
				 }else{
				 a<<iter.presents;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.content); 
				 }else{
				 a<<iter.content;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.postresult); 
				 }else{
				 a<<iter.postresult;
				 }
				 break;
			case 18: 
 				 if(isyinhao){ a<<jsonaddslash(iter.postfiles); 
				 }else{
				 a<<iter.postfiles;
				 }
				 break;
			case 19: 
 				 if(isyinhao){ a<<jsonaddslash(iter.jiluphoto); 
				 }else{
				 a<<iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
				 } 
			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
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
 	 ktemp=iter.xmeetid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.xpjid;
	 break;
case 3: 
 	 ktemp=iter.xtaskid;
	 break;
case 4: 
 	 ktemp=iter.adminid;
	 break;
case 12: 
 	 ktemp=iter.regdate;
	 break;
case 13: 
 	 ktemp=iter.expecttime;
	 break;
case 14: 
 	 ktemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 				 a.emplace(iter.xmeetid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.xpjid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.xtaskid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.adminid,iter);
				 break;
			case 12: 
 				 a.emplace(iter.regdate,iter);
				 break;
			case 13: 
 				 a.emplace(iter.expecttime,iter);
				 break;
			case 14: 
 				 a.emplace(iter.endtime,iter);
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
 				 a.emplace(iter.title,iter);
			 break;
			case 6: 
 				 a.emplace(iter.zhuchi,iter);
			 break;
			case 7: 
 				 a.emplace(iter.jilu,iter);
			 break;
			case 8: 
 				 a.emplace(iter.meetnotice,iter);
			 break;
			case 9: 
 				 a.emplace(iter.meetfiles,iter);
			 break;
			case 10: 
 				 a.emplace(iter.address,iter);
			 break;
			case 11: 
 				 a.emplace(iter.meettime,iter);
			 break;
			case 15: 
 				 a.emplace(iter.presents,iter);
			 break;
			case 16: 
 				 a.emplace(iter.content,iter);
			 break;
			case 17: 
 				 a.emplace(iter.postresult,iter);
			 break;
			case 18: 
 				 a.emplace(iter.postfiles,iter);
			 break;
			case 19: 
 				 a.emplace(iter.jiluphoto,iter);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
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
 				 ktemp=iter.xmeetid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.xpjid;
			 break;
			case 3: 
 				 ktemp=iter.xtaskid;
			 break;
			case 4: 
 				 ktemp=iter.adminid;
			 break;
			case 12: 
 				 ktemp=iter.regdate;
			 break;
			case 13: 
 				 ktemp=iter.expecttime;
			 break;
			case 14: 
 				 ktemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 	 ktemp=iter.title;
	 break;
case 6: 
 	 ktemp=iter.zhuchi;
	 break;
case 7: 
 	 ktemp=iter.jilu;
	 break;
case 8: 
 	 ktemp=iter.meetnotice;
	 break;
case 9: 
 	 ktemp=iter.meetfiles;
	 break;
case 10: 
 	 ktemp=iter.address;
	 break;
case 11: 
 	 ktemp=iter.meettime;
	 break;
case 15: 
 	 ktemp=iter.presents;
	 break;
case 16: 
 	 ktemp=iter.content;
	 break;
case 17: 
 	 ktemp=iter.postresult;
	 break;
case 18: 
 	 ktemp=iter.postfiles;
	 break;
case 19: 
 	 ktemp=iter.jiluphoto;
	 break;
	  }
 switch(vpos){
case 5: 
 	 vtemp=iter.title;
	 break;
case 6: 
 	 vtemp=iter.zhuchi;
	 break;
case 7: 
 	 vtemp=iter.jilu;
	 break;
case 8: 
 	 vtemp=iter.meetnotice;
	 break;
case 9: 
 	 vtemp=iter.meetfiles;
	 break;
case 10: 
 	 vtemp=iter.address;
	 break;
case 11: 
 	 vtemp=iter.meettime;
	 break;
case 15: 
 	 vtemp=iter.presents;
	 break;
case 16: 
 	 vtemp=iter.content;
	 break;
case 17: 
 	 vtemp=iter.postresult;
	 break;
case 18: 
 	 vtemp=iter.postfiles;
	 break;
case 19: 
 	 vtemp=iter.jiluphoto;
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
 	 a.emplace_back(iter.xmeetid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.xpjid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.xtaskid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.adminid,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.regdate,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.expecttime,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.endtime,iter);
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
 	 a.emplace_back(iter.title,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.zhuchi,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.jilu,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.meetnotice,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.meetfiles,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.address,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.meettime,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.presents,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.content,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.postresult,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.postfiles,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.jiluphoto,iter);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.endtime);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
				 break;
			 }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.endtime);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.endtime);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.xmeetid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.xpjid;
				 break;
			case 3: 
 				 vtemp=iter.xtaskid;
				 break;
			case 4: 
 				 vtemp=iter.adminid;
				 break;
			case 12: 
 				 vtemp=iter.regdate;
				 break;
			case 13: 
 				 vtemp=iter.expecttime;
				 break;
			case 14: 
 				 vtemp=iter.endtime;
				 break;
			 }

			switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.endtime);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.title;
				 break;
			case 6: 
 				 vtemp=iter.zhuchi;
				 break;
			case 7: 
 				 vtemp=iter.jilu;
				 break;
			case 8: 
 				 vtemp=iter.meetnotice;
				 break;
			case 9: 
 				 vtemp=iter.meetfiles;
				 break;
			case 10: 
 				 vtemp=iter.address;
				 break;
			case 11: 
 				 vtemp=iter.meettime;
				 break;
			case 15: 
 				 vtemp=iter.presents;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.postresult;
				 break;
			case 18: 
 				 vtemp=iter.postfiles;
				 break;
			case 19: 
 				 vtemp=iter.jiluphoto;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.endtime);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.zhuchi);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.jilu);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.meetnotice);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.meetfiles);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.address);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.meettime);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.presents);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.postresult);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.postfiles);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.jiluphoto);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.xmeetid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.xpjid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.xtaskid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.regdate);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.expecttime);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.endtime);
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
 				 a[iter.xmeetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.xtaskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.adminid].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.regdate].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.expecttime].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.endtime].emplace_back(iter);
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
 				 a[iter.title].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.zhuchi].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.jilu].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.meetnotice].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.meetfiles].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.meettime].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.presents].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.postresult].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.postfiles].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.jiluphoto].emplace_back(iter);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.zhuchi].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.jilu].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.meetnotice].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.meetfiles].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.meettime].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.presents].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.postresult].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.postfiles].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.jiluphoto].emplace_back(iter);
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
 				 ktemp=iter.title;
				 break;
			case 6: 
 				 ktemp=iter.zhuchi;
				 break;
			case 7: 
 				 ktemp=iter.jilu;
				 break;
			case 8: 
 				 ktemp=iter.meetnotice;
				 break;
			case 9: 
 				 ktemp=iter.meetfiles;
				 break;
			case 10: 
 				 ktemp=iter.address;
				 break;
			case 11: 
 				 ktemp=iter.meettime;
				 break;
			case 15: 
 				 ktemp=iter.presents;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.postresult;
				 break;
			case 18: 
 				 ktemp=iter.postfiles;
				 break;
			case 19: 
 				 ktemp=iter.jiluphoto;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xmeetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.xtaskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.expecttime].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.endtime].emplace_back(iter);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.xmeetid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.xpjid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.xtaskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.regdate].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.expecttime].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.endtime].emplace_back(iter);
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
 				 ktemp=iter.xmeetid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.xpjid;
				 break;
			case 3: 
 				 ktemp=iter.xtaskid;
				 break;
			case 4: 
 				 ktemp=iter.adminid;
				 break;
			case 12: 
 				 ktemp=iter.regdate;
				 break;
			case 13: 
 				 ktemp=iter.expecttime;
				 break;
			case 14: 
 				 ktemp=iter.endtime;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.zhuchi].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.jilu].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.meetnotice].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.meetfiles].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.address].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.meettime].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.presents].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.postresult].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.postfiles].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.jiluphoto].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   