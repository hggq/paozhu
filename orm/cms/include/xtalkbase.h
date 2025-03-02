#ifndef ORM_CMS_XTALKBASEMATA_H
#define ORM_CMS_XTALKBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Sun, 02 Mar 2025 10:54:33 GMT
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

struct xtalkbase
{
    struct meta{
     unsigned  int  talkid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  adminid = 0; ///**/
 unsigned  int  taskid = 0; ///**/
 unsigned  int  projectid = 0; ///**/
 std::string  content = ""; ///**/
 unsigned  char  isdelete = 0; ///**/
 unsigned  int  addtime = 0; ///**/
 unsigned  int  update_at = 0; ///**/
 unsigned  char  islock = 0; ///**/
 unsigned  int  replyid = 0; ///**/
 } data;
 std::vector<xtalkbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<xtalkbase::meta>::iterator begin(){     return record.begin(); }
std::vector<xtalkbase::meta>::iterator end(){     return record.end(); }
std::vector<xtalkbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<xtalkbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,11> col_names={"talkid","userid","adminid","taskid","projectid","content","isdelete","addtime","update_at","islock","replyid"};
static constexpr std::array<unsigned char,11> col_types={3,3,3,3,3,252,1,3,3,1,3};
static constexpr std::array<unsigned char,11> col_length={0,0,0,0,0,0,0,0,0,0,0};
static constexpr std::array<unsigned char,11> col_decimals={0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="xtalk";
static constexpr std::string_view modelname="Xtalk";

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
 if(colpospppc=='e'){ return 7; }
   	 break;
 }
 break;
case 'c':
   	 return 5;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 9;
break;
case 8:
   	 return 6;
break;
 }
 break;
case 'p':
   	 return 4;
break;
case 'r':
   	 return 10;
break;
case 't':
 switch(coln.size()){  
case 6:
 if(coln.size()>2&&(coln[2]=='l'||coln[2]=='L')){ return 0; }
 if(coln.size()>2&&(coln[2]=='s'||coln[2]=='S')){ return 3; }
   	 break;
 }
 break;
case 'u':
 switch(coln.size()){  
case 6:
   	 return 1;
break;
case 9:
   	 return 8;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "talkid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     xtalkbase::meta metatemp;    
            data = metatemp; 
      }
      
      std::string soft_remove_sql([[maybe_unused]] const std::string &fieldsql){
          std::string temp;
     
         if(fieldsql.size()<2)
         {
            temp="UPDATE `";
            temp.append(tablename);
            temp.push_back('`');
            temp.append(" SET ");
         }
         else
         {
            temp=_makeupdatesql(fieldsql);
            if(temp.size()>2)
            {
                if(temp.back()==0x20&&temp[temp.size()-2]=='T')
                {

                }
                else
                {
                    temp.push_back(',');
                }
            }
            
         }   
         
      	temp.push_back('`');
		temp.append("isdelete");
		temp.push_back('`');
		temp.append("=1 ");
		 if(fieldsql.size()>0){ data.isdelete=1; }
	
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

        if(data.talkid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.projectid);
}
tempsql<<",'"<<stringaddslash(data.content)<<"'";
if(data.isdelete==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.replyid);
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

        if(insert_data.talkid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.talkid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.adminid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.adminid);
}
if(insert_data.taskid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.taskid);
}
if(insert_data.projectid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.projectid);
}
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
if(insert_data.isdelete==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isdelete);
}
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.update_at==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.update_at);
}
if(insert_data.islock==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.islock);
}
if(insert_data.replyid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.replyid);
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


            	if(insert_data[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].talkid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].adminid);
	}
	if(insert_data[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].taskid);
	}
	if(insert_data[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].projectid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
	if(insert_data[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isdelete);
	}
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].update_at);
	}
	if(insert_data[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].islock);
	}
	if(insert_data[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].replyid);
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

        if(data.talkid==0){
	tempsql<<"`talkid`=0";
 }else{ 
	tempsql<<"`talkid`="<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",`adminid`=0";
 }else{ 
	tempsql<<",`adminid`="<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",`taskid`=0";
 }else{ 
	tempsql<<",`taskid`="<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",`projectid`=0";
 }else{ 
	tempsql<<",`projectid`="<<std::to_string(data.projectid);
}
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
if(data.isdelete==0){
	tempsql<<",`isdelete`=0";
 }else{ 
	tempsql<<",`isdelete`="<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",`addtime`=0";
 }else{ 
	tempsql<<",`addtime`="<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",`update_at`=0";
 }else{ 
	tempsql<<",`update_at`="<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",`islock`=0";
 }else{ 
	tempsql<<",`islock`="<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",`replyid`=0";
 }else{ 
	tempsql<<",`replyid`="<<std::to_string(data.replyid);
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
if(data.talkid==0){
	tempsql<<"`talkid`=0";
 }else{ 
	tempsql<<"`talkid`="<<std::to_string(data.talkid);
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
if(data.adminid==0){
	tempsql<<"`adminid`=0";
 }else{ 
	tempsql<<"`adminid`="<<std::to_string(data.adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.taskid==0){
	tempsql<<"`taskid`=0";
 }else{ 
	tempsql<<"`taskid`="<<std::to_string(data.taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.projectid==0){
	tempsql<<"`projectid`=0";
 }else{ 
	tempsql<<"`projectid`="<<std::to_string(data.projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isdelete==0){
	tempsql<<"`isdelete`=0";
 }else{ 
	tempsql<<"`isdelete`="<<std::to_string(data.isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.update_at==0){
	tempsql<<"`update_at`=0";
 }else{ 
	tempsql<<"`update_at`="<<std::to_string(data.update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.islock==0){
	tempsql<<"`islock`=0";
 }else{ 
	tempsql<<"`islock`="<<std::to_string(data.islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.replyid==0){
	tempsql<<"`replyid`=0";
 }else{ 
	tempsql<<"`replyid`="<<std::to_string(data.replyid);
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
            	if(record[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].talkid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].taskid);
	}
	if(record[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].projectid);
	}
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	if(record[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isdelete);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].update_at);
	}
	if(record[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].islock);
	}
	if(record[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].replyid);
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
            	if(record[i].talkid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].talkid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].adminid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].adminid);
	}
	if(record[i].taskid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].taskid);
	}
	if(record[i].projectid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].projectid);
	}
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	if(record[i].isdelete==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isdelete);
	}
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].update_at==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].update_at);
	}
	if(record[i].islock==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].islock);
	}
	if(record[i].replyid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].replyid);
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
if(data.talkid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.talkid));
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
if(data.adminid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.adminid));
}
 break;
 case 3:
if(data.taskid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.taskid));
}
 break;
 case 4:
if(data.projectid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.projectid));
}
 break;
 case 5:
	temparray.push_back(data.content);
 break;
 case 6:
if(data.isdelete==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isdelete));
}
 break;
 case 7:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 8:
if(data.update_at==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.update_at));
}
 break;
 case 9:
if(data.islock==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.islock));
}
 break;
 case 10:
if(data.replyid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.replyid));
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
if(data.talkid==0){
	tempsql.insert({"talkid","0"});
 }else{ 
	tempsql.insert({"talkid",std::to_string(data.talkid)});
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
if(data.adminid==0){
	tempsql.insert({"adminid","0"});
 }else{ 
	tempsql.insert({"adminid",std::to_string(data.adminid)});
}
 break;
 case 3:
if(data.taskid==0){
	tempsql.insert({"taskid","0"});
 }else{ 
	tempsql.insert({"taskid",std::to_string(data.taskid)});
}
 break;
 case 4:
if(data.projectid==0){
	tempsql.insert({"projectid","0"});
 }else{ 
	tempsql.insert({"projectid",std::to_string(data.projectid)});
}
 break;
 case 5:
	tempsql.insert({"content",data.content});
 break;
 case 6:
if(data.isdelete==0){
	tempsql.insert({"isdelete","0"});
 }else{ 
	tempsql.insert({"isdelete",std::to_string(data.isdelete)});
}
 break;
 case 7:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 8:
if(data.update_at==0){
	tempsql.insert({"update_at","0"});
 }else{ 
	tempsql.insert({"update_at",std::to_string(data.update_at)});
}
 break;
 case 9:
if(data.islock==0){
	tempsql.insert({"islock","0"});
 }else{ 
	tempsql.insert({"islock",std::to_string(data.islock)});
}
 break;
 case 10:
if(data.replyid==0){
	tempsql.insert({"replyid","0"});
 }else{ 
	tempsql.insert({"replyid",std::to_string(data.replyid)});
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
if(data.talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(data.talkid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.adminid==0){
	tempsql<<",\"adminid\":0";
 }else{ 
	tempsql<<",\"adminid\":"<<std::to_string(data.adminid);
}
if(data.taskid==0){
	tempsql<<",\"taskid\":0";
 }else{ 
	tempsql<<",\"taskid\":"<<std::to_string(data.taskid);
}
if(data.projectid==0){
	tempsql<<",\"projectid\":0";
 }else{ 
	tempsql<<",\"projectid\":"<<std::to_string(data.projectid);
}
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
if(data.isdelete==0){
	tempsql<<",\"isdelete\":0";
 }else{ 
	tempsql<<",\"isdelete\":"<<std::to_string(data.isdelete);
}
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.update_at==0){
	tempsql<<",\"update_at\":0";
 }else{ 
	tempsql<<",\"update_at\":"<<std::to_string(data.update_at);
}
if(data.islock==0){
	tempsql<<",\"islock\":0";
 }else{ 
	tempsql<<",\"islock\":"<<std::to_string(data.islock);
}
if(data.replyid==0){
	tempsql<<",\"replyid\":0";
 }else{ 
	tempsql<<",\"replyid\":"<<std::to_string(data.replyid);
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
if(data.talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(data.talkid);
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
if(data.adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(data.adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(data.taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(data.projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(data.isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(data.update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(data.islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(data.replyid);
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
        xtalkbase::meta metatemp; 
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
			data.talkid=std::stoul(set_value_name);
		}catch (...) { 
			data.talkid=0;
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
			data.adminid=std::stoul(set_value_name);
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=std::stoul(set_value_name);
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=std::stoul(set_value_name);
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content.append(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=std::stoi(set_value_name);
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=std::stoul(set_value_name);
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=std::stoul(set_value_name);
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=std::stoi(set_value_name);
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=std::stoul(set_value_name);
		}catch (...) { 
			data.replyid=0;
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
			data.talkid=set_value_name;
		}catch (...) { 
			data.talkid=0;
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
			data.adminid=set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=set_value_name;
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=set_value_name;
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=set_value_name;
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=set_value_name;
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=set_value_name;
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=set_value_name;
		}catch (...) { 
			data.replyid=0;
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
			data.talkid=(unsigned int)set_value_name;
		}catch (...) { 
			data.talkid=0;
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
			data.adminid=(unsigned int)set_value_name;
		}catch (...) { 
			data.adminid=0;
			 }
			break;
		case 3:
		 try{
			data.taskid=(unsigned int)set_value_name;
		}catch (...) { 
			data.taskid=0;
			 }
			break;
		case 4:
		 try{
			data.projectid=(unsigned int)set_value_name;
		}catch (...) { 
			data.projectid=0;
			 }
			break;
		case 5:
		 try{
			data.content=std::to_string(set_value_name);
		}catch (...) { 
			data.content.clear();
			 }
			break;
		case 6:
		 try{
			data.isdelete=(int)set_value_name;
		}catch (...) { 
			data.isdelete=0;
			 }
			break;
		case 7:
		 try{
			data.addtime=(unsigned int)set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 8:
		 try{
			data.update_at=(unsigned int)set_value_name;
		}catch (...) { 
			data.update_at=0;
			 }
			break;
		case 9:
		 try{
			data.islock=(int)set_value_name;
		}catch (...) { 
			data.islock=0;
			 }
			break;
		case 10:
		 try{
			data.replyid=(unsigned int)set_value_name;
		}catch (...) { 
			data.replyid=0;
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
if(record[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(record[n].talkid);
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(record[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(record[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(record[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(record[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(record[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(record[n].replyid);
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
if(record[n].talkid==0){
	tempsql<<"\"talkid\":0";
 }else{ 
	tempsql<<"\"talkid\":"<<std::to_string(record[n].talkid);
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
if(record[n].adminid==0){
	tempsql<<"\"adminid\":0";
 }else{ 
	tempsql<<"\"adminid\":"<<std::to_string(record[n].adminid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].taskid==0){
	tempsql<<"\"taskid\":0";
 }else{ 
	tempsql<<"\"taskid\":"<<std::to_string(record[n].taskid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].projectid==0){
	tempsql<<"\"projectid\":0";
 }else{ 
	tempsql<<"\"projectid\":"<<std::to_string(record[n].projectid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isdelete==0){
	tempsql<<"\"isdelete\":0";
 }else{ 
	tempsql<<"\"isdelete\":"<<std::to_string(record[n].isdelete);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].update_at==0){
	tempsql<<"\"update_at\":0";
 }else{ 
	tempsql<<"\"update_at\":"<<std::to_string(record[n].update_at);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].islock==0){
	tempsql<<"\"islock\":0";
 }else{ 
	tempsql<<"\"islock\":"<<std::to_string(record[n].islock);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].replyid==0){
	tempsql<<"\"replyid\":0";
 }else{ 
	tempsql<<"\"replyid\":"<<std::to_string(record[n].replyid);
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
   long long getPK(){  return data.talkid; } 
 void setPK(long long val){  data.talkid=val;} 
 unsigned  int  getTalkid(){  return data.talkid; } 
 void setTalkid( unsigned  int  val){  data.talkid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getAdminid(){  return data.adminid; } 
 void setAdminid( unsigned  int  val){  data.adminid=val;} 

 unsigned  int  getTaskid(){  return data.taskid; } 
 void setTaskid( unsigned  int  val){  data.taskid=val;} 

 unsigned  int  getProjectid(){  return data.projectid; } 
 void setProjectid( unsigned  int  val){  data.projectid=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 unsigned  char  getIsdelete(){  return data.isdelete; } 
 void setIsdelete( unsigned  char  val){  data.isdelete=val;} 

 unsigned  int  getAddtime(){  return data.addtime; } 
 void setAddtime( unsigned  int  val){  data.addtime=val;} 

 unsigned  int  getUpdateAt(){  return data.update_at; } 
 void setUpdateAt( unsigned  int  val){  data.update_at=val;} 

 unsigned  char  getIslock(){  return data.islock; } 
 void setIslock( unsigned  char  val){  data.islock=val;} 

 unsigned  int  getReplyid(){  return data.replyid; } 
 void setReplyid( unsigned  int  val){  data.replyid=val;} 

xtalkbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
xtalkbase::meta getData(){
 	 return data; 
} 
std::vector<xtalkbase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="content")
		{
			return data.content;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="talkid")
		{
			return data.talkid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="adminid")
		{
			return data.adminid;
		}
		 if(key_name=="taskid")
		{
			return data.taskid;
		}
		 if(key_name=="projectid")
		{
			return data.projectid;
		}
		 if(key_name=="isdelete")
		{
			return data.isdelete;
		}
		 if(key_name=="addtime")
		{
			return data.addtime;
		}
		 if(key_name=="update_at")
		{
			return data.update_at;
		}
		 if(key_name=="islock")
		{
			return data.islock;
		}
		 if(key_name=="replyid")
		{
			return data.replyid;
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
 				 a.emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a.emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a.emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a.emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a.emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a.emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a.emplace_back(iter.islock);
				 break;
			case 10: 
 				 a.emplace_back(iter.replyid);
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
 				 return data.talkid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.adminid;
				 break;
			case 3: 
 				 return data.taskid;
				 break;
			case 4: 
 				 return data.projectid;
				 break;
			case 6: 
 				 return data.isdelete;
				 break;
			case 7: 
 				 return data.addtime;
				 break;
			case 8: 
 				 return data.update_at;
				 break;
			case 9: 
 				 return data.islock;
				 break;
			case 10: 
 				 return data.replyid;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] xtalkbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.talkid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.adminid;
				 break;
			case 3: 
 				 return iter.taskid;
				 break;
			case 4: 
 				 return iter.projectid;
				 break;
			case 6: 
 				 return iter.isdelete;
				 break;
			case 7: 
 				 return iter.addtime;
				 break;
			case 8: 
 				 return iter.update_at;
				 break;
			case 9: 
 				 return iter.islock;
				 break;
			case 10: 
 				 return iter.replyid;
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
            T getVal([[maybe_unused]] xtalkbase::meta & iter,std::string keyname)
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
 				 return data.content;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] xtalkbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 5: 
 				 return iter.content;
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
 				 a.emplace_back(iter.content);
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
 				 a<<std::to_string(iter.talkid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.adminid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.taskid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.projectid);
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.content); 
				 }else{
				 a<<iter.content;
				 }
				 break;
			case 6: 
 				 a<<std::to_string(iter.isdelete);
				 break;
			case 7: 
 				 a<<std::to_string(iter.addtime);
				 break;
			case 8: 
 				 a<<std::to_string(iter.update_at);
				 break;
			case 9: 
 				 a<<std::to_string(iter.islock);
				 break;
			case 10: 
 				 a<<std::to_string(iter.replyid);
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
 				 ktemp=iter.content;
				 break;
				 } 
			switch(vpos){
			case 5: 
 				 vtemp=iter.content;
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
 				 ktemp=iter.content;
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
 	 ktemp=iter.talkid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.adminid;
	 break;
case 3: 
 	 ktemp=iter.taskid;
	 break;
case 4: 
 	 ktemp=iter.projectid;
	 break;
case 6: 
 	 ktemp=iter.isdelete;
	 break;
case 7: 
 	 ktemp=iter.addtime;
	 break;
case 8: 
 	 ktemp=iter.update_at;
	 break;
case 9: 
 	 ktemp=iter.islock;
	 break;
case 10: 
 	 ktemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.content;
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
 				 ktemp=iter.content;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 				 a.emplace(iter.talkid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.adminid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.taskid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.projectid,iter);
				 break;
			case 6: 
 				 a.emplace(iter.isdelete,iter);
				 break;
			case 7: 
 				 a.emplace(iter.addtime,iter);
				 break;
			case 8: 
 				 a.emplace(iter.update_at,iter);
				 break;
			case 9: 
 				 a.emplace(iter.islock,iter);
				 break;
			case 10: 
 				 a.emplace(iter.replyid,iter);
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
 				 a.emplace(iter.content,iter);
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
 				 ktemp=iter.content;
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
 				 ktemp=iter.talkid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.adminid;
			 break;
			case 3: 
 				 ktemp=iter.taskid;
			 break;
			case 4: 
 				 ktemp=iter.projectid;
			 break;
			case 6: 
 				 ktemp=iter.isdelete;
			 break;
			case 7: 
 				 ktemp=iter.addtime;
			 break;
			case 8: 
 				 ktemp=iter.update_at;
			 break;
			case 9: 
 				 ktemp=iter.islock;
			 break;
			case 10: 
 				 ktemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.content;
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
 				 ktemp=iter.content;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 	 ktemp=iter.content;
	 break;
	  }
 switch(vpos){
case 5: 
 	 vtemp=iter.content;
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
 	 a.emplace_back(iter.talkid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.adminid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.taskid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.projectid,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.isdelete,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.addtime,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.update_at,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.islock,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.replyid,iter);
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
 	 a.emplace_back(iter.content,iter);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.replyid);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
				 break;
			 }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.replyid);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
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
 				 ktemp=iter.content;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
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
 				 ktemp=iter.content;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.replyid);
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
 				 ktemp=iter.content;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.talkid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.adminid;
				 break;
			case 3: 
 				 vtemp=iter.taskid;
				 break;
			case 4: 
 				 vtemp=iter.projectid;
				 break;
			case 6: 
 				 vtemp=iter.isdelete;
				 break;
			case 7: 
 				 vtemp=iter.addtime;
				 break;
			case 8: 
 				 vtemp=iter.update_at;
				 break;
			case 9: 
 				 vtemp=iter.islock;
				 break;
			case 10: 
 				 vtemp=iter.replyid;
				 break;
			 }

			switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
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
 				 ktemp=iter.content;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
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
 				 ktemp=iter.content;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.replyid);
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
 				 ktemp=iter.content;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.content;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
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
 				 ktemp=iter.content;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.content);
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
 				 ktemp=iter.content;
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
 				 ktemp=iter.content;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.replyid);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.content);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.talkid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.adminid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.taskid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.projectid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isdelete);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.update_at);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.islock);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.replyid);
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
 				 a[iter.talkid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.adminid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.taskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.projectid].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.isdelete].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.addtime].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.update_at].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.islock].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.replyid].emplace_back(iter);
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
 				 a[iter.content].emplace_back(iter);
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
 				 ktemp=iter.content;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.content].emplace_back(iter);
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
 				 ktemp=iter.content;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.talkid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.taskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.projectid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isdelete].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.update_at].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.islock].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.replyid].emplace_back(iter);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.talkid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.adminid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.taskid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.projectid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isdelete].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.update_at].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.islock].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.replyid].emplace_back(iter);
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
 				 ktemp=iter.talkid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.adminid;
				 break;
			case 3: 
 				 ktemp=iter.taskid;
				 break;
			case 4: 
 				 ktemp=iter.projectid;
				 break;
			case 6: 
 				 ktemp=iter.isdelete;
				 break;
			case 7: 
 				 ktemp=iter.addtime;
				 break;
			case 8: 
 				 ktemp=iter.update_at;
				 break;
			case 9: 
 				 ktemp=iter.islock;
				 break;
			case 10: 
 				 ktemp=iter.replyid;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   