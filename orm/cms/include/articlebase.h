#ifndef ORM_CMS_ARTICLEBASEMATA_H
#define ORM_CMS_ARTICLEBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Sat, 10 Dec 2022 05:33:36 GMT
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
#include <mysqlx/xdevapi.h> 
namespace orm { 
     namespace cms { 

struct articlebase
{
    struct meta{
    unsigned  int aid= 0; //
 int classtype= 0; //
 int userid= 0; //
 std::string title; //
 std::string keywords; //关键字
 std::string fromsource; //文章来源
 std::string author; //文章作者
 std::string addip; //
 std::string createtime; //显示的创建时间
unsigned  long long addtime=0; //添加或修改时间
 int readnum= 0; //
 int review= 0; //
 std::string icoimg; //列表图片
 std::string content; //
 char isopen=0; //是否开放
 char iscomment=0; //是否可以评论
 std::string fromlocal; //发表地址
 std::string texturl; //url用英文代替
 std::string summary; //文章摘要
 std::string editauthor; //文章编辑
 } data;
 std::vector<articlebase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<unsigned char> _keypos{0x00};
mysqlx::Row _row;
std::vector<articlebase::meta>::iterator begin(){     return record.begin(); }
std::vector<articlebase::meta>::iterator end(){     return record.end(); }
std::vector<articlebase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<articlebase::meta>::const_iterator end() const{     return record.end(); }
const std::array<std::string,20> colnames={"aid","classtype","userid","title","keywords","fromsource","author","addip","createtime","addtime","readnum","review","icoimg","content","isopen","iscomment","fromlocal","texturl","summary","editauthor"};
const std::array<unsigned char,20> colnamestype= {1,1,1,30,30,30,30,30,30,1,1,1,30,30,1,1,30,30,30,30};
std::string tablename="article";
std::string modelname="Article";

	  unsigned char findcolpos(std::string coln){
		    unsigned char  bi=coln[0];
            char colpospppc;
	         if(bi<91&&bi>64){
				bi+=32;
			}
            switch(coln[0]){


         case 'a':
 switch(coln.size()){  
case 3:
   	 return 0;
break;
case 5:
   	 return 7;
break;
case 6:
   	 return 6;
break;
case 7:
   	 return 9;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 7:
   	 return 13;
break;
case 9:
   	 return 1;
break;
case 10:
   	 return 8;
break;
 }
 break;
case 'e':
   	 return 19;
break;
case 'f':
 switch(coln.size()){  
case 9:
   	 return 16;
break;
case 10:
   	 return 5;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91&&bi>64){ colpospppc+=32; }
 if(colpospppc=='g'){ return 12; }
 if(colpospppc=='n'){ return 14; }
   	 break;
case 9:
   	 return 15;
break;
 }
 break;
case 'k':
   	 return 4;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 11;
break;
case 7:
   	 return 10;
break;
 }
 break;
case 's':
   	 return 18;
break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 3;
break;
case 7:
   	 return 17;
break;
 }
 break;
case 'u':
   	 return 2;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "aid";
}

      void _setColnamevalue(){
          articlebase::meta metatemp;   
         for(unsigned char i=0;i<_keypos.size();i++){
                 switch(_keypos[i]){
        		case 0: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.aid=_row[i].get<unsigned>();
		 } 
			 break;
		case 1: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.classtype=_row[i].get<unsigned>();
		 } 
			 break;
		case 2: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.userid=_row[i].get<unsigned>();
		 } 
			 break;
		case 3: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.title=(std::string)_row[i];
		 } 
			 break;
		case 4: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.keywords=(std::string)_row[i];
		 } 
			 break;
		case 5: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.fromsource=(std::string)_row[i];
		 } 
			 break;
		case 6: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.author=(std::string)_row[i];
		 } 
			 break;
		case 7: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.addip=(std::string)_row[i];
		 } 
			 break;
		case 8: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.createtime=(std::string)_row[i];
		 } 
			 break;
		case 9: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.addtime=_row[i].get<uint64_t>();
		 } 
			 break;
		case 10: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.readnum=_row[i].get<unsigned>();
		 } 
			 break;
		case 11: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.review=_row[i].get<unsigned>();
		 } 
			 break;
		case 12: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.icoimg=(std::string)_row[i];
		 } 
			 break;
		case 13: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.content=(std::string)_row[i];
		 } 
			 break;
		case 14: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.isopen=_row[i].get<unsigned>();
		 } 
			 break;
		case 15: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.iscomment=_row[i].get<unsigned>();
		 } 
			 break;
		case 16: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.fromlocal=(std::string)_row[i];
		 } 
			 break;
		case 17: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.texturl=(std::string)_row[i];
		 } 
			 break;
		case 18: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.summary=(std::string)_row[i];
		 } 
			 break;
		case 19: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.editauthor=(std::string)_row[i];
		 } 
			 break;

                 }

                 if(i>210){
                     break;
                 }
          }
          data=metatemp;
          record.emplace_back(metatemp);   
   } 
         void _addnewrowvalue(){
           articlebase::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        		case 0: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.aid=_row[i].get<unsigned>();
		 } 
			 break;
		case 1: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.classtype=_row[i].get<unsigned>();
		 } 
			 break;
		case 2: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.userid=_row[i].get<unsigned>();
		 } 
			 break;
		case 3: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.title=(std::string)_row[i];
		 } 
			 break;
		case 4: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.keywords=(std::string)_row[i];
		 } 
			 break;
		case 5: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.fromsource=(std::string)_row[i];
		 } 
			 break;
		case 6: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.author=(std::string)_row[i];
		 } 
			 break;
		case 7: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.addip=(std::string)_row[i];
		 } 
			 break;
		case 8: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.createtime=(std::string)_row[i];
		 } 
			 break;
		case 9: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.addtime=_row[i].get<uint64_t>();
		 } 
			 break;
		case 10: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.readnum=_row[i].get<unsigned>();
		 } 
			 break;
		case 11: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.review=_row[i].get<unsigned>();
		 } 
			 break;
		case 12: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.icoimg=(std::string)_row[i];
		 } 
			 break;
		case 13: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.content=(std::string)_row[i];
		 } 
			 break;
		case 14: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.isopen=_row[i].get<unsigned>();
		 } 
			 break;
		case 15: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::INT64||_row[i].getType()==mysqlx::Value::Type::UINT64){ 
				 metatemp.iscomment=_row[i].get<unsigned>();
		 } 
			 break;
		case 16: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.fromlocal=(std::string)_row[i];
		 } 
			 break;
		case 17: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.texturl=(std::string)_row[i];
		 } 
			 break;
		case 18: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.summary=(std::string)_row[i];
		 } 
			 break;
		case 19: 
 
	 if(_row[i].getType()==mysqlx::Value::Type::STRING){ 
				 metatemp.editauthor=(std::string)_row[i];
		 } 
			 break;

                  }
                 if(i>210){
                     break;
                 }
          }
           record.emplace_back(std::move(metatemp)); 
   } 

  inline  std::string stringaddslash(std::string &content){
        std::string temp;
        for(int i=0;i<content.size();i++){
            if(content[i]=='\''){
                temp.append("\\'");
                continue;
            }else if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            temp.push_back(content[i]);
        }
        return std::move(temp);
   }  
  inline  std::string jsonaddslash(std::string &content){
        std::string temp;
        for(int i=0;i<content.size();i++){
            if(content[i]=='"'){
                temp.append("\\\"");
                continue;
            }
            temp.push_back(content[i]);
        }
        return std::move(temp);
   }  

   std::string _makeinsertsql(){
       int j=0;
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

        if(data.aid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.aid);
}
if(data.classtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
tempsql<<",'"<<stringaddslash(data.title)<<"'";
tempsql<<",'"<<stringaddslash(data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(data.fromsource)<<"'";
tempsql<<",'"<<stringaddslash(data.author)<<"'";
tempsql<<",'"<<stringaddslash(data.addip)<<"'";
tempsql<<",'"<<stringaddslash(data.createtime)<<"'";
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.review);
}
tempsql<<",'"<<stringaddslash(data.icoimg)<<"'";
tempsql<<",'"<<stringaddslash(data.content)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.iscomment==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.iscomment);
}
tempsql<<",'"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",'"<<stringaddslash(data.texturl)<<"'";
tempsql<<",'"<<stringaddslash(data.summary)<<"'";
tempsql<<",'"<<stringaddslash(data.editauthor)<<"'";
tempsql<<")";

     
       return tempsql.str();
   }     
    std::string _makeupdatesql(std::string fileld){
       int j=0;
            std::ostringstream tempsql;
                 tempsql<<"UPDATE ";
                 tempsql<<tablename;
                 tempsql<<" SET ";

            bool isall=false;
            if(fileld.empty()){
                isall=true;
            }
            if(isall){

        if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
if(data.classtype==0){
	tempsql<<",`classtype`=0";
 }else{ 
	tempsql<<",`classtype`="<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
tempsql<<",`title`='"<<stringaddslash(data.title)<<"'";
tempsql<<",`keywords`='"<<stringaddslash(data.keywords)<<"'";
tempsql<<",`fromsource`='"<<stringaddslash(data.fromsource)<<"'";
tempsql<<",`author`='"<<stringaddslash(data.author)<<"'";
tempsql<<",`addip`='"<<stringaddslash(data.addip)<<"'";
tempsql<<",`createtime`='"<<stringaddslash(data.createtime)<<"'";
if(data.addtime==0){
	tempsql<<",`addtime`=0";
 }else{ 
	tempsql<<",`addtime`="<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",`readnum`=0";
 }else{ 
	tempsql<<",`readnum`="<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",`review`=0";
 }else{ 
	tempsql<<",`review`="<<std::to_string(data.review);
}
tempsql<<",`icoimg`='"<<stringaddslash(data.icoimg)<<"'";
tempsql<<",`content`='"<<stringaddslash(data.content)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.iscomment==0){
	tempsql<<",`iscomment`=0";
 }else{ 
	tempsql<<",`iscomment`="<<std::to_string(data.iscomment);
}
tempsql<<",`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",`texturl`='"<<stringaddslash(data.texturl)<<"'";
tempsql<<",`summary`='"<<stringaddslash(data.summary)<<"'";
tempsql<<",`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
 }else{ 

     
  int jj=0;
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
if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"`classtype`=0";
 }else{ 
	tempsql<<"`classtype`="<<std::to_string(data.classtype);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`keywords`='"<<stringaddslash(data.keywords)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromsource`='"<<stringaddslash(data.fromsource)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`author`='"<<stringaddslash(data.author)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addip`='"<<stringaddslash(data.addip)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`createtime`='"<<stringaddslash(data.createtime)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"`readnum`=0";
 }else{ 
	tempsql<<"`readnum`="<<std::to_string(data.readnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"`review`=0";
 }else{ 
	tempsql<<"`review`="<<std::to_string(data.review);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`icoimg`='"<<stringaddslash(data.icoimg)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"`iscomment`=0";
 }else{ 
	tempsql<<"`iscomment`="<<std::to_string(data.iscomment);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`texturl`='"<<stringaddslash(data.texturl)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`summary`='"<<stringaddslash(data.summary)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
 break;

     
                  default:
                                ;
                     }
                 }   

            }        

        return std::move(tempsql.str());
   } 
   std::string datatojson(){
       std::ostringstream tempsql;

        tempsql<<"{";
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
if(data.classtype==0){
	tempsql<<",\"classtype\":0";
 }else{ 
	tempsql<<",\"classtype\":"<<std::to_string(data.classtype);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
tempsql<<",\"title\":\""<<utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"keywords\":\""<<utf8_to_jsonstring(data.keywords);
tempsql<<"\"";
tempsql<<",\"fromsource\":\""<<utf8_to_jsonstring(data.fromsource);
tempsql<<"\"";
tempsql<<",\"author\":\""<<utf8_to_jsonstring(data.author);
tempsql<<"\"";
tempsql<<",\"addip\":\""<<utf8_to_jsonstring(data.addip);
tempsql<<"\"";
tempsql<<",\"createtime\":\""<<utf8_to_jsonstring(data.createtime);
tempsql<<"\"";
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.readnum==0){
	tempsql<<",\"readnum\":0";
 }else{ 
	tempsql<<",\"readnum\":"<<std::to_string(data.readnum);
}
if(data.review==0){
	tempsql<<",\"review\":0";
 }else{ 
	tempsql<<",\"review\":"<<std::to_string(data.review);
}
tempsql<<",\"icoimg\":\""<<utf8_to_jsonstring(data.icoimg);
tempsql<<"\"";
tempsql<<",\"content\":\""<<utf8_to_jsonstring(data.content);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.iscomment==0){
	tempsql<<",\"iscomment\":0";
 }else{ 
	tempsql<<",\"iscomment\":"<<std::to_string(data.iscomment);
}
tempsql<<",\"fromlocal\":\""<<utf8_to_jsonstring(data.fromlocal);
tempsql<<"\"";
tempsql<<",\"texturl\":\""<<utf8_to_jsonstring(data.texturl);
tempsql<<"\"";
tempsql<<",\"summary\":\""<<utf8_to_jsonstring(data.summary);
tempsql<<"\"";
tempsql<<",\"editauthor\":\""<<utf8_to_jsonstring(data.editauthor);
tempsql<<"\"";
tempsql<<"}";

     
     return std::move(tempsql.str());             
   }   
   
   std::string datatojson(std::string fileld){
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
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(data.classtype);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<utf8_to_jsonstring(data.keywords)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<utf8_to_jsonstring(data.fromsource)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<utf8_to_jsonstring(data.author)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<utf8_to_jsonstring(data.addip)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<utf8_to_jsonstring(data.createtime)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(data.readnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(data.review);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<utf8_to_jsonstring(data.icoimg)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(data.iscomment);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<utf8_to_jsonstring(data.fromlocal)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<utf8_to_jsonstring(data.texturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<utf8_to_jsonstring(data.summary)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<utf8_to_jsonstring(data.editauthor)<<"\"";
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
     return std::move(tempsql.str());             
   }   
   
   std::string tojson(std::string fileld=""){
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
              for(int n=0;n<record.size();n++){
                  if(n>0){
                      tempsql<<",{";
                  }else{
                      tempsql<<"{";
                  }  
                 
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return std::move(tempsql.str());             
   }   
   
   std::string tojson(std::function<bool(std::string&,meta&)> func,std::string fileld=""){
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
              for(int n=0;n<record.size();n++){
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
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;

                             default:
                                ;
                     }
                 }   
      tempsql<<"}";  
            }
      tempsql<<"]";
     return std::move(tempsql.str());             
   }   
   long long getPK(){  return data.aid; } 
 void setPK(long long val){  data.aid=val;} 
unsigned  int  getAid(){  return data.aid; } 
 void setAid(unsigned  int  val){  data.aid=val;} 

 int  getClasstype(){  return data.classtype; } 
 void setClasstype( int  val){  data.classtype=val;} 

 int  getUserid(){  return data.userid; } 
 void setUserid( int  val){  data.userid=val;} 

std::string getTitle(){  return data.title; } 
std::string& getRefTitle(){  return std::ref(data.title); } 
 void setTitle(std::string &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

std::string getKeywords(){  return data.keywords; } 
std::string& getRefKeywords(){  return std::ref(data.keywords); } 
 void setKeywords(std::string &val){  data.keywords=val;} 
 void setKeywords(std::string_view val){  data.keywords=val;} 

std::string getFromsource(){  return data.fromsource; } 
std::string& getRefFromsource(){  return std::ref(data.fromsource); } 
 void setFromsource(std::string &val){  data.fromsource=val;} 
 void setFromsource(std::string_view val){  data.fromsource=val;} 

std::string getAuthor(){  return data.author; } 
std::string& getRefAuthor(){  return std::ref(data.author); } 
 void setAuthor(std::string &val){  data.author=val;} 
 void setAuthor(std::string_view val){  data.author=val;} 

std::string getAddip(){  return data.addip; } 
std::string& getRefAddip(){  return std::ref(data.addip); } 
 void setAddip(std::string &val){  data.addip=val;} 
 void setAddip(std::string_view val){  data.addip=val;} 

std::string getCreatetime(){  return data.createtime; } 
std::string& getRefCreatetime(){  return std::ref(data.createtime); } 
 void setCreatetime(std::string &val){  data.createtime=val;} 
 void setCreatetime(std::string_view val){  data.createtime=val;} 

unsigned  long long  getAddtime(){  return data.addtime; } 
 void setAddtime(unsigned  long long  val){  data.addtime=val;} 

 int  getReadnum(){  return data.readnum; } 
 void setReadnum( int  val){  data.readnum=val;} 

 int  getReview(){  return data.review; } 
 void setReview( int  val){  data.review=val;} 

std::string getIcoimg(){  return data.icoimg; } 
std::string& getRefIcoimg(){  return std::ref(data.icoimg); } 
 void setIcoimg(std::string &val){  data.icoimg=val;} 
 void setIcoimg(std::string_view val){  data.icoimg=val;} 

std::string getContent(){  return data.content; } 
std::string& getRefContent(){  return std::ref(data.content); } 
 void setContent(std::string &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 int  getIsopen(){  return data.isopen; } 
 void setIsopen( int  val){  data.isopen=val;} 

 int  getIscomment(){  return data.iscomment; } 
 void setIscomment( int  val){  data.iscomment=val;} 

std::string getFromlocal(){  return data.fromlocal; } 
std::string& getRefFromlocal(){  return std::ref(data.fromlocal); } 
 void setFromlocal(std::string &val){  data.fromlocal=val;} 
 void setFromlocal(std::string_view val){  data.fromlocal=val;} 

std::string getTexturl(){  return data.texturl; } 
std::string& getRefTexturl(){  return std::ref(data.texturl); } 
 void setTexturl(std::string &val){  data.texturl=val;} 
 void setTexturl(std::string_view val){  data.texturl=val;} 

std::string getSummary(){  return data.summary; } 
std::string& getRefSummary(){  return std::ref(data.summary); } 
 void setSummary(std::string &val){  data.summary=val;} 
 void setSummary(std::string_view val){  data.summary=val;} 

std::string getEditauthor(){  return data.editauthor; } 
std::string& getRefEditauthor(){  return std::ref(data.editauthor); } 
 void setEditauthor(std::string &val){  data.editauthor=val;} 
 void setEditauthor(std::string_view val){  data.editauthor=val;} 

articlebase::meta getnewData(){
 	 struct meta newdata;
	 return std::move(newdata); 
} 
articlebase::meta getData(){
 	 return data; 
} 
std::vector<articlebase::meta> getRecord(){
 	 return record; 
} 

    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >  
    std::vector<T> getCol(std::string keyname){
         std::vector<T> a;
         unsigned char kpos;
         kpos=findcolpos(keyname);
        for(auto &iter:record){
                switch(kpos){

   case 0: 
 	 a.emplace_back(iter.aid);
	 break;
case 1: 
 	 a.emplace_back(iter.classtype);
	 break;
case 2: 
 	 a.emplace_back(iter.userid);
	 break;
case 9: 
 	 a.emplace_back(iter.addtime);
	 break;
case 10: 
 	 a.emplace_back(iter.readnum);
	 break;
case 11: 
 	 a.emplace_back(iter.review);
	 break;
case 14: 
 	 a.emplace_back(iter.isopen);
	 break;
case 15: 
 	 a.emplace_back(iter.iscomment);
	 break;

          }
        }
        return a;
    }
    
    template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true >    
    std::vector<T> getCol(std::string keyname){
        std::vector<T> a;
         unsigned char kpos;
         kpos=findcolpos(keyname);
        for(auto &iter:record){
                switch(kpos){

   
    }
            }
            return a;
        }  
    
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
    T getVal(std::string keyname){
         
         unsigned char kpos;
         kpos=findcolpos(keyname);
      
                switch(kpos){

   case 0: 
 	 return data.aid;
	 break;
case 1: 
 	 return data.classtype;
	 break;
case 2: 
 	 return data.userid;
	 break;
case 9: 
 	 return data.addtime;
	 break;
case 10: 
 	 return data.readnum;
	 break;
case 11: 
 	 return data.review;
	 break;
case 14: 
 	 return data.isopen;
	 break;
case 15: 
 	 return data.iscomment;
	 break;

    }
            return 0;
        }  
    
     template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
    T getVal(articlebase::meta & iter,std::string keyname){
         
         unsigned char kpos;
         kpos=findcolpos(keyname);
       
                switch(kpos){

   case 0: 
 	 return iter.aid;
	 break;
case 1: 
 	 return iter.classtype;
	 break;
case 2: 
 	 return iter.userid;
	 break;
case 9: 
 	 return iter.addtime;
	 break;
case 10: 
 	 return iter.readnum;
	 break;
case 11: 
 	 return iter.review;
	 break;
case 14: 
 	 return iter.isopen;
	 break;
case 15: 
 	 return iter.iscomment;
	 break;

    }
            return 0;
        }  
    
        template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
        T getVal(std::string keyname){
            
            unsigned char kpos;
            kpos=findcolpos(keyname);
        
            switch(kpos){

    
    }
            return 0.0;
        }  
    
     template<typename T, typename std::enable_if<std::is_floating_point_v<T>,bool>::type = true > 
    T getVal(articlebase::meta & iter,std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
         
                switch(kpos){

   
    }
            return 0.0;
        }  
    
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
    std::string getVal(std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
        
                switch(kpos){

   case 3: 
 	 return data.title;
	 break;
case 4: 
 	 return data.keywords;
	 break;
case 5: 
 	 return data.fromsource;
	 break;
case 6: 
 	 return data.author;
	 break;
case 7: 
 	 return data.addip;
	 break;
case 8: 
 	 return data.createtime;
	 break;
case 12: 
 	 return data.icoimg;
	 break;
case 13: 
 	 return data.content;
	 break;
case 16: 
 	 return data.fromlocal;
	 break;
case 17: 
 	 return data.texturl;
	 break;
case 18: 
 	 return data.summary;
	 break;
case 19: 
 	 return data.editauthor;
	 break;

  }
        return "";
    }  
   
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
    std::string getVal(articlebase::meta & iter,std::string keyname){
         
         unsigned char kpos;
        kpos=findcolpos(keyname);
       
                switch(kpos){

   case 3: 
 	 return iter.title;
	 break;
case 4: 
 	 return iter.keywords;
	 break;
case 5: 
 	 return iter.fromsource;
	 break;
case 6: 
 	 return iter.author;
	 break;
case 7: 
 	 return iter.addip;
	 break;
case 8: 
 	 return iter.createtime;
	 break;
case 12: 
 	 return iter.icoimg;
	 break;
case 13: 
 	 return iter.content;
	 break;
case 16: 
 	 return iter.fromlocal;
	 break;
case 17: 
 	 return iter.texturl;
	 break;
case 18: 
 	 return iter.summary;
	 break;
case 19: 
 	 return iter.editauthor;
	 break;

    }
            return "";
        }  
     
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >   
    std::vector<std::string> getCol(std::string keyname){
        std::vector<std::string> a;
         unsigned char kpos;
        kpos=findcolpos(keyname);
 
         for(auto &iter:record){
                switch(kpos){

   case 3: 
 	 a.emplace_back(iter.title);
	 break;
case 4: 
 	 a.emplace_back(iter.keywords);
	 break;
case 5: 
 	 a.emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a.emplace_back(iter.author);
	 break;
case 7: 
 	 a.emplace_back(iter.addip);
	 break;
case 8: 
 	 a.emplace_back(iter.createtime);
	 break;
case 12: 
 	 a.emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a.emplace_back(iter.content);
	 break;
case 16: 
 	 a.emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a.emplace_back(iter.texturl);
	 break;
case 18: 
 	 a.emplace_back(iter.summary);
	 break;
case 19: 
 	 a.emplace_back(iter.editauthor);
	 break;

   }
        }       

        return a;
    }  
    std::string getstrCol(std::string keyname,bool isyinhao=false){
        std::ostringstream a;
             unsigned char kpos;
        kpos=findcolpos(keyname);
  
         int j=0;
         if(isyinhao&&record.size()>0){
             a<<'"';
         }
         for(auto &iter:record){
                if(j>0){
                    if(isyinhao){
                        a<<"\",\"";
                    }else{
                        a<<',';    
                    }
                }
                 switch(kpos){

   case 0: 
 	 a<<std::to_string(iter.aid);
	 break;
case 1: 
 	 a<<std::to_string(iter.classtype);
	 break;
case 2: 
 	 a<<std::to_string(iter.userid);
	 break;
case 3: 
 	 if(isyinhao){ a<<jsonaddslash(iter.title); 
	 }else{
	 a<<iter.title;
	 }
	 break;
case 4: 
 	 if(isyinhao){ a<<jsonaddslash(iter.keywords); 
	 }else{
	 a<<iter.keywords;
	 }
	 break;
case 5: 
 	 if(isyinhao){ a<<jsonaddslash(iter.fromsource); 
	 }else{
	 a<<iter.fromsource;
	 }
	 break;
case 6: 
 	 if(isyinhao){ a<<jsonaddslash(iter.author); 
	 }else{
	 a<<iter.author;
	 }
	 break;
case 7: 
 	 if(isyinhao){ a<<jsonaddslash(iter.addip); 
	 }else{
	 a<<iter.addip;
	 }
	 break;
case 8: 
 	 if(isyinhao){ a<<jsonaddslash(iter.createtime); 
	 }else{
	 a<<iter.createtime;
	 }
	 break;
case 9: 
 	 a<<std::to_string(iter.addtime);
	 break;
case 10: 
 	 a<<std::to_string(iter.readnum);
	 break;
case 11: 
 	 a<<std::to_string(iter.review);
	 break;
case 12: 
 	 if(isyinhao){ a<<jsonaddslash(iter.icoimg); 
	 }else{
	 a<<iter.icoimg;
	 }
	 break;
case 13: 
 	 if(isyinhao){ a<<jsonaddslash(iter.content); 
	 }else{
	 a<<iter.content;
	 }
	 break;
case 14: 
 	 a<<std::to_string(iter.isopen);
	 break;
case 15: 
 	 a<<std::to_string(iter.iscomment);
	 break;
case 16: 
 	 if(isyinhao){ a<<jsonaddslash(iter.fromlocal); 
	 }else{
	 a<<iter.fromlocal;
	 }
	 break;
case 17: 
 	 if(isyinhao){ a<<jsonaddslash(iter.texturl); 
	 }else{
	 a<<iter.texturl;
	 }
	 break;
case 18: 
 	 if(isyinhao){ a<<jsonaddslash(iter.summary); 
	 }else{
	 a<<iter.summary;
	 }
	 break;
case 19: 
 	 if(isyinhao){ a<<jsonaddslash(iter.editauthor); 
	 }else{
	 a<<iter.editauthor;
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
    std::map<std::string,std::string> getCols(std::string keyname,std::string valname){
        std::map<std::string,std::string> a;
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         std::string ktemp,vtemp;
         for(auto &iter:record){
                
                switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	 } 
		 switch(vpos){ 
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;

        }
                    a.emplace(ktemp,vtemp);
            }       
            return a;
        } 

  template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<std::string,U> getCols(std::string keyname,std::string valname){
        std::map<std::string,U> a;
        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         std::string ktemp;
         U vtemp;
         for(auto &iter:record){
                
                switch(kpos){
 
       case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	 } 
        switch(vpos){ 

            }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    } 
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>       
  std::map<T,U> getCols(std::string keyname,std::string valname){
        std::map<T,U> a;
        unsigned char kpos,vpos;
         kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
         T ktemp;
         U vtemp;
         for(auto &iter:record){
                
                switch(kpos){
 
       case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	 } 
 		   switch(vpos){ 

             }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    }  
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>      
    std::map<T,std::string> getCols(std::string keyname,std::string valname){
        std::map<T,std::string> a;
                unsigned char kpos,vpos;
         kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             std::string vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;

  }
                a.emplace(ktemp,vtemp);
        }       
        return a;
    }     
     template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>       
    std::map<std::string,U> getCols(std::string keyname,std::string valname){
        std::map<std::string,U> a;
             unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             std::string  ktemp;
             U  vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }
 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;

                 }

             
                a.emplace(ktemp,vtemp);
        }       
        return a;
    }  
    template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>   
    std::map<T,U> getCols(std::string keyname,std::string valname){
        std::map<T,U> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }
 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;

                   }

                    a.emplace(ktemp,vtemp);
            }       

            return a;
        }   
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >         
    std::map<T,meta> getmapRows(std::string keyname){
        std::map<T,meta> a;

            unsigned char kpos;
         kpos=findcolpos(keyname);
 
          //   long long ktemp;
        
         for(auto &iter:record){
                switch(kpos){

   case 0: 
 	 a.emplace(iter.aid,iter);
	 break;
case 1: 
 	 a.emplace(iter.classtype,iter);
	 break;
case 2: 
 	 a.emplace(iter.userid,iter);
	 break;
case 9: 
 	 a.emplace(iter.addtime,iter);
	 break;
case 10: 
 	 a.emplace(iter.readnum,iter);
	 break;
case 11: 
 	 a.emplace(iter.review,iter);
	 break;
case 14: 
 	 a.emplace(iter.isopen,iter);
	 break;
case 15: 
 	 a.emplace(iter.iscomment,iter);
	 break;

                }
        }       
        return a;
    }     
   template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >    
   std::map<std::string,meta> getmapRows(std::string keyname){
        std::map<std::string,meta> a;

         unsigned char kpos;
 
        kpos=findcolpos(keyname);
    

        //std::string ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 a.emplace(iter.title,iter);
	 break;
case 4: 
 	 a.emplace(iter.keywords,iter);
	 break;
case 5: 
 	 a.emplace(iter.fromsource,iter);
	 break;
case 6: 
 	 a.emplace(iter.author,iter);
	 break;
case 7: 
 	 a.emplace(iter.addip,iter);
	 break;
case 8: 
 	 a.emplace(iter.createtime,iter);
	 break;
case 12: 
 	 a.emplace(iter.icoimg,iter);
	 break;
case 13: 
 	 a.emplace(iter.content,iter);
	 break;
case 16: 
 	 a.emplace(iter.fromlocal,iter);
	 break;
case 17: 
 	 a.emplace(iter.texturl,iter);
	 break;
case 18: 
 	 a.emplace(iter.summary,iter);
	 break;
case 19: 
 	 a.emplace(iter.editauthor,iter);
	 break;

                }

                //a.emplace(ktemp,iter);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>  
    std::vector<std::pair<std::string,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<std::string,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             std::string ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }
 switch(vpos){

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }
 switch(vpos){

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
  template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
  template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>     
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }
 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
  template<typename T,typename U, typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }
 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
  template<typename T,typename U, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>     
  std::vector<std::pair<T,U>> getvecCols(std::string keyname,std::string valname){
        std::vector<std::pair<T,U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
             T ktemp;
             U vtemp;
         for(auto &iter:record){
             
                switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;

                   }

                    a.emplace_back(ktemp,vtemp);
            }       

            return a;
        }   
    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true >   
    std::vector<std::pair<T,meta>> getvecRows(std::string keyname){
        std::vector<std::pair<T,meta>> a;

         unsigned char kpos;
 
        kpos=findcolpos(keyname);
    

       // T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 a.emplace_back(iter.aid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.classtype,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.addtime,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.readnum,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.review,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.iscomment,iter);
	 break;

                }

                
        }       

        return a;
    }
    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true >  
    std::vector<std::pair<std::string,meta>> getvecRows(std::string keyname){
        std::vector<std::pair<std::string,meta>> a;

         unsigned char kpos;
        kpos=findcolpos(keyname);
        //std::string ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 a.emplace_back(iter.title,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.keywords,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.fromsource,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.author,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.addip,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.createtime,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.icoimg,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.content,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.fromlocal,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.texturl,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.summary,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.editauthor,iter);
	 break;

                }

                
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        
        T ktemp;
        U vtemp;
        //D vtemp;
         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        //D vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){
case 0: 
 	 a[ktemp][vtemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp][vtemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp][vtemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp][vtemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp][vtemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp][vtemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp][vtemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp][vtemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){
case 3: 
 	 a[ktemp][vtemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp][vtemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp][vtemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp][vtemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp][vtemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp][vtemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp][vtemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp][vtemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp][vtemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp][vtemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp][vtemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp][vtemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){
case 0: 
 	 a[ktemp][vtemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp][vtemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp][vtemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp][vtemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp][vtemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp][vtemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp][vtemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp][vtemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){
case 3: 
 	 a[ktemp][vtemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp][vtemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp][vtemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp][vtemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp][vtemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp][vtemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp][vtemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp][vtemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp][vtemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp][vtemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp][vtemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp][vtemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        
        T ktemp;
        U vtemp;
        //D vtemp;
         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
        //D vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){
case 0: 
 	 a[ktemp][vtemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp][vtemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp][vtemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp][vtemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp][vtemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp][vtemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp][vtemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp][vtemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 0: 
 	 vtemp=iter.aid;
	 break;
case 1: 
 	 vtemp=iter.classtype;
	 break;
case 2: 
 	 vtemp=iter.userid;
	 break;
case 9: 
 	 vtemp=iter.addtime;
	 break;
case 10: 
 	 vtemp=iter.readnum;
	 break;
case 11: 
 	 vtemp=iter.review;
	 break;
case 14: 
 	 vtemp=iter.isopen;
	 break;
case 15: 
 	 vtemp=iter.iscomment;
	 break;
	  }

 switch(dpos){
case 3: 
 	 a[ktemp][vtemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp][vtemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp][vtemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp][vtemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp][vtemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp][vtemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp][vtemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp][vtemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp][vtemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp][vtemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp][vtemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp][vtemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_floating_point<D>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_integral_v<D>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){
case 0: 
 	 a[ktemp][vtemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp][vtemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp][vtemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp][vtemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp][vtemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp][vtemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp][vtemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp][vtemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<D,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<D>>> getgroupCols(std::string keyname,std::string valname,std::string dataname){
        std::map<T,std::map<U,std::vector<D>>> a;

        unsigned char kpos,vpos,dpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        dpos=findcolpos(dataname);
        T ktemp;
        U vtemp;
       // D dtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 3: 
 	 vtemp=iter.title;
	 break;
case 4: 
 	 vtemp=iter.keywords;
	 break;
case 5: 
 	 vtemp=iter.fromsource;
	 break;
case 6: 
 	 vtemp=iter.author;
	 break;
case 7: 
 	 vtemp=iter.addip;
	 break;
case 8: 
 	 vtemp=iter.createtime;
	 break;
case 12: 
 	 vtemp=iter.icoimg;
	 break;
case 13: 
 	 vtemp=iter.content;
	 break;
case 16: 
 	 vtemp=iter.fromlocal;
	 break;
case 17: 
 	 vtemp=iter.texturl;
	 break;
case 18: 
 	 vtemp=iter.summary;
	 break;
case 19: 
 	 vtemp=iter.editauthor;
	 break;
	  }

 switch(dpos){
case 3: 
 	 a[ktemp][vtemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp][vtemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp][vtemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp][vtemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp][vtemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp][vtemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp][vtemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp][vtemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp][vtemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp][vtemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp][vtemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp][vtemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 3: 
 	 a[ktemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 0: 
 	 a[ktemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 3: 
 	 a[ktemp].emplace_back(iter.title);
	 break;
case 4: 
 	 a[ktemp].emplace_back(iter.keywords);
	 break;
case 5: 
 	 a[ktemp].emplace_back(iter.fromsource);
	 break;
case 6: 
 	 a[ktemp].emplace_back(iter.author);
	 break;
case 7: 
 	 a[ktemp].emplace_back(iter.addip);
	 break;
case 8: 
 	 a[ktemp].emplace_back(iter.createtime);
	 break;
case 12: 
 	 a[ktemp].emplace_back(iter.icoimg);
	 break;
case 13: 
 	 a[ktemp].emplace_back(iter.content);
	 break;
case 16: 
 	 a[ktemp].emplace_back(iter.fromlocal);
	 break;
case 17: 
 	 a[ktemp].emplace_back(iter.texturl);
	 break;
case 18: 
 	 a[ktemp].emplace_back(iter.summary);
	 break;
case 19: 
 	 a[ktemp].emplace_back(iter.editauthor);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_floating_point<U>::value,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::vector<U>> getgroupCols(std::string keyname,std::string valname){
        std::map<T,std::vector<U>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);

        T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 0: 
 	 a[ktemp].emplace_back(iter.aid);
	 break;
case 1: 
 	 a[ktemp].emplace_back(iter.classtype);
	 break;
case 2: 
 	 a[ktemp].emplace_back(iter.userid);
	 break;
case 9: 
 	 a[ktemp].emplace_back(iter.addtime);
	 break;
case 10: 
 	 a[ktemp].emplace_back(iter.readnum);
	 break;
case 11: 
 	 a[ktemp].emplace_back(iter.review);
	 break;
case 14: 
 	 a[ktemp].emplace_back(iter.isopen);
	 break;
case 15: 
 	 a[ktemp].emplace_back(iter.iscomment);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename std::enable_if<std::is_integral_v<T>,bool>::type = true>    
  std::map<T,std::vector<meta>> getgroupRows(std::string keyname){
        std::map<T,std::vector<meta>> a;

        unsigned char kpos;
        kpos=findcolpos(keyname);

        //T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 a[iter.aid].emplace_back(iter);
	 break;
case 1: 
 	 a[iter.classtype].emplace_back(iter);
	 break;
case 2: 
 	 a[iter.userid].emplace_back(iter);
	 break;
case 9: 
 	 a[iter.addtime].emplace_back(iter);
	 break;
case 10: 
 	 a[iter.readnum].emplace_back(iter);
	 break;
case 11: 
 	 a[iter.review].emplace_back(iter);
	 break;
case 14: 
 	 a[iter.isopen].emplace_back(iter);
	 break;
case 15: 
 	 a[iter.iscomment].emplace_back(iter);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>    
  std::map<T,std::vector<meta>> getgroupRows(std::string keyname){
        std::map<T,std::vector<meta>> a;

        unsigned char kpos;
        kpos=findcolpos(keyname);

        //T ktemp;
        //U vtemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 a[iter.title].emplace_back(iter);
	 break;
case 4: 
 	 a[iter.keywords].emplace_back(iter);
	 break;
case 5: 
 	 a[iter.fromsource].emplace_back(iter);
	 break;
case 6: 
 	 a[iter.author].emplace_back(iter);
	 break;
case 7: 
 	 a[iter.addip].emplace_back(iter);
	 break;
case 8: 
 	 a[iter.createtime].emplace_back(iter);
	 break;
case 12: 
 	 a[iter.icoimg].emplace_back(iter);
	 break;
case 13: 
 	 a[iter.content].emplace_back(iter);
	 break;
case 16: 
 	 a[iter.fromlocal].emplace_back(iter);
	 break;
case 17: 
 	 a[iter.texturl].emplace_back(iter);
	 break;
case 18: 
 	 a[iter.summary].emplace_back(iter);
	 break;
case 19: 
 	 a[iter.editauthor].emplace_back(iter);
	 break;

                   }
                //a[ktemp].emplace_back(vtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename D,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true, typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;

         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 3: 
 	 a[ktemp][iter.title].emplace_back(iter);
	 break;
case 4: 
 	 a[ktemp][iter.keywords].emplace_back(iter);
	 break;
case 5: 
 	 a[ktemp][iter.fromsource].emplace_back(iter);
	 break;
case 6: 
 	 a[ktemp][iter.author].emplace_back(iter);
	 break;
case 7: 
 	 a[ktemp][iter.addip].emplace_back(iter);
	 break;
case 8: 
 	 a[ktemp][iter.createtime].emplace_back(iter);
	 break;
case 12: 
 	 a[ktemp][iter.icoimg].emplace_back(iter);
	 break;
case 13: 
 	 a[ktemp][iter.content].emplace_back(iter);
	 break;
case 16: 
 	 a[ktemp][iter.fromlocal].emplace_back(iter);
	 break;
case 17: 
 	 a[ktemp][iter.texturl].emplace_back(iter);
	 break;
case 18: 
 	 a[ktemp][iter.summary].emplace_back(iter);
	 break;
case 19: 
 	 a[ktemp][iter.editauthor].emplace_back(iter);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 3: 
 	 ktemp=iter.title;
	 break;
case 4: 
 	 ktemp=iter.keywords;
	 break;
case 5: 
 	 ktemp=iter.fromsource;
	 break;
case 6: 
 	 ktemp=iter.author;
	 break;
case 7: 
 	 ktemp=iter.addip;
	 break;
case 8: 
 	 ktemp=iter.createtime;
	 break;
case 12: 
 	 ktemp=iter.icoimg;
	 break;
case 13: 
 	 ktemp=iter.content;
	 break;
case 16: 
 	 ktemp=iter.fromlocal;
	 break;
case 17: 
 	 ktemp=iter.texturl;
	 break;
case 18: 
 	 ktemp=iter.summary;
	 break;
case 19: 
 	 ktemp=iter.editauthor;
	 break;
	  }

 switch(vpos){
case 0: 
 	 a[ktemp][iter.aid].emplace_back(iter);
	 break;
case 1: 
 	 a[ktemp][iter.classtype].emplace_back(iter);
	 break;
case 2: 
 	 a[ktemp][iter.userid].emplace_back(iter);
	 break;
case 9: 
 	 a[ktemp][iter.addtime].emplace_back(iter);
	 break;
case 10: 
 	 a[ktemp][iter.readnum].emplace_back(iter);
	 break;
case 11: 
 	 a[ktemp][iter.review].emplace_back(iter);
	 break;
case 14: 
 	 a[ktemp][iter.isopen].emplace_back(iter);
	 break;
case 15: 
 	 a[ktemp][iter.iscomment].emplace_back(iter);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<U>,bool>::type = true,typename std::enable_if<std::is_integral_v<U>,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 0: 
 	 a[ktemp][iter.aid].emplace_back(iter);
	 break;
case 1: 
 	 a[ktemp][iter.classtype].emplace_back(iter);
	 break;
case 2: 
 	 a[ktemp][iter.userid].emplace_back(iter);
	 break;
case 9: 
 	 a[ktemp][iter.addtime].emplace_back(iter);
	 break;
case 10: 
 	 a[ktemp][iter.readnum].emplace_back(iter);
	 break;
case 11: 
 	 a[ktemp][iter.review].emplace_back(iter);
	 break;
case 14: 
 	 a[ktemp][iter.isopen].emplace_back(iter);
	 break;
case 15: 
 	 a[ktemp][iter.iscomment].emplace_back(iter);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
    template<typename T,typename U,typename std::enable_if<std::is_integral_v<T>,bool>::type = true,typename std::enable_if<std::is_same<U,std::string>::value,bool>::type = true>    
  std::map<T,std::map<U,std::vector<meta>>> getgroupRows(std::string keyname,std::string valname){
        std::map<T,std::map<U,std::vector<meta>>> a;

        unsigned char kpos,vpos;
        kpos=findcolpos(keyname);
        vpos=findcolpos(valname);
        T ktemp;
        
         for(auto &iter:record){
             
                   switch(kpos){

   case 0: 
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.classtype;
	 break;
case 2: 
 	 ktemp=iter.userid;
	 break;
case 9: 
 	 ktemp=iter.addtime;
	 break;
case 10: 
 	 ktemp=iter.readnum;
	 break;
case 11: 
 	 ktemp=iter.review;
	 break;
case 14: 
 	 ktemp=iter.isopen;
	 break;
case 15: 
 	 ktemp=iter.iscomment;
	 break;
	  }

 switch(vpos){
case 3: 
 	 a[ktemp][iter.title].emplace_back(iter);
	 break;
case 4: 
 	 a[ktemp][iter.keywords].emplace_back(iter);
	 break;
case 5: 
 	 a[ktemp][iter.fromsource].emplace_back(iter);
	 break;
case 6: 
 	 a[ktemp][iter.author].emplace_back(iter);
	 break;
case 7: 
 	 a[ktemp][iter.addip].emplace_back(iter);
	 break;
case 8: 
 	 a[ktemp][iter.createtime].emplace_back(iter);
	 break;
case 12: 
 	 a[ktemp][iter.icoimg].emplace_back(iter);
	 break;
case 13: 
 	 a[ktemp][iter.content].emplace_back(iter);
	 break;
case 16: 
 	 a[ktemp][iter.fromlocal].emplace_back(iter);
	 break;
case 17: 
 	 a[ktemp][iter.texturl].emplace_back(iter);
	 break;
case 18: 
 	 a[ktemp][iter.summary].emplace_back(iter);
	 break;
case 19: 
 	 a[ktemp][iter.editauthor].emplace_back(iter);
	 break;

                   }
                //a[ktemp][vtemp].emplace_back(dtemp);
        }       

        return a;
    }
};
 } 
}
#endif
   