#ifndef ORM_CMS_TOPICBASEMATA_H
#define ORM_CMS_TOPICBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Tue, 10 Dec 2024 07:42:16 GMT
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

struct topicbase
{
    struct meta{
    unsigned  int topicid= 0; //
unsigned  int userid= 0; //
unsigned  int parentid= 0; //
unsigned  int cateid= 0; //类型
unsigned  char sorttype=0; //内容排序
unsigned  int languagetype= 0; //语言类型
unsigned  char isview=0; //是否显示
unsigned  char isside=1; //是否边栏
unsigned  int sortid= 0; //排序id
 std::string title=""; //标题
 std::string twotitle=""; //第二标题
 std::string memo=""; //备注
 std::string templatename=""; //模板名称
 std::string url=""; //网址
 std::string urlpath=""; //路径
 std::string imgurl=""; //图片地址
 std::string topimg=""; //头图
unsigned  int accesscode= 0; //权限代码
 } data;
  
        struct meta_tree{
        unsigned  int topicid= 0; //
unsigned  int userid= 0; //
unsigned  int parentid= 0; //
unsigned  int cateid= 0; //类型
unsigned  char sorttype=0; //内容排序
unsigned  int languagetype= 0; //语言类型
unsigned  char isview=0; //是否显示
unsigned  char isside=1; //是否边栏
unsigned  int sortid= 0; //排序id
 std::string title=""; //标题
 std::string twotitle=""; //第二标题
 std::string memo=""; //备注
 std::string templatename=""; //模板名称
 std::string url=""; //网址
 std::string urlpath=""; //路径
 std::string imgurl=""; //图片地址
 std::string topimg=""; //头图
unsigned  int accesscode= 0; //权限代码

	std::vector<meta_tree> children;
 };
 std::vector<topicbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<unsigned char> _keypos{0x00};
MYSQL_ROW _row;
std::vector<topicbase::meta>::iterator begin(){     return record.begin(); }
std::vector<topicbase::meta>::iterator end(){     return record.end(); }
std::vector<topicbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<topicbase::meta>::const_iterator end() const{     return record.end(); }
const std::array<std::string,18> colnames={"topicid","userid","parentid","cateid","sorttype","languagetype","isview","isside","sortid","title","twotitle","memo","templatename","url","urlpath","imgurl","topimg","accesscode"};
const std::array<unsigned char,18> colnamestype= {3,3,3,3,1,3,1,1,3,253,253,253,253,253,253,253,252,3};
std::string tablename="topic";
std::string modelname="Topic";

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
   	 return 17;
break;
case 'c':
   	 return 3;
break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 7; }
 if(colpospppc=='l'){ return 15; }
 if(colpospppc=='w'){ return 6; }
   	 break;
 }
 break;
case 'l':
   	 return 5;
break;
case 'm':
   	 return 11;
break;
case 'p':
   	 return 2;
break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 8;
break;
case 8:
   	 return 4;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 9;
break;
case 6:
   	 return 16;
break;
case 7:
   	 return 0;
break;
case 8:
   	 return 10;
break;
case 12:
   	 return 12;
break;
 }
 break;
case 'u':
 switch(coln.size()){  
case 3:
   	 return 13;
break;
case 6:
   	 return 1;
break;
case 7:
   	 return 14;
break;
 }
 break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "topicid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     topicbase::meta metatemp;    
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
			data.topicid=std::stoul(_row[i]);
		}catch (...) { 
			data.topicid=0;
			 }
			break;
	case 1:
		 try{
			data.userid=std::stoul(_row[i]);
		}catch (...) { 
			data.userid=0;
			 }
			break;
	case 2:
		 try{
			data.parentid=std::stoul(_row[i]);
		}catch (...) { 
			data.parentid=0;
			 }
			break;
	case 3:
		 try{
			data.cateid=std::stoul(_row[i]);
		}catch (...) { 
			data.cateid=0;
			 }
			break;
	case 4:
		 try{
			data.sorttype=std::stoi(_row[i]);
		}catch (...) { 
			data.sorttype=0;
			 }
			break;
	case 5:
		 try{
			data.languagetype=std::stoul(_row[i]);
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
	case 6:
		 try{
			data.isview=std::stoi(_row[i]);
		}catch (...) { 
			data.isview=0;
			 }
			break;
	case 7:
		 try{
			data.isside=std::stoi(_row[i]);
		}catch (...) { 
			data.isside=0;
			 }
			break;
	case 8:
		 try{
			data.sortid=std::stoul(_row[i]);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
	case 9:
		 try{
			data.title.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.title.clear();
			 }
			break;
	case 10:
		 try{
			data.twotitle.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.twotitle.clear();
			 }
			break;
	case 11:
		 try{
			data.memo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.memo.clear();
			 }
			break;
	case 12:
		 try{
			data.templatename.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.templatename.clear();
			 }
			break;
	case 13:
		 try{
			data.url.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.url.clear();
			 }
			break;
	case 14:
		 try{
			data.urlpath.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
	case 15:
		 try{
			data.imgurl.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.imgurl.clear();
			 }
			break;
	case 16:
		 try{
			data.topimg.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.topimg.clear();
			 }
			break;
	case 17:
		 try{
			data.accesscode=std::stoul(_row[i]);
		}catch (...) { 
			data.accesscode=0;
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
           topicbase::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        	case 0:
		 try{
			metatemp.topicid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.topicid=0;
			 }
			break;
	case 1:
		 try{
			metatemp.userid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.userid=0;
			 }
			break;
	case 2:
		 try{
			metatemp.parentid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.parentid=0;
			 }
			break;
	case 3:
		 try{
			metatemp.cateid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.cateid=0;
			 }
			break;
	case 4:
		 try{
			metatemp.sorttype=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.sorttype=0;
			 }
			break;
	case 5:
		 try{
			metatemp.languagetype=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.languagetype=0;
			 }
			break;
	case 6:
		 try{
			metatemp.isview=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.isview=0;
			 }
			break;
	case 7:
		 try{
			metatemp.isside=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.isside=0;
			 }
			break;
	case 8:
		 try{
			metatemp.sortid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.sortid=0;
			 }
			break;
	case 9:
		 try{
			metatemp.title.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.title.clear();
			 }
			break;
	case 10:
		 try{
			metatemp.twotitle.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.twotitle.clear();
			 }
			break;
	case 11:
		 try{
			metatemp.memo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.memo.clear();
			 }
			break;
	case 12:
		 try{
			metatemp.templatename.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.templatename.clear();
			 }
			break;
	case 13:
		 try{
			metatemp.url.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.url.clear();
			 }
			break;
	case 14:
		 try{
			metatemp.urlpath.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.urlpath.clear();
			 }
			break;
	case 15:
		 try{
			metatemp.imgurl.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.imgurl.clear();
			 }
			break;
	case 16:
		 try{
			metatemp.topimg.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.topimg.clear();
			 }
			break;
	case 17:
		 try{
			metatemp.accesscode=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.accesscode=0;
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

        if(data.topicid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.twotitle)<<"'";
tempsql<<",'"<<stringaddslash(data.memo)<<"'";
tempsql<<",'"<<stringaddslash(data.templatename)<<"'";
tempsql<<",'"<<stringaddslash(data.url)<<"'";
tempsql<<",'"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(data.imgurl)<<"'";
tempsql<<",'"<<stringaddslash(data.topimg)<<"'";
if(data.accesscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.accesscode);
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

        if(insert_data.topicid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.topicid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.parentid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.parentid);
}
if(insert_data.cateid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.cateid);
}
if(insert_data.sorttype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sorttype);
}
if(insert_data.languagetype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.languagetype);
}
if(insert_data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isview);
}
if(insert_data.isside==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isside);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.twotitle)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.memo)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.templatename)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.url)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.urlpath)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.imgurl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.topimg)<<"'";
if(insert_data.accesscode==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.accesscode);
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


        	if(insert_data[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].parentid);
	}
	if(insert_data[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].cateid);
	}
	if(insert_data[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sorttype);
	}
	if(insert_data[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].languagetype);
	}
	if(insert_data[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isview);
	}
	if(insert_data[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isside);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].twotitle)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].memo)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].templatename)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].url)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].urlpath)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].imgurl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].topimg)<<"'";
	if(insert_data[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].accesscode);
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

        if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",`parentid`=0";
 }else{ 
	tempsql<<",`parentid`="<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",`cateid`=0";
 }else{ 
	tempsql<<",`cateid`="<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",`sorttype`=0";
 }else{ 
	tempsql<<",`sorttype`="<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",`languagetype`=0";
 }else{ 
	tempsql<<",`languagetype`="<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",`isview`=0";
 }else{ 
	tempsql<<",`isview`="<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",`isside`=0";
 }else{ 
	tempsql<<",`isside`="<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`twotitle`='"<<stringaddslash(data.twotitle)<<"'";
tempsql<<",`memo`='"<<stringaddslash(data.memo)<<"'";
tempsql<<",`templatename`='"<<stringaddslash(data.templatename)<<"'";
tempsql<<",`url`='"<<stringaddslash(data.url)<<"'";
tempsql<<",`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
tempsql<<",`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
tempsql<<",`topimg`='"<<stringaddslash(data.topimg)<<"'";
if(data.accesscode==0){
	tempsql<<",`accesscode`=0";
 }else{ 
	tempsql<<",`accesscode`="<<std::to_string(data.accesscode);
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
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
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
if(data.parentid==0){
	tempsql<<"`parentid`=0";
 }else{ 
	tempsql<<"`parentid`="<<std::to_string(data.parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.cateid==0){
	tempsql<<"`cateid`=0";
 }else{ 
	tempsql<<"`cateid`="<<std::to_string(data.cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sorttype==0){
	tempsql<<"`sorttype`=0";
 }else{ 
	tempsql<<"`sorttype`="<<std::to_string(data.sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"`languagetype`=0";
 }else{ 
	tempsql<<"`languagetype`="<<std::to_string(data.languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"`isview`=0";
 }else{ 
	tempsql<<"`isview`="<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isside==0){
	tempsql<<"`isside`=0";
 }else{ 
	tempsql<<"`isside`="<<std::to_string(data.isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`twotitle`='"<<stringaddslash(data.twotitle)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`memo`='"<<stringaddslash(data.memo)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`templatename`='"<<stringaddslash(data.templatename)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`url`='"<<stringaddslash(data.url)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`urlpath`='"<<stringaddslash(data.urlpath)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`imgurl`='"<<stringaddslash(data.imgurl)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`topimg`='"<<stringaddslash(data.topimg)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.accesscode==0){
	tempsql<<"`accesscode`=0";
 }else{ 
	tempsql<<"`accesscode`="<<std::to_string(data.accesscode);
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
            	if(record[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].cateid);
	}
	if(record[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sorttype);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isside);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].twotitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].templatename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].url)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].topimg)<<"'";
	if(record[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].accesscode);
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
            	if(record[i].topicid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].topicid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].parentid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].parentid);
	}
	if(record[i].cateid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].cateid);
	}
	if(record[i].sorttype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sorttype);
	}
	if(record[i].languagetype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].languagetype);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isside==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isside);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].twotitle)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].templatename)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].url)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].urlpath)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].imgurl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].topimg)<<"'";
	if(record[i].accesscode==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].accesscode);
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
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
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
if(data.parentid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.parentid));
}
 break;
 case 3:
if(data.cateid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.cateid));
}
 break;
 case 4:
if(data.sorttype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sorttype));
}
 break;
 case 5:
if(data.languagetype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.languagetype));
}
 break;
 case 6:
if(data.isview==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isview));
}
 break;
 case 7:
if(data.isside==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isside));
}
 break;
 case 8:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 9:
	temparray.push_back(data.title);
 break;
 case 10:
	temparray.push_back(data.twotitle);
 break;
 case 11:
	temparray.push_back(data.memo);
 break;
 case 12:
	temparray.push_back(data.templatename);
 break;
 case 13:
	temparray.push_back(data.url);
 break;
 case 14:
	temparray.push_back(data.urlpath);
 break;
 case 15:
	temparray.push_back(data.imgurl);
 break;
 case 16:
	temparray.push_back(data.topimg);
 break;
 case 17:
if(data.accesscode==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.accesscode));
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
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
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
if(data.parentid==0){
	tempsql.insert({"parentid","0"});
 }else{ 
	tempsql.insert({"parentid",std::to_string(data.parentid)});
}
 break;
 case 3:
if(data.cateid==0){
	tempsql.insert({"cateid","0"});
 }else{ 
	tempsql.insert({"cateid",std::to_string(data.cateid)});
}
 break;
 case 4:
if(data.sorttype==0){
	tempsql.insert({"sorttype","0"});
 }else{ 
	tempsql.insert({"sorttype",std::to_string(data.sorttype)});
}
 break;
 case 5:
if(data.languagetype==0){
	tempsql.insert({"languagetype","0"});
 }else{ 
	tempsql.insert({"languagetype",std::to_string(data.languagetype)});
}
 break;
 case 6:
if(data.isview==0){
	tempsql.insert({"isview","0"});
 }else{ 
	tempsql.insert({"isview",std::to_string(data.isview)});
}
 break;
 case 7:
if(data.isside==0){
	tempsql.insert({"isside","0"});
 }else{ 
	tempsql.insert({"isside",std::to_string(data.isside)});
}
 break;
 case 8:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 9:
	tempsql.insert({"title",data.title});
 break;
 case 10:
	tempsql.insert({"twotitle",data.twotitle});
 break;
 case 11:
	tempsql.insert({"memo",data.memo});
 break;
 case 12:
	tempsql.insert({"templatename",data.templatename});
 break;
 case 13:
	tempsql.insert({"url",data.url});
 break;
 case 14:
	tempsql.insert({"urlpath",data.urlpath});
 break;
 case 15:
	tempsql.insert({"imgurl",data.imgurl});
 break;
 case 16:
	tempsql.insert({"topimg",data.topimg});
 break;
 case 17:
if(data.accesscode==0){
	tempsql.insert({"accesscode","0"});
 }else{ 
	tempsql.insert({"accesscode",std::to_string(data.accesscode)});
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
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.parentid==0){
	tempsql<<",\"parentid\":0";
 }else{ 
	tempsql<<",\"parentid\":"<<std::to_string(data.parentid);
}
if(data.cateid==0){
	tempsql<<",\"cateid\":0";
 }else{ 
	tempsql<<",\"cateid\":"<<std::to_string(data.cateid);
}
if(data.sorttype==0){
	tempsql<<",\"sorttype\":0";
 }else{ 
	tempsql<<",\"sorttype\":"<<std::to_string(data.sorttype);
}
if(data.languagetype==0){
	tempsql<<",\"languagetype\":0";
 }else{ 
	tempsql<<",\"languagetype\":"<<std::to_string(data.languagetype);
}
if(data.isview==0){
	tempsql<<",\"isview\":0";
 }else{ 
	tempsql<<",\"isview\":"<<std::to_string(data.isview);
}
if(data.isside==0){
	tempsql<<",\"isside\":0";
 }else{ 
	tempsql<<",\"isside\":"<<std::to_string(data.isside);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"twotitle\":\""<<http::utf8_to_jsonstring(data.twotitle);
tempsql<<"\"";
tempsql<<",\"memo\":\""<<http::utf8_to_jsonstring(data.memo);
tempsql<<"\"";
tempsql<<",\"templatename\":\""<<http::utf8_to_jsonstring(data.templatename);
tempsql<<"\"";
tempsql<<",\"url\":\""<<http::utf8_to_jsonstring(data.url);
tempsql<<"\"";
tempsql<<",\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath);
tempsql<<"\"";
tempsql<<",\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl);
tempsql<<"\"";
tempsql<<",\"topimg\":\""<<http::utf8_to_jsonstring(data.topimg);
tempsql<<"\"";
if(data.accesscode==0){
	tempsql<<",\"accesscode\":0";
 }else{ 
	tempsql<<",\"accesscode\":"<<std::to_string(data.accesscode);
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
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
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
if(data.parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(data.parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(data.cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(data.sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(data.languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(data.isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(data.isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(data.twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(data.memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(data.templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(data.url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(data.urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(data.imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(data.topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(data.accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(data.accesscode);
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
        topicbase::meta metatemp; 
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
			data.topicid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicid=0;
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
			data.parentid=std::stoul(set_value_name);
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.cateid=std::stoul(set_value_name);
		}catch (...) { 
			data.cateid=0;
			 }
			break;
		case 4:
		 try{
			data.sorttype=std::stoi(set_value_name);
		}catch (...) { 
			data.sorttype=0;
			 }
			break;
		case 5:
		 try{
			data.languagetype=std::stoul(set_value_name);
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 6:
		 try{
			data.isview=std::stoi(set_value_name);
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isside=std::stoi(set_value_name);
		}catch (...) { 
			data.isside=0;
			 }
			break;
		case 8:
		 try{
			data.sortid=std::stoul(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 9:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 10:
		 try{
			data.twotitle.append(set_value_name);
		}catch (...) { 
			data.twotitle.clear();
			 }
			break;
		case 11:
		 try{
			data.memo.append(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 12:
		 try{
			data.templatename.append(set_value_name);
		}catch (...) { 
			data.templatename.clear();
			 }
			break;
		case 13:
		 try{
			data.url.append(set_value_name);
		}catch (...) { 
			data.url.clear();
			 }
			break;
		case 14:
		 try{
			data.urlpath.append(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 15:
		 try{
			data.imgurl.append(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
			 }
			break;
		case 16:
		 try{
			data.topimg.append(set_value_name);
		}catch (...) { 
			data.topimg.clear();
			 }
			break;
		case 17:
		 try{
			data.accesscode=std::stoul(set_value_name);
		}catch (...) { 
			data.accesscode=0;
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
			data.topicid=set_value_name;
		}catch (...) { 
			data.topicid=0;
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
			data.parentid=set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.cateid=set_value_name;
		}catch (...) { 
			data.cateid=0;
			 }
			break;
		case 4:
		 try{
			data.sorttype=set_value_name;
		}catch (...) { 
			data.sorttype=0;
			 }
			break;
		case 5:
		 try{
			data.languagetype=set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 6:
		 try{
			data.isview=set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isside=set_value_name;
		}catch (...) { 
			data.isside=0;
			 }
			break;
		case 8:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 9:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 10:
		 try{
			data.twotitle=std::to_string(set_value_name);
		}catch (...) { 
			data.twotitle.clear();
			 }
			break;
		case 11:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 12:
		 try{
			data.templatename=std::to_string(set_value_name);
		}catch (...) { 
			data.templatename.clear();
			 }
			break;
		case 13:
		 try{
			data.url=std::to_string(set_value_name);
		}catch (...) { 
			data.url.clear();
			 }
			break;
		case 14:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 15:
		 try{
			data.imgurl=std::to_string(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
			 }
			break;
		case 16:
		 try{
			data.topimg=std::to_string(set_value_name);
		}catch (...) { 
			data.topimg.clear();
			 }
			break;
		case 17:
		 try{
			data.accesscode=set_value_name;
		}catch (...) { 
			data.accesscode=0;
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
			data.topicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicid=0;
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
			data.parentid=(unsigned int)set_value_name;
		}catch (...) { 
			data.parentid=0;
			 }
			break;
		case 3:
		 try{
			data.cateid=(unsigned int)set_value_name;
		}catch (...) { 
			data.cateid=0;
			 }
			break;
		case 4:
		 try{
			data.sorttype=(int)set_value_name;
		}catch (...) { 
			data.sorttype=0;
			 }
			break;
		case 5:
		 try{
			data.languagetype=(unsigned int)set_value_name;
		}catch (...) { 
			data.languagetype=0;
			 }
			break;
		case 6:
		 try{
			data.isview=(int)set_value_name;
		}catch (...) { 
			data.isview=0;
			 }
			break;
		case 7:
		 try{
			data.isside=(int)set_value_name;
		}catch (...) { 
			data.isside=0;
			 }
			break;
		case 8:
		 try{
			data.sortid=(unsigned int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 9:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 10:
		 try{
			data.twotitle=std::to_string(set_value_name);
		}catch (...) { 
			data.twotitle.clear();
			 }
			break;
		case 11:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 12:
		 try{
			data.templatename=std::to_string(set_value_name);
		}catch (...) { 
			data.templatename.clear();
			 }
			break;
		case 13:
		 try{
			data.url=std::to_string(set_value_name);
		}catch (...) { 
			data.url.clear();
			 }
			break;
		case 14:
		 try{
			data.urlpath=std::to_string(set_value_name);
		}catch (...) { 
			data.urlpath.clear();
			 }
			break;
		case 15:
		 try{
			data.imgurl=std::to_string(set_value_name);
		}catch (...) { 
			data.imgurl.clear();
			 }
			break;
		case 16:
		 try{
			data.topimg=std::to_string(set_value_name);
		}catch (...) { 
			data.topimg.clear();
			 }
			break;
		case 17:
		 try{
			data.accesscode=(unsigned int)set_value_name;
		}catch (...) { 
			data.accesscode=0;
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
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
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(record[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(record[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(record[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(record[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(record[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(record[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(record[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(record[n].accesscode);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
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
if(record[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(record[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(record[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(record[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(record[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(record[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(record[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(record[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(record[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(record[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(record[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(record[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(record[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(record[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(record[n].accesscode);
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
   long long getPK(){  return data.topicid; } 
 void setPK(long long val){  data.topicid=val;} 
unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid(unsigned  int  val){  data.topicid=val;} 

unsigned  int  getUserid(){  return data.userid; } 
 void setUserid(unsigned  int  val){  data.userid=val;} 

unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid(unsigned  int  val){  data.parentid=val;} 

unsigned  int  getCateid(){  return data.cateid; } 
 void setCateid(unsigned  int  val){  data.cateid=val;} 

unsigned  int  getSorttype(){  return data.sorttype; } 
 void setSorttype(unsigned  int  val){  data.sorttype=val;} 

unsigned  int  getLanguagetype(){  return data.languagetype; } 
 void setLanguagetype(unsigned  int  val){  data.languagetype=val;} 

unsigned  int  getIsview(){  return data.isview; } 
 void setIsview(unsigned  int  val){  data.isview=val;} 

unsigned  int  getIsside(){  return data.isside; } 
 void setIsside(unsigned  int  val){  data.isside=val;} 

unsigned  int  getSortid(){  return data.sortid; } 
 void setSortid(unsigned  int  val){  data.sortid=val;} 

std::string getTitle(){  return data.title; } 
std::string& getRefTitle(){  return std::ref(data.title); } 
 void setTitle(std::string &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

std::string getTwotitle(){  return data.twotitle; } 
std::string& getRefTwotitle(){  return std::ref(data.twotitle); } 
 void setTwotitle(std::string &val){  data.twotitle=val;} 
 void setTwotitle(std::string_view val){  data.twotitle=val;} 

std::string getMemo(){  return data.memo; } 
std::string& getRefMemo(){  return std::ref(data.memo); } 
 void setMemo(std::string &val){  data.memo=val;} 
 void setMemo(std::string_view val){  data.memo=val;} 

std::string getTemplatename(){  return data.templatename; } 
std::string& getRefTemplatename(){  return std::ref(data.templatename); } 
 void setTemplatename(std::string &val){  data.templatename=val;} 
 void setTemplatename(std::string_view val){  data.templatename=val;} 

std::string getUrl(){  return data.url; } 
std::string& getRefUrl(){  return std::ref(data.url); } 
 void setUrl(std::string &val){  data.url=val;} 
 void setUrl(std::string_view val){  data.url=val;} 

std::string getUrlpath(){  return data.urlpath; } 
std::string& getRefUrlpath(){  return std::ref(data.urlpath); } 
 void setUrlpath(std::string &val){  data.urlpath=val;} 
 void setUrlpath(std::string_view val){  data.urlpath=val;} 

std::string getImgurl(){  return data.imgurl; } 
std::string& getRefImgurl(){  return std::ref(data.imgurl); } 
 void setImgurl(std::string &val){  data.imgurl=val;} 
 void setImgurl(std::string_view val){  data.imgurl=val;} 

std::string getTopimg(){  return data.topimg; } 
std::string& getRefTopimg(){  return std::ref(data.topimg); } 
 void setTopimg(std::string &val){  data.topimg=val;} 
 void setTopimg(std::string_view val){  data.topimg=val;} 

unsigned  int  getAccesscode(){  return data.accesscode; } 
 void setAccesscode(unsigned  int  val){  data.accesscode=val;} 

topicbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
topicbase::meta getData(){
 	 return data; 
} 
std::vector<topicbase::meta> getRecord(){
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
                     for(jj=0;jj<colnames.size();jj++){
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
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
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(tree_data[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(tree_data[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(tree_data[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(tree_data[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(tree_data[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(tree_data[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(tree_data[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(tree_data[n].accesscode);
}
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
                     for(jj=0;jj<colnames.size();jj++){
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
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
if(tree_data[n].parentid==0){
	tempsql<<"\"parentid\":0";
 }else{ 
	tempsql<<"\"parentid\":"<<std::to_string(tree_data[n].parentid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].cateid==0){
	tempsql<<"\"cateid\":0";
 }else{ 
	tempsql<<"\"cateid\":"<<std::to_string(tree_data[n].cateid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sorttype==0){
	tempsql<<"\"sorttype\":0";
 }else{ 
	tempsql<<"\"sorttype\":"<<std::to_string(tree_data[n].sorttype);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].languagetype==0){
	tempsql<<"\"languagetype\":0";
 }else{ 
	tempsql<<"\"languagetype\":"<<std::to_string(tree_data[n].languagetype);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isview==0){
	tempsql<<"\"isview\":0";
 }else{ 
	tempsql<<"\"isview\":"<<std::to_string(tree_data[n].isview);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isside==0){
	tempsql<<"\"isside\":0";
 }else{ 
	tempsql<<"\"isside\":"<<std::to_string(tree_data[n].isside);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(tree_data[n].title)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"twotitle\":\""<<http::utf8_to_jsonstring(tree_data[n].twotitle)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"templatename\":\""<<http::utf8_to_jsonstring(tree_data[n].templatename)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"url\":\""<<http::utf8_to_jsonstring(tree_data[n].url)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"urlpath\":\""<<http::utf8_to_jsonstring(tree_data[n].urlpath)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"imgurl\":\""<<http::utf8_to_jsonstring(tree_data[n].imgurl)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topimg\":\""<<http::utf8_to_jsonstring(tree_data[n].topimg)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].accesscode==0){
	tempsql<<"\"accesscode\":0";
 }else{ 
	tempsql<<"\"accesscode\":"<<std::to_string(tree_data[n].accesscode);
}
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
   

    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="twotitle")
		{
			return data.twotitle;
		}
		 if(key_name=="memo")
		{
			return data.memo;
		}
		 if(key_name=="templatename")
		{
			return data.templatename;
		}
		 if(key_name=="url")
		{
			return data.url;
		}
		 if(key_name=="urlpath")
		{
			return data.urlpath;
		}
		 if(key_name=="imgurl")
		{
			return data.imgurl;
		}
		 if(key_name=="topimg")
		{
			return data.topimg;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="topicid")
		{
			return data.topicid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="parentid")
		{
			return data.parentid;
		}
		 if(key_name=="cateid")
		{
			return data.cateid;
		}
		 if(key_name=="sorttype")
		{
			return data.sorttype;
		}
		 if(key_name=="languagetype")
		{
			return data.languagetype;
		}
		 if(key_name=="isview")
		{
			return data.isview;
		}
		 if(key_name=="isside")
		{
			return data.isside;
		}
		 if(key_name=="sortid")
		{
			return data.sortid;
		}
		 if(key_name=="accesscode")
		{
			return data.accesscode;
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
 				 a.emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a.emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a.emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a.emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a.emplace_back(iter.isview);
				 break;
			case 7: 
 				 a.emplace_back(iter.isside);
				 break;
			case 8: 
 				 a.emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a.emplace_back(iter.accesscode);
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
 				 return data.topicid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.parentid;
				 break;
			case 3: 
 				 return data.cateid;
				 break;
			case 4: 
 				 return data.sorttype;
				 break;
			case 5: 
 				 return data.languagetype;
				 break;
			case 6: 
 				 return data.isview;
				 break;
			case 7: 
 				 return data.isside;
				 break;
			case 8: 
 				 return data.sortid;
				 break;
			case 17: 
 				 return data.accesscode;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] topicbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.topicid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.parentid;
				 break;
			case 3: 
 				 return iter.cateid;
				 break;
			case 4: 
 				 return iter.sorttype;
				 break;
			case 5: 
 				 return iter.languagetype;
				 break;
			case 6: 
 				 return iter.isview;
				 break;
			case 7: 
 				 return iter.isside;
				 break;
			case 8: 
 				 return iter.sortid;
				 break;
			case 17: 
 				 return iter.accesscode;
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
            T getVal([[maybe_unused]] topicbase::meta & iter,std::string keyname)
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

   			case 9: 
 				 return data.title;
				 break;
			case 10: 
 				 return data.twotitle;
				 break;
			case 11: 
 				 return data.memo;
				 break;
			case 12: 
 				 return data.templatename;
				 break;
			case 13: 
 				 return data.url;
				 break;
			case 14: 
 				 return data.urlpath;
				 break;
			case 15: 
 				 return data.imgurl;
				 break;
			case 16: 
 				 return data.topimg;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] topicbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 9: 
 				 return iter.title;
				 break;
			case 10: 
 				 return iter.twotitle;
				 break;
			case 11: 
 				 return iter.memo;
				 break;
			case 12: 
 				 return iter.templatename;
				 break;
			case 13: 
 				 return iter.url;
				 break;
			case 14: 
 				 return iter.urlpath;
				 break;
			case 15: 
 				 return iter.imgurl;
				 break;
			case 16: 
 				 return iter.topimg;
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

    			case 9: 
 				 a.emplace_back(iter.title);
					 break;
			case 10: 
 				 a.emplace_back(iter.twotitle);
					 break;
			case 11: 
 				 a.emplace_back(iter.memo);
					 break;
			case 12: 
 				 a.emplace_back(iter.templatename);
					 break;
			case 13: 
 				 a.emplace_back(iter.url);
					 break;
			case 14: 
 				 a.emplace_back(iter.urlpath);
					 break;
			case 15: 
 				 a.emplace_back(iter.imgurl);
					 break;
			case 16: 
 				 a.emplace_back(iter.topimg);
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
 				 a<<std::to_string(iter.topicid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.parentid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.cateid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.sorttype);
				 break;
			case 5: 
 				 a<<std::to_string(iter.languagetype);
				 break;
			case 6: 
 				 a<<std::to_string(iter.isview);
				 break;
			case 7: 
 				 a<<std::to_string(iter.isside);
				 break;
			case 8: 
 				 a<<std::to_string(iter.sortid);
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.twotitle); 
				 }else{
				 a<<iter.twotitle;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.memo); 
				 }else{
				 a<<iter.memo;
				 }
				 break;
			case 12: 
 				 if(isyinhao){ a<<jsonaddslash(iter.templatename); 
				 }else{
				 a<<iter.templatename;
				 }
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.url); 
				 }else{
				 a<<iter.url;
				 }
				 break;
			case 14: 
 				 if(isyinhao){ a<<jsonaddslash(iter.urlpath); 
				 }else{
				 a<<iter.urlpath;
				 }
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.imgurl); 
				 }else{
				 a<<iter.imgurl;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.topimg); 
				 }else{
				 a<<iter.topimg;
				 }
				 break;
			case 17: 
 				 a<<std::to_string(iter.accesscode);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
				 } 
			switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
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
 
       			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
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
 	 ktemp=iter.topicid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.parentid;
	 break;
case 3: 
 	 ktemp=iter.cateid;
	 break;
case 4: 
 	 ktemp=iter.sorttype;
	 break;
case 5: 
 	 ktemp=iter.languagetype;
	 break;
case 6: 
 	 ktemp=iter.isview;
	 break;
case 7: 
 	 ktemp=iter.isside;
	 break;
case 8: 
 	 ktemp=iter.sortid;
	 break;
case 17: 
 	 ktemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }
 			switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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
 				 a.emplace(iter.topicid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.parentid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.cateid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.sorttype,iter);
				 break;
			case 5: 
 				 a.emplace(iter.languagetype,iter);
				 break;
			case 6: 
 				 a.emplace(iter.isview,iter);
				 break;
			case 7: 
 				 a.emplace(iter.isside,iter);
				 break;
			case 8: 
 				 a.emplace(iter.sortid,iter);
				 break;
			case 17: 
 				 a.emplace(iter.accesscode,iter);
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

   			case 9: 
 				 a.emplace(iter.title,iter);
			 break;
			case 10: 
 				 a.emplace(iter.twotitle,iter);
			 break;
			case 11: 
 				 a.emplace(iter.memo,iter);
			 break;
			case 12: 
 				 a.emplace(iter.templatename,iter);
			 break;
			case 13: 
 				 a.emplace(iter.url,iter);
			 break;
			case 14: 
 				 a.emplace(iter.urlpath,iter);
			 break;
			case 15: 
 				 a.emplace(iter.imgurl,iter);
			 break;
			case 16: 
 				 a.emplace(iter.topimg,iter);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
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
 				 ktemp=iter.topicid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.parentid;
			 break;
			case 3: 
 				 ktemp=iter.cateid;
			 break;
			case 4: 
 				 ktemp=iter.sorttype;
			 break;
			case 5: 
 				 ktemp=iter.languagetype;
			 break;
			case 6: 
 				 ktemp=iter.isview;
			 break;
			case 7: 
 				 ktemp=iter.isside;
			 break;
			case 8: 
 				 ktemp=iter.sortid;
			 break;
			case 17: 
 				 ktemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }
 			switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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

   case 9: 
 	 ktemp=iter.title;
	 break;
case 10: 
 	 ktemp=iter.twotitle;
	 break;
case 11: 
 	 ktemp=iter.memo;
	 break;
case 12: 
 	 ktemp=iter.templatename;
	 break;
case 13: 
 	 ktemp=iter.url;
	 break;
case 14: 
 	 ktemp=iter.urlpath;
	 break;
case 15: 
 	 ktemp=iter.imgurl;
	 break;
case 16: 
 	 ktemp=iter.topimg;
	 break;
	  }
 switch(vpos){
case 9: 
 	 vtemp=iter.title;
	 break;
case 10: 
 	 vtemp=iter.twotitle;
	 break;
case 11: 
 	 vtemp=iter.memo;
	 break;
case 12: 
 	 vtemp=iter.templatename;
	 break;
case 13: 
 	 vtemp=iter.url;
	 break;
case 14: 
 	 vtemp=iter.urlpath;
	 break;
case 15: 
 	 vtemp=iter.imgurl;
	 break;
case 16: 
 	 vtemp=iter.topimg;
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
 	 a.emplace_back(iter.topicid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.parentid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.cateid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.sorttype,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.languagetype,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.isview,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.isside,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.sortid,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.accesscode,iter);
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

   case 9: 
 	 a.emplace_back(iter.title,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.twotitle,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.memo,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.templatename,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.url,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.urlpath,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.imgurl,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.topimg,iter);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.accesscode);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
				 break;
			 }

			 switch(dpos){
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.topimg);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			 }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			 }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.accesscode);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
				 break;
			  }

			 switch(dpos){
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.topimg);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.accesscode);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.topicid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 3: 
 				 vtemp=iter.cateid;
				 break;
			case 4: 
 				 vtemp=iter.sorttype;
				 break;
			case 5: 
 				 vtemp=iter.languagetype;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isside;
				 break;
			case 8: 
 				 vtemp=iter.sortid;
				 break;
			case 17: 
 				 vtemp=iter.accesscode;
				 break;
			 }

			switch(dpos){
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.topimg);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			 }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.accesscode);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }

			 switch(vpos){
			case 9: 
 				 vtemp=iter.title;
				 break;
			case 10: 
 				 vtemp=iter.twotitle;
				 break;
			case 11: 
 				 vtemp=iter.memo;
				 break;
			case 12: 
 				 vtemp=iter.templatename;
				 break;
			case 13: 
 				 vtemp=iter.url;
				 break;
			case 14: 
 				 vtemp=iter.urlpath;
				 break;
			case 15: 
 				 vtemp=iter.imgurl;
				 break;
			case 16: 
 				 vtemp=iter.topimg;
				 break;
			  }

			 switch(dpos){
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.topimg);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }

			 switch(vpos){
			case 9: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.topimg);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.accesscode);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			 }

			 switch(vpos){
			case 9: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.twotitle);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.memo);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.templatename);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.url);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.urlpath);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.imgurl);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.topimg);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.cateid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.sorttype);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.languagetype);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.isside);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.accesscode);
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
 				 a[iter.topicid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.parentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.cateid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.sorttype].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.languagetype].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.isside].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.sortid].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.accesscode].emplace_back(iter);
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

   			case 9: 
 				 a[iter.title].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.twotitle].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.memo].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.templatename].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.url].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.urlpath].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.imgurl].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.topimg].emplace_back(iter);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
			 }

			 switch(vpos){
			case 9: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.twotitle].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.memo].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.templatename].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.url].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.imgurl].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.topimg].emplace_back(iter);
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

   			case 9: 
 				 ktemp=iter.title;
				 break;
			case 10: 
 				 ktemp=iter.twotitle;
				 break;
			case 11: 
 				 ktemp=iter.memo;
				 break;
			case 12: 
 				 ktemp=iter.templatename;
				 break;
			case 13: 
 				 ktemp=iter.url;
				 break;
			case 14: 
 				 ktemp=iter.urlpath;
				 break;
			case 15: 
 				 ktemp=iter.imgurl;
				 break;
			case 16: 
 				 ktemp=iter.topimg;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.cateid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.sorttype].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.languagetype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.isside].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.accesscode].emplace_back(iter);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.cateid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.sorttype].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.languagetype].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.isside].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.accesscode].emplace_back(iter);
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
 				 ktemp=iter.topicid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 3: 
 				 ktemp=iter.cateid;
				 break;
			case 4: 
 				 ktemp=iter.sorttype;
				 break;
			case 5: 
 				 ktemp=iter.languagetype;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isside;
				 break;
			case 8: 
 				 ktemp=iter.sortid;
				 break;
			case 17: 
 				 ktemp=iter.accesscode;
				 break;
			  }

			 switch(vpos){
			case 9: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.twotitle].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.memo].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.templatename].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.url].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.urlpath].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.imgurl].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.topimg].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   