#ifndef ORM_CMS_ARTICLEBASEMATA_H
#define ORM_CMS_ARTICLEBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Wed, 26 Mar 2025 14:12:12 GMT
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

struct articlebase
{
    struct meta{
     unsigned  int  aid = 0; ///**/
 unsigned  int  topicid = 0; ///*栏目id*/
 unsigned  int  classtype = 0; ///**/
 unsigned  int  userid = 0; ///**/
 int  sortid = 0; ///*排序*/
 std::string  topicname = ""; ///**/
 std::string  title = ""; ///**/
 std::string  keywords = ""; ///*关键字*/
 std::string  fromsource = ""; ///*文章来源*/
 std::string  author = ""; ///*文章作者*/
 std::string  addip = ""; ///**/
 std::string  createtime = ""; ///*显示的创建时间*/
 unsigned  long long  addtime = 0; ///*添加或修改时间*/
 int  readnum = 0; ///**/
 int  review = 0; ///**/
 std::string  icoimg = ""; ///*列表图片*/
 std::string  content = ""; ///**/
 std::string  mdcontent = ""; ///*markdown content*/
 char  isopen = 0; ///*是否开放*/
 char  ishome = 0; ///*show homepage*/
 char  iscomment = 0; ///*是否可以评论*/
 char  showtype = 0; ///*art content display type*/
 std::string  fromlocal = ""; ///*发表地址*/
 std::string  texturl = ""; ///*url用英文代替*/
 std::string  summary = ""; ///*文章摘要*/
 std::string  editauthor = ""; ///*文章编辑*/
 std::string  relatecontent = ""; ///*相关内容*/
 } data;
 std::vector<articlebase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<articlebase::meta>::iterator begin(){     return record.begin(); }
std::vector<articlebase::meta>::iterator end(){     return record.end(); }
std::vector<articlebase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<articlebase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string_view,27> col_names={"aid","topicid","classtype","userid","sortid","topicname","title","keywords","fromsource","author","addip","createtime","addtime","readnum","review","icoimg","content","mdcontent","isopen","ishome","iscomment","showtype","fromlocal","texturl","summary","editauthor","relatecontent"};
static constexpr std::array<unsigned char,27> col_types={3,3,3,3,3,253,253,253,253,253,253,253,8,3,3,253,252,252,1,1,1,1,253,253,253,253,253};
static constexpr std::array<unsigned char,27> col_length={0,0,0,0,0,140,128,128,60,40,20,20,0,0,0,255,0,0,0,0,0,0,60,255,255,40,0};
static constexpr std::array<unsigned char,27> col_decimals={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="article";
static constexpr std::string_view modelname="Article";

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
case 3:
   	 return 0;
break;
case 5:
   	 return 10;
break;
case 6:
   	 return 9;
break;
case 7:
   	 return 12;
break;
 }
 break;
case 'c':
 switch(coln.size()){  
case 7:
   	 return 16;
break;
case 9:
   	 return 2;
break;
case 10:
   	 return 11;
break;
 }
 break;
case 'e':
   	 return 25;
break;
case 'f':
 switch(coln.size()){  
case 9:
   	 return 22;
break;
case 10:
   	 return 8;
break;
 }
 break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 19; }
 if(colpospppc=='g'){ return 15; }
 if(colpospppc=='n'){ return 18; }
   	 break;
case 9:
   	 return 20;
break;
 }
 break;
case 'k':
   	 return 7;
break;
case 'm':
   	 return 17;
break;
case 'r':
 switch(coln.size()){  
case 6:
   	 return 14;
break;
case 7:
   	 return 13;
break;
case 13:
   	 return 26;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
   	 return 4;
break;
case 7:
   	 return 24;
break;
case 8:
   	 return 21;
break;
 }
 break;
case 't':
 switch(coln.size()){  
case 5:
   	 return 6;
break;
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 1; }
 if(colpospppc=='l'){ return 23; }
   	 break;
case 9:
   	 return 5;
break;
 }
 break;
case 'u':
   	 return 3;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "aid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     articlebase::meta metatemp;    
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

        if(data.aid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.topicid);
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
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.topicname)<<"'";
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
tempsql<<",'"<<stringaddslash(data.mdcontent)<<"'";
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.showtype);
}
tempsql<<",'"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",'"<<stringaddslash(data.texturl)<<"'";
tempsql<<",'"<<stringaddslash(data.summary)<<"'";
tempsql<<",'"<<stringaddslash(data.editauthor)<<"'";
tempsql<<",'"<<stringaddslash(data.relatecontent)<<"'";
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

        if(insert_data.aid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.aid);
}
if(insert_data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.topicid);
}
if(insert_data.classtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.classtype);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.topicname)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.title)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.fromsource)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.author)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.addip)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.createtime)<<"'";
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.readnum==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.readnum);
}
if(insert_data.review==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.review);
}
tempsql<<",'"<<stringaddslash(insert_data.icoimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.content)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.mdcontent)<<"'";
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
if(insert_data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ishome);
}
if(insert_data.iscomment==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.iscomment);
}
if(insert_data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.showtype);
}
tempsql<<",'"<<stringaddslash(insert_data.fromlocal)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.texturl)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.summary)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.editauthor)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.relatecontent)<<"'";
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


            	if(insert_data[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].aid);
	}
	if(insert_data[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].classtype);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].topicname)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].title)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].keywords)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].fromsource)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].author)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].addip)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].createtime)<<"'";
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].readnum);
	}
	if(insert_data[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].review);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].icoimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].content)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].mdcontent)<<"'";
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
	if(insert_data[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ishome);
	}
	if(insert_data[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].iscomment);
	}
	if(insert_data[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].showtype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].fromlocal)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].texturl)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].summary)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].editauthor)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].relatecontent)<<"'";
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

        if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",`topicid`=0";
 }else{ 
	tempsql<<",`topicid`="<<std::to_string(data.topicid);
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
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`topicname`='"<<stringaddslash(data.topicname)<<"'";
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
tempsql<<",`mdcontent`='"<<stringaddslash(data.mdcontent)<<"'";
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",`ishome`=0";
 }else{ 
	tempsql<<",`ishome`="<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",`iscomment`=0";
 }else{ 
	tempsql<<",`iscomment`="<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",`showtype`=0";
 }else{ 
	tempsql<<",`showtype`="<<std::to_string(data.showtype);
}
tempsql<<",`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
tempsql<<",`texturl`='"<<stringaddslash(data.texturl)<<"'";
tempsql<<",`summary`='"<<stringaddslash(data.summary)<<"'";
tempsql<<",`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
tempsql<<",`relatecontent`='"<<stringaddslash(data.relatecontent)<<"'";
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
if(data.aid==0){
	tempsql<<"`aid`=0";
 }else{ 
	tempsql<<"`aid`="<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"`classtype`=0";
 }else{ 
	tempsql<<"`classtype`="<<std::to_string(data.classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"`userid`=0";
 }else{ 
	tempsql<<"`userid`="<<std::to_string(data.userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`topicname`='"<<stringaddslash(data.topicname)<<"'";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`title`='"<<stringaddslash(data.title)<<"'";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`keywords`='"<<stringaddslash(data.keywords)<<"'";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromsource`='"<<stringaddslash(data.fromsource)<<"'";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`author`='"<<stringaddslash(data.author)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`addip`='"<<stringaddslash(data.addip)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`createtime`='"<<stringaddslash(data.createtime)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"`readnum`=0";
 }else{ 
	tempsql<<"`readnum`="<<std::to_string(data.readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"`review`=0";
 }else{ 
	tempsql<<"`review`="<<std::to_string(data.review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`icoimg`='"<<stringaddslash(data.icoimg)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`content`='"<<stringaddslash(data.content)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`mdcontent`='"<<stringaddslash(data.mdcontent)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"`ishome`=0";
 }else{ 
	tempsql<<"`ishome`="<<std::to_string(data.ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"`iscomment`=0";
 }else{ 
	tempsql<<"`iscomment`="<<std::to_string(data.iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"`showtype`=0";
 }else{ 
	tempsql<<"`showtype`="<<std::to_string(data.showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`fromlocal`='"<<stringaddslash(data.fromlocal)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`texturl`='"<<stringaddslash(data.texturl)<<"'";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`summary`='"<<stringaddslash(data.summary)<<"'";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`editauthor`='"<<stringaddslash(data.editauthor)<<"'";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`relatecontent`='"<<stringaddslash(data.relatecontent)<<"'";
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
            	if(record[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].aid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].classtype);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].topicname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fromsource)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].author)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].createtime)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readnum);
	}
	if(record[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].review);
	}
	tempsql<<",'"<<stringaddslash(record[i].icoimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mdcontent)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscomment);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].fromlocal)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].texturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].summary)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].relatecontent)<<"'";
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
            	if(record[i].aid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].aid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].classtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].classtype);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].topicname)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].title)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].fromsource)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].author)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].addip)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].createtime)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].readnum==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].readnum);
	}
	if(record[i].review==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].review);
	}
	tempsql<<",'"<<stringaddslash(record[i].icoimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].content)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].mdcontent)<<"'";
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].iscomment==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].iscomment);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].fromlocal)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].texturl)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].summary)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editauthor)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].relatecontent)<<"'";
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
if(data.aid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.aid));
}
 break;
 case 1:
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 2:
if(data.classtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.classtype));
}
 break;
 case 3:
if(data.userid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.userid));
}
 break;
 case 4:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 5:
	temparray.push_back(data.topicname);
 break;
 case 6:
	temparray.push_back(data.title);
 break;
 case 7:
	temparray.push_back(data.keywords);
 break;
 case 8:
	temparray.push_back(data.fromsource);
 break;
 case 9:
	temparray.push_back(data.author);
 break;
 case 10:
	temparray.push_back(data.addip);
 break;
 case 11:
	temparray.push_back(data.createtime);
 break;
 case 12:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 13:
if(data.readnum==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.readnum));
}
 break;
 case 14:
if(data.review==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.review));
}
 break;
 case 15:
	temparray.push_back(data.icoimg);
 break;
 case 16:
	temparray.push_back(data.content);
 break;
 case 17:
	temparray.push_back(data.mdcontent);
 break;
 case 18:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 19:
if(data.ishome==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ishome));
}
 break;
 case 20:
if(data.iscomment==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.iscomment));
}
 break;
 case 21:
if(data.showtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.showtype));
}
 break;
 case 22:
	temparray.push_back(data.fromlocal);
 break;
 case 23:
	temparray.push_back(data.texturl);
 break;
 case 24:
	temparray.push_back(data.summary);
 break;
 case 25:
	temparray.push_back(data.editauthor);
 break;
 case 26:
	temparray.push_back(data.relatecontent);
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
if(data.aid==0){
	tempsql.insert({"aid","0"});
 }else{ 
	tempsql.insert({"aid",std::to_string(data.aid)});
}
 break;
 case 1:
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 2:
if(data.classtype==0){
	tempsql.insert({"classtype","0"});
 }else{ 
	tempsql.insert({"classtype",std::to_string(data.classtype)});
}
 break;
 case 3:
if(data.userid==0){
	tempsql.insert({"userid","0"});
 }else{ 
	tempsql.insert({"userid",std::to_string(data.userid)});
}
 break;
 case 4:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 5:
	tempsql.insert({"topicname",data.topicname});
 break;
 case 6:
	tempsql.insert({"title",data.title});
 break;
 case 7:
	tempsql.insert({"keywords",data.keywords});
 break;
 case 8:
	tempsql.insert({"fromsource",data.fromsource});
 break;
 case 9:
	tempsql.insert({"author",data.author});
 break;
 case 10:
	tempsql.insert({"addip",data.addip});
 break;
 case 11:
	tempsql.insert({"createtime",data.createtime});
 break;
 case 12:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 13:
if(data.readnum==0){
	tempsql.insert({"readnum","0"});
 }else{ 
	tempsql.insert({"readnum",std::to_string(data.readnum)});
}
 break;
 case 14:
if(data.review==0){
	tempsql.insert({"review","0"});
 }else{ 
	tempsql.insert({"review",std::to_string(data.review)});
}
 break;
 case 15:
	tempsql.insert({"icoimg",data.icoimg});
 break;
 case 16:
	tempsql.insert({"content",data.content});
 break;
 case 17:
	tempsql.insert({"mdcontent",data.mdcontent});
 break;
 case 18:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 19:
if(data.ishome==0){
	tempsql.insert({"ishome","0"});
 }else{ 
	tempsql.insert({"ishome",std::to_string(data.ishome)});
}
 break;
 case 20:
if(data.iscomment==0){
	tempsql.insert({"iscomment","0"});
 }else{ 
	tempsql.insert({"iscomment",std::to_string(data.iscomment)});
}
 break;
 case 21:
if(data.showtype==0){
	tempsql.insert({"showtype","0"});
 }else{ 
	tempsql.insert({"showtype",std::to_string(data.showtype)});
}
 break;
 case 22:
	tempsql.insert({"fromlocal",data.fromlocal});
 break;
 case 23:
	tempsql.insert({"texturl",data.texturl});
 break;
 case 24:
	tempsql.insert({"summary",data.summary});
 break;
 case 25:
	tempsql.insert({"editauthor",data.editauthor});
 break;
 case 26:
	tempsql.insert({"relatecontent",data.relatecontent});
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
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
if(data.topicid==0){
	tempsql<<",\"topicid\":0";
 }else{ 
	tempsql<<",\"topicid\":"<<std::to_string(data.topicid);
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
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"topicname\":\""<<http::utf8_to_jsonstring(data.topicname);
tempsql<<"\"";
tempsql<<",\"title\":\""<<http::utf8_to_jsonstring(data.title);
tempsql<<"\"";
tempsql<<",\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords);
tempsql<<"\"";
tempsql<<",\"fromsource\":\""<<http::utf8_to_jsonstring(data.fromsource);
tempsql<<"\"";
tempsql<<",\"author\":\""<<http::utf8_to_jsonstring(data.author);
tempsql<<"\"";
tempsql<<",\"addip\":\""<<http::utf8_to_jsonstring(data.addip);
tempsql<<"\"";
tempsql<<",\"createtime\":\""<<http::utf8_to_jsonstring(data.createtime);
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
tempsql<<",\"icoimg\":\""<<http::utf8_to_jsonstring(data.icoimg);
tempsql<<"\"";
tempsql<<",\"content\":\""<<http::utf8_to_jsonstring(data.content);
tempsql<<"\"";
tempsql<<",\"mdcontent\":\""<<http::utf8_to_jsonstring(data.mdcontent);
tempsql<<"\"";
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
if(data.ishome==0){
	tempsql<<",\"ishome\":0";
 }else{ 
	tempsql<<",\"ishome\":"<<std::to_string(data.ishome);
}
if(data.iscomment==0){
	tempsql<<",\"iscomment\":0";
 }else{ 
	tempsql<<",\"iscomment\":"<<std::to_string(data.iscomment);
}
if(data.showtype==0){
	tempsql<<",\"showtype\":0";
 }else{ 
	tempsql<<",\"showtype\":"<<std::to_string(data.showtype);
}
tempsql<<",\"fromlocal\":\""<<http::utf8_to_jsonstring(data.fromlocal);
tempsql<<"\"";
tempsql<<",\"texturl\":\""<<http::utf8_to_jsonstring(data.texturl);
tempsql<<"\"";
tempsql<<",\"summary\":\""<<http::utf8_to_jsonstring(data.summary);
tempsql<<"\"";
tempsql<<",\"editauthor\":\""<<http::utf8_to_jsonstring(data.editauthor);
tempsql<<"\"";
tempsql<<",\"relatecontent\":\""<<http::utf8_to_jsonstring(data.relatecontent);
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
if(data.aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(data.aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(data.classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(data.classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(data.userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(data.topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(data.title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(data.fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(data.author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(data.addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(data.createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(data.readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(data.readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(data.review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(data.review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(data.icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(data.content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(data.mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(data.ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(data.iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(data.showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(data.fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(data.texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(data.summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(data.editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(data.relatecontent)<<"\"";
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
        articlebase::meta metatemp; 
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
			data.aid=std::stoul(set_value_name);
		}catch (...) { 
			data.aid=0;
			 }
			break;
		case 1:
		 try{
			data.topicid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 2:
		 try{
			data.classtype=std::stoul(set_value_name);
		}catch (...) { 
			data.classtype=0;
			 }
			break;
		case 3:
		 try{
			data.userid=std::stoul(set_value_name);
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 4:
		 try{
			data.sortid=std::stoi(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 5:
		 try{
			data.topicname.append(set_value_name);
		}catch (...) { 
			data.topicname.clear();
			 }
			break;
		case 6:
		 try{
			data.title.append(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 7:
		 try{
			data.keywords.append(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 8:
		 try{
			data.fromsource.append(set_value_name);
		}catch (...) { 
			data.fromsource.clear();
			 }
			break;
		case 9:
		 try{
			data.author.append(set_value_name);
		}catch (...) { 
			data.author.clear();
			 }
			break;
		case 10:
		 try{
			data.addip.append(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 11:
		 try{
			data.createtime.append(set_value_name);
		}catch (...) { 
			data.createtime.clear();
			 }
			break;
		case 12:
		 try{
			data.addtime=std::stoull(set_value_name);
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 13:
		 try{
			data.readnum=std::stoi(set_value_name);
		}catch (...) { 
			data.readnum=0;
			 }
			break;
		case 14:
		 try{
			data.review=std::stoi(set_value_name);
		}catch (...) { 
			data.review=0;
			 }
			break;
		case 15:
		 try{
			data.icoimg.append(set_value_name);
		}catch (...) { 
			data.icoimg.clear();
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
			data.mdcontent.append(set_value_name);
		}catch (...) { 
			data.mdcontent.clear();
			 }
			break;
		case 18:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 19:
		 try{
			data.ishome=std::stoi(set_value_name);
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 20:
		 try{
			data.iscomment=std::stoi(set_value_name);
		}catch (...) { 
			data.iscomment=0;
			 }
			break;
		case 21:
		 try{
			data.showtype=std::stoi(set_value_name);
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 22:
		 try{
			data.fromlocal.append(set_value_name);
		}catch (...) { 
			data.fromlocal.clear();
			 }
			break;
		case 23:
		 try{
			data.texturl.append(set_value_name);
		}catch (...) { 
			data.texturl.clear();
			 }
			break;
		case 24:
		 try{
			data.summary.append(set_value_name);
		}catch (...) { 
			data.summary.clear();
			 }
			break;
		case 25:
		 try{
			data.editauthor.append(set_value_name);
		}catch (...) { 
			data.editauthor.clear();
			 }
			break;
		case 26:
		 try{
			data.relatecontent.append(set_value_name);
		}catch (...) { 
			data.relatecontent.clear();
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
			data.aid=set_value_name;
		}catch (...) { 
			data.aid=0;
			 }
			break;
		case 1:
		 try{
			data.topicid=set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 2:
		 try{
			data.classtype=set_value_name;
		}catch (...) { 
			data.classtype=0;
			 }
			break;
		case 3:
		 try{
			data.userid=set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 4:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 5:
		 try{
			data.topicname=std::to_string(set_value_name);
		}catch (...) { 
			data.topicname.clear();
			 }
			break;
		case 6:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 7:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 8:
		 try{
			data.fromsource=std::to_string(set_value_name);
		}catch (...) { 
			data.fromsource.clear();
			 }
			break;
		case 9:
		 try{
			data.author=std::to_string(set_value_name);
		}catch (...) { 
			data.author.clear();
			 }
			break;
		case 10:
		 try{
			data.addip=std::to_string(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 11:
		 try{
			data.createtime=std::to_string(set_value_name);
		}catch (...) { 
			data.createtime.clear();
			 }
			break;
		case 12:
		 try{
			data.addtime=set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 13:
		 try{
			data.readnum=set_value_name;
		}catch (...) { 
			data.readnum=0;
			 }
			break;
		case 14:
		 try{
			data.review=set_value_name;
		}catch (...) { 
			data.review=0;
			 }
			break;
		case 15:
		 try{
			data.icoimg=std::to_string(set_value_name);
		}catch (...) { 
			data.icoimg.clear();
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
			data.mdcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.mdcontent.clear();
			 }
			break;
		case 18:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 19:
		 try{
			data.ishome=set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 20:
		 try{
			data.iscomment=set_value_name;
		}catch (...) { 
			data.iscomment=0;
			 }
			break;
		case 21:
		 try{
			data.showtype=set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 22:
		 try{
			data.fromlocal=std::to_string(set_value_name);
		}catch (...) { 
			data.fromlocal.clear();
			 }
			break;
		case 23:
		 try{
			data.texturl=std::to_string(set_value_name);
		}catch (...) { 
			data.texturl.clear();
			 }
			break;
		case 24:
		 try{
			data.summary=std::to_string(set_value_name);
		}catch (...) { 
			data.summary.clear();
			 }
			break;
		case 25:
		 try{
			data.editauthor=std::to_string(set_value_name);
		}catch (...) { 
			data.editauthor.clear();
			 }
			break;
		case 26:
		 try{
			data.relatecontent=std::to_string(set_value_name);
		}catch (...) { 
			data.relatecontent.clear();
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
			data.aid=(unsigned int)set_value_name;
		}catch (...) { 
			data.aid=0;
			 }
			break;
		case 1:
		 try{
			data.topicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 2:
		 try{
			data.classtype=(unsigned int)set_value_name;
		}catch (...) { 
			data.classtype=0;
			 }
			break;
		case 3:
		 try{
			data.userid=(unsigned int)set_value_name;
		}catch (...) { 
			data.userid=0;
			 }
			break;
		case 4:
		 try{
			data.sortid=(int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 5:
		 try{
			data.topicname=std::to_string(set_value_name);
		}catch (...) { 
			data.topicname.clear();
			 }
			break;
		case 6:
		 try{
			data.title=std::to_string(set_value_name);
		}catch (...) { 
			data.title.clear();
			 }
			break;
		case 7:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 8:
		 try{
			data.fromsource=std::to_string(set_value_name);
		}catch (...) { 
			data.fromsource.clear();
			 }
			break;
		case 9:
		 try{
			data.author=std::to_string(set_value_name);
		}catch (...) { 
			data.author.clear();
			 }
			break;
		case 10:
		 try{
			data.addip=std::to_string(set_value_name);
		}catch (...) { 
			data.addip.clear();
			 }
			break;
		case 11:
		 try{
			data.createtime=std::to_string(set_value_name);
		}catch (...) { 
			data.createtime.clear();
			 }
			break;
		case 12:
		 try{
			data.addtime=(unsigned long long)set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 13:
		 try{
			data.readnum=(int)set_value_name;
		}catch (...) { 
			data.readnum=0;
			 }
			break;
		case 14:
		 try{
			data.review=(int)set_value_name;
		}catch (...) { 
			data.review=0;
			 }
			break;
		case 15:
		 try{
			data.icoimg=std::to_string(set_value_name);
		}catch (...) { 
			data.icoimg.clear();
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
			data.mdcontent=std::to_string(set_value_name);
		}catch (...) { 
			data.mdcontent.clear();
			 }
			break;
		case 18:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 19:
		 try{
			data.ishome=(int)set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 20:
		 try{
			data.iscomment=(int)set_value_name;
		}catch (...) { 
			data.iscomment=0;
			 }
			break;
		case 21:
		 try{
			data.showtype=(int)set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 22:
		 try{
			data.fromlocal=std::to_string(set_value_name);
		}catch (...) { 
			data.fromlocal.clear();
			 }
			break;
		case 23:
		 try{
			data.texturl=std::to_string(set_value_name);
		}catch (...) { 
			data.texturl.clear();
			 }
			break;
		case 24:
		 try{
			data.summary=std::to_string(set_value_name);
		}catch (...) { 
			data.summary.clear();
			 }
			break;
		case 25:
		 try{
			data.editauthor=std::to_string(set_value_name);
		}catch (...) { 
			data.editauthor.clear();
			 }
			break;
		case 26:
		 try{
			data.relatecontent=std::to_string(set_value_name);
		}catch (...) { 
			data.relatecontent.clear();
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
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(record[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(record[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(record[n].relatecontent)<<"\"";
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
if(record[n].aid==0){
	tempsql<<"\"aid\":0";
 }else{ 
	tempsql<<"\"aid\":"<<std::to_string(record[n].aid);
}
 break;
 case 1:
 if(jj>0){ tempsql<<","; } 
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 2:
 if(jj>0){ tempsql<<","; } 
if(record[n].classtype==0){
	tempsql<<"\"classtype\":0";
 }else{ 
	tempsql<<"\"classtype\":"<<std::to_string(record[n].classtype);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].userid==0){
	tempsql<<"\"userid\":0";
 }else{ 
	tempsql<<"\"userid\":"<<std::to_string(record[n].userid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"topicname\":\""<<http::utf8_to_jsonstring(record[n].topicname)<<"\"";
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"title\":\""<<http::utf8_to_jsonstring(record[n].title)<<"\"";
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromsource\":\""<<http::utf8_to_jsonstring(record[n].fromsource)<<"\"";
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"author\":\""<<http::utf8_to_jsonstring(record[n].author)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"addip\":\""<<http::utf8_to_jsonstring(record[n].addip)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"createtime\":\""<<http::utf8_to_jsonstring(record[n].createtime)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
if(record[n].readnum==0){
	tempsql<<"\"readnum\":0";
 }else{ 
	tempsql<<"\"readnum\":"<<std::to_string(record[n].readnum);
}
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
if(record[n].review==0){
	tempsql<<"\"review\":0";
 }else{ 
	tempsql<<"\"review\":"<<std::to_string(record[n].review);
}
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"icoimg\":\""<<http::utf8_to_jsonstring(record[n].icoimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"content\":\""<<http::utf8_to_jsonstring(record[n].content)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"mdcontent\":\""<<http::utf8_to_jsonstring(record[n].mdcontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].iscomment==0){
	tempsql<<"\"iscomment\":0";
 }else{ 
	tempsql<<"\"iscomment\":"<<std::to_string(record[n].iscomment);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"fromlocal\":\""<<http::utf8_to_jsonstring(record[n].fromlocal)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"texturl\":\""<<http::utf8_to_jsonstring(record[n].texturl)<<"\"";
 break;
 case 24:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"summary\":\""<<http::utf8_to_jsonstring(record[n].summary)<<"\"";
 break;
 case 25:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editauthor\":\""<<http::utf8_to_jsonstring(record[n].editauthor)<<"\"";
 break;
 case 26:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"relatecontent\":\""<<http::utf8_to_jsonstring(record[n].relatecontent)<<"\"";
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
   long long getPK(){  return data.aid; } 
 void setPK(long long val){  data.aid=val;} 
 unsigned  int  getAid(){  return data.aid; } 
 void setAid( unsigned  int  val){  data.aid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getClasstype(){  return data.classtype; } 
 void setClasstype( unsigned  int  val){  data.classtype=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 int  getSortid(){  return data.sortid; } 
 void setSortid( int  val){  data.sortid=val;} 

 std::string  getTopicname(){  return data.topicname; } 
 std::string & getRefTopicname(){  return std::ref(data.topicname); } 
 void setTopicname( std::string  &val){  data.topicname=val;} 
 void setTopicname(std::string_view val){  data.topicname=val;} 

 std::string  getTitle(){  return data.title; } 
 std::string & getRefTitle(){  return std::ref(data.title); } 
 void setTitle( std::string  &val){  data.title=val;} 
 void setTitle(std::string_view val){  data.title=val;} 

 std::string  getKeywords(){  return data.keywords; } 
 std::string & getRefKeywords(){  return std::ref(data.keywords); } 
 void setKeywords( std::string  &val){  data.keywords=val;} 
 void setKeywords(std::string_view val){  data.keywords=val;} 

 std::string  getFromsource(){  return data.fromsource; } 
 std::string & getRefFromsource(){  return std::ref(data.fromsource); } 
 void setFromsource( std::string  &val){  data.fromsource=val;} 
 void setFromsource(std::string_view val){  data.fromsource=val;} 

 std::string  getAuthor(){  return data.author; } 
 std::string & getRefAuthor(){  return std::ref(data.author); } 
 void setAuthor( std::string  &val){  data.author=val;} 
 void setAuthor(std::string_view val){  data.author=val;} 

 std::string  getAddip(){  return data.addip; } 
 std::string & getRefAddip(){  return std::ref(data.addip); } 
 void setAddip( std::string  &val){  data.addip=val;} 
 void setAddip(std::string_view val){  data.addip=val;} 

 std::string  getCreatetime(){  return data.createtime; } 
 std::string & getRefCreatetime(){  return std::ref(data.createtime); } 
 void setCreatetime( std::string  &val){  data.createtime=val;} 
 void setCreatetime(std::string_view val){  data.createtime=val;} 

 unsigned  long long  getAddtime(){  return data.addtime; } 
 void setAddtime( unsigned  long long  val){  data.addtime=val;} 

 int  getReadnum(){  return data.readnum; } 
 void setReadnum( int  val){  data.readnum=val;} 

 int  getReview(){  return data.review; } 
 void setReview( int  val){  data.review=val;} 

 std::string  getIcoimg(){  return data.icoimg; } 
 std::string & getRefIcoimg(){  return std::ref(data.icoimg); } 
 void setIcoimg( std::string  &val){  data.icoimg=val;} 
 void setIcoimg(std::string_view val){  data.icoimg=val;} 

 std::string  getContent(){  return data.content; } 
 std::string & getRefContent(){  return std::ref(data.content); } 
 void setContent( std::string  &val){  data.content=val;} 
 void setContent(std::string_view val){  data.content=val;} 

 std::string  getMdcontent(){  return data.mdcontent; } 
 std::string & getRefMdcontent(){  return std::ref(data.mdcontent); } 
 void setMdcontent( std::string  &val){  data.mdcontent=val;} 
 void setMdcontent(std::string_view val){  data.mdcontent=val;} 

 char  getIsopen(){  return data.isopen; } 
 void setIsopen( char  val){  data.isopen=val;} 

 char  getIshome(){  return data.ishome; } 
 void setIshome( char  val){  data.ishome=val;} 

 char  getIscomment(){  return data.iscomment; } 
 void setIscomment( char  val){  data.iscomment=val;} 

 char  getShowtype(){  return data.showtype; } 
 void setShowtype( char  val){  data.showtype=val;} 

 std::string  getFromlocal(){  return data.fromlocal; } 
 std::string & getRefFromlocal(){  return std::ref(data.fromlocal); } 
 void setFromlocal( std::string  &val){  data.fromlocal=val;} 
 void setFromlocal(std::string_view val){  data.fromlocal=val;} 

 std::string  getTexturl(){  return data.texturl; } 
 std::string & getRefTexturl(){  return std::ref(data.texturl); } 
 void setTexturl( std::string  &val){  data.texturl=val;} 
 void setTexturl(std::string_view val){  data.texturl=val;} 

 std::string  getSummary(){  return data.summary; } 
 std::string & getRefSummary(){  return std::ref(data.summary); } 
 void setSummary( std::string  &val){  data.summary=val;} 
 void setSummary(std::string_view val){  data.summary=val;} 

 std::string  getEditauthor(){  return data.editauthor; } 
 std::string & getRefEditauthor(){  return std::ref(data.editauthor); } 
 void setEditauthor( std::string  &val){  data.editauthor=val;} 
 void setEditauthor(std::string_view val){  data.editauthor=val;} 

 std::string  getRelatecontent(){  return data.relatecontent; } 
 std::string & getRefRelatecontent(){  return std::ref(data.relatecontent); } 
 void setRelatecontent( std::string  &val){  data.relatecontent=val;} 
 void setRelatecontent(std::string_view val){  data.relatecontent=val;} 

articlebase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
articlebase::meta getData(){
 	 return data; 
} 
std::vector<articlebase::meta> getRecord(){
 	 return record; 
} 


    template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="topicname")
		{
			return data.topicname;
		}
		 if(key_name=="title")
		{
			return data.title;
		}
		 if(key_name=="keywords")
		{
			return data.keywords;
		}
		 if(key_name=="fromsource")
		{
			return data.fromsource;
		}
		 if(key_name=="author")
		{
			return data.author;
		}
		 if(key_name=="addip")
		{
			return data.addip;
		}
		 if(key_name=="createtime")
		{
			return data.createtime;
		}
		 if(key_name=="icoimg")
		{
			return data.icoimg;
		}
		 if(key_name=="content")
		{
			return data.content;
		}
		 if(key_name=="mdcontent")
		{
			return data.mdcontent;
		}
		 if(key_name=="fromlocal")
		{
			return data.fromlocal;
		}
		 if(key_name=="texturl")
		{
			return data.texturl;
		}
		 if(key_name=="summary")
		{
			return data.summary;
		}
		 if(key_name=="editauthor")
		{
			return data.editauthor;
		}
		 if(key_name=="relatecontent")
		{
			return data.relatecontent;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="aid")
		{
			return data.aid;
		}
		 if(key_name=="topicid")
		{
			return data.topicid;
		}
		 if(key_name=="classtype")
		{
			return data.classtype;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="sortid")
		{
			return data.sortid;
		}
		 if(key_name=="addtime")
		{
			return data.addtime;
		}
		 if(key_name=="readnum")
		{
			return data.readnum;
		}
		 if(key_name=="review")
		{
			return data.review;
		}
		 if(key_name=="isopen")
		{
			return data.isopen;
		}
		 if(key_name=="ishome")
		{
			return data.ishome;
		}
		 if(key_name=="iscomment")
		{
			return data.iscomment;
		}
		 if(key_name=="showtype")
		{
			return data.showtype;
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
 				 a.emplace_back(iter.aid);
				 break;
			case 1: 
 				 a.emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a.emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a.emplace_back(iter.userid);
				 break;
			case 4: 
 				 a.emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a.emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a.emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a.emplace_back(iter.review);
				 break;
			case 18: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a.emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a.emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a.emplace_back(iter.showtype);
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
 				 return data.aid;
				 break;
			case 1: 
 				 return data.topicid;
				 break;
			case 2: 
 				 return data.classtype;
				 break;
			case 3: 
 				 return data.userid;
				 break;
			case 4: 
 				 return data.sortid;
				 break;
			case 12: 
 				 return data.addtime;
				 break;
			case 13: 
 				 return data.readnum;
				 break;
			case 14: 
 				 return data.review;
				 break;
			case 18: 
 				 return data.isopen;
				 break;
			case 19: 
 				 return data.ishome;
				 break;
			case 20: 
 				 return data.iscomment;
				 break;
			case 21: 
 				 return data.showtype;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] articlebase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.aid;
				 break;
			case 1: 
 				 return iter.topicid;
				 break;
			case 2: 
 				 return iter.classtype;
				 break;
			case 3: 
 				 return iter.userid;
				 break;
			case 4: 
 				 return iter.sortid;
				 break;
			case 12: 
 				 return iter.addtime;
				 break;
			case 13: 
 				 return iter.readnum;
				 break;
			case 14: 
 				 return iter.review;
				 break;
			case 18: 
 				 return iter.isopen;
				 break;
			case 19: 
 				 return iter.ishome;
				 break;
			case 20: 
 				 return iter.iscomment;
				 break;
			case 21: 
 				 return iter.showtype;
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
            T getVal([[maybe_unused]] articlebase::meta & iter,std::string keyname)
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
 				 return data.topicname;
				 break;
			case 6: 
 				 return data.title;
				 break;
			case 7: 
 				 return data.keywords;
				 break;
			case 8: 
 				 return data.fromsource;
				 break;
			case 9: 
 				 return data.author;
				 break;
			case 10: 
 				 return data.addip;
				 break;
			case 11: 
 				 return data.createtime;
				 break;
			case 15: 
 				 return data.icoimg;
				 break;
			case 16: 
 				 return data.content;
				 break;
			case 17: 
 				 return data.mdcontent;
				 break;
			case 22: 
 				 return data.fromlocal;
				 break;
			case 23: 
 				 return data.texturl;
				 break;
			case 24: 
 				 return data.summary;
				 break;
			case 25: 
 				 return data.editauthor;
				 break;
			case 26: 
 				 return data.relatecontent;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] articlebase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 5: 
 				 return iter.topicname;
				 break;
			case 6: 
 				 return iter.title;
				 break;
			case 7: 
 				 return iter.keywords;
				 break;
			case 8: 
 				 return iter.fromsource;
				 break;
			case 9: 
 				 return iter.author;
				 break;
			case 10: 
 				 return iter.addip;
				 break;
			case 11: 
 				 return iter.createtime;
				 break;
			case 15: 
 				 return iter.icoimg;
				 break;
			case 16: 
 				 return iter.content;
				 break;
			case 17: 
 				 return iter.mdcontent;
				 break;
			case 22: 
 				 return iter.fromlocal;
				 break;
			case 23: 
 				 return iter.texturl;
				 break;
			case 24: 
 				 return iter.summary;
				 break;
			case 25: 
 				 return iter.editauthor;
				 break;
			case 26: 
 				 return iter.relatecontent;
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
 				 a.emplace_back(iter.topicname);
					 break;
			case 6: 
 				 a.emplace_back(iter.title);
					 break;
			case 7: 
 				 a.emplace_back(iter.keywords);
					 break;
			case 8: 
 				 a.emplace_back(iter.fromsource);
					 break;
			case 9: 
 				 a.emplace_back(iter.author);
					 break;
			case 10: 
 				 a.emplace_back(iter.addip);
					 break;
			case 11: 
 				 a.emplace_back(iter.createtime);
					 break;
			case 15: 
 				 a.emplace_back(iter.icoimg);
					 break;
			case 16: 
 				 a.emplace_back(iter.content);
					 break;
			case 17: 
 				 a.emplace_back(iter.mdcontent);
					 break;
			case 22: 
 				 a.emplace_back(iter.fromlocal);
					 break;
			case 23: 
 				 a.emplace_back(iter.texturl);
					 break;
			case 24: 
 				 a.emplace_back(iter.summary);
					 break;
			case 25: 
 				 a.emplace_back(iter.editauthor);
					 break;
			case 26: 
 				 a.emplace_back(iter.relatecontent);
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
 				 a<<std::to_string(iter.aid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.topicid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.classtype);
				 break;
			case 3: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.sortid);
				 break;
			case 5: 
 				 if(isyinhao){ a<<jsonaddslash(iter.topicname); 
				 }else{
				 a<<iter.topicname;
				 }
				 break;
			case 6: 
 				 if(isyinhao){ a<<jsonaddslash(iter.title); 
				 }else{
				 a<<iter.title;
				 }
				 break;
			case 7: 
 				 if(isyinhao){ a<<jsonaddslash(iter.keywords); 
				 }else{
				 a<<iter.keywords;
				 }
				 break;
			case 8: 
 				 if(isyinhao){ a<<jsonaddslash(iter.fromsource); 
				 }else{
				 a<<iter.fromsource;
				 }
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.author); 
				 }else{
				 a<<iter.author;
				 }
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.addip); 
				 }else{
				 a<<iter.addip;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.createtime); 
				 }else{
				 a<<iter.createtime;
				 }
				 break;
			case 12: 
 				 a<<std::to_string(iter.addtime);
				 break;
			case 13: 
 				 a<<std::to_string(iter.readnum);
				 break;
			case 14: 
 				 a<<std::to_string(iter.review);
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.icoimg); 
				 }else{
				 a<<iter.icoimg;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.content); 
				 }else{
				 a<<iter.content;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.mdcontent); 
				 }else{
				 a<<iter.mdcontent;
				 }
				 break;
			case 18: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 19: 
 				 a<<std::to_string(iter.ishome);
				 break;
			case 20: 
 				 a<<std::to_string(iter.iscomment);
				 break;
			case 21: 
 				 a<<std::to_string(iter.showtype);
				 break;
			case 22: 
 				 if(isyinhao){ a<<jsonaddslash(iter.fromlocal); 
				 }else{
				 a<<iter.fromlocal;
				 }
				 break;
			case 23: 
 				 if(isyinhao){ a<<jsonaddslash(iter.texturl); 
				 }else{
				 a<<iter.texturl;
				 }
				 break;
			case 24: 
 				 if(isyinhao){ a<<jsonaddslash(iter.summary); 
				 }else{
				 a<<iter.summary;
				 }
				 break;
			case 25: 
 				 if(isyinhao){ a<<jsonaddslash(iter.editauthor); 
				 }else{
				 a<<iter.editauthor;
				 }
				 break;
			case 26: 
 				 if(isyinhao){ a<<jsonaddslash(iter.relatecontent); 
				 }else{
				 a<<iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
				 } 
			switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
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
 	 ktemp=iter.aid;
	 break;
case 1: 
 	 ktemp=iter.topicid;
	 break;
case 2: 
 	 ktemp=iter.classtype;
	 break;
case 3: 
 	 ktemp=iter.userid;
	 break;
case 4: 
 	 ktemp=iter.sortid;
	 break;
case 12: 
 	 ktemp=iter.addtime;
	 break;
case 13: 
 	 ktemp=iter.readnum;
	 break;
case 14: 
 	 ktemp=iter.review;
	 break;
case 18: 
 	 ktemp=iter.isopen;
	 break;
case 19: 
 	 ktemp=iter.ishome;
	 break;
case 20: 
 	 ktemp=iter.iscomment;
	 break;
case 21: 
 	 ktemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 				 a.emplace(iter.aid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.topicid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.classtype,iter);
				 break;
			case 3: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.sortid,iter);
				 break;
			case 12: 
 				 a.emplace(iter.addtime,iter);
				 break;
			case 13: 
 				 a.emplace(iter.readnum,iter);
				 break;
			case 14: 
 				 a.emplace(iter.review,iter);
				 break;
			case 18: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 19: 
 				 a.emplace(iter.ishome,iter);
				 break;
			case 20: 
 				 a.emplace(iter.iscomment,iter);
				 break;
			case 21: 
 				 a.emplace(iter.showtype,iter);
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
 				 a.emplace(iter.topicname,iter);
			 break;
			case 6: 
 				 a.emplace(iter.title,iter);
			 break;
			case 7: 
 				 a.emplace(iter.keywords,iter);
			 break;
			case 8: 
 				 a.emplace(iter.fromsource,iter);
			 break;
			case 9: 
 				 a.emplace(iter.author,iter);
			 break;
			case 10: 
 				 a.emplace(iter.addip,iter);
			 break;
			case 11: 
 				 a.emplace(iter.createtime,iter);
			 break;
			case 15: 
 				 a.emplace(iter.icoimg,iter);
			 break;
			case 16: 
 				 a.emplace(iter.content,iter);
			 break;
			case 17: 
 				 a.emplace(iter.mdcontent,iter);
			 break;
			case 22: 
 				 a.emplace(iter.fromlocal,iter);
			 break;
			case 23: 
 				 a.emplace(iter.texturl,iter);
			 break;
			case 24: 
 				 a.emplace(iter.summary,iter);
			 break;
			case 25: 
 				 a.emplace(iter.editauthor,iter);
			 break;
			case 26: 
 				 a.emplace(iter.relatecontent,iter);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
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
 				 ktemp=iter.aid;
			 break;
			case 1: 
 				 ktemp=iter.topicid;
			 break;
			case 2: 
 				 ktemp=iter.classtype;
			 break;
			case 3: 
 				 ktemp=iter.userid;
			 break;
			case 4: 
 				 ktemp=iter.sortid;
			 break;
			case 12: 
 				 ktemp=iter.addtime;
			 break;
			case 13: 
 				 ktemp=iter.readnum;
			 break;
			case 14: 
 				 ktemp=iter.review;
			 break;
			case 18: 
 				 ktemp=iter.isopen;
			 break;
			case 19: 
 				 ktemp=iter.ishome;
			 break;
			case 20: 
 				 ktemp=iter.iscomment;
			 break;
			case 21: 
 				 ktemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }
 			switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 	 ktemp=iter.topicname;
	 break;
case 6: 
 	 ktemp=iter.title;
	 break;
case 7: 
 	 ktemp=iter.keywords;
	 break;
case 8: 
 	 ktemp=iter.fromsource;
	 break;
case 9: 
 	 ktemp=iter.author;
	 break;
case 10: 
 	 ktemp=iter.addip;
	 break;
case 11: 
 	 ktemp=iter.createtime;
	 break;
case 15: 
 	 ktemp=iter.icoimg;
	 break;
case 16: 
 	 ktemp=iter.content;
	 break;
case 17: 
 	 ktemp=iter.mdcontent;
	 break;
case 22: 
 	 ktemp=iter.fromlocal;
	 break;
case 23: 
 	 ktemp=iter.texturl;
	 break;
case 24: 
 	 ktemp=iter.summary;
	 break;
case 25: 
 	 ktemp=iter.editauthor;
	 break;
case 26: 
 	 ktemp=iter.relatecontent;
	 break;
	  }
 switch(vpos){
case 5: 
 	 vtemp=iter.topicname;
	 break;
case 6: 
 	 vtemp=iter.title;
	 break;
case 7: 
 	 vtemp=iter.keywords;
	 break;
case 8: 
 	 vtemp=iter.fromsource;
	 break;
case 9: 
 	 vtemp=iter.author;
	 break;
case 10: 
 	 vtemp=iter.addip;
	 break;
case 11: 
 	 vtemp=iter.createtime;
	 break;
case 15: 
 	 vtemp=iter.icoimg;
	 break;
case 16: 
 	 vtemp=iter.content;
	 break;
case 17: 
 	 vtemp=iter.mdcontent;
	 break;
case 22: 
 	 vtemp=iter.fromlocal;
	 break;
case 23: 
 	 vtemp=iter.texturl;
	 break;
case 24: 
 	 vtemp=iter.summary;
	 break;
case 25: 
 	 vtemp=iter.editauthor;
	 break;
case 26: 
 	 vtemp=iter.relatecontent;
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
 	 a.emplace_back(iter.aid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.topicid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.classtype,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.sortid,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.addtime,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.readnum,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.review,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.ishome,iter);
	 break;
case 20: 
 	 a.emplace_back(iter.iscomment,iter);
	 break;
case 21: 
 	 a.emplace_back(iter.showtype,iter);
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
 	 a.emplace_back(iter.topicname,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.title,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.keywords,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.fromsource,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.author,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.addip,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.createtime,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.icoimg,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.content,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.mdcontent,iter);
	 break;
case 22: 
 	 a.emplace_back(iter.fromlocal,iter);
	 break;
case 23: 
 	 a.emplace_back(iter.texturl,iter);
	 break;
case 24: 
 	 a.emplace_back(iter.summary,iter);
	 break;
case 25: 
 	 a.emplace_back(iter.editauthor,iter);
	 break;
case 26: 
 	 a.emplace_back(iter.relatecontent,iter);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
				 break;
			 }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.aid;
				 break;
			case 1: 
 				 vtemp=iter.topicid;
				 break;
			case 2: 
 				 vtemp=iter.classtype;
				 break;
			case 3: 
 				 vtemp=iter.userid;
				 break;
			case 4: 
 				 vtemp=iter.sortid;
				 break;
			case 12: 
 				 vtemp=iter.addtime;
				 break;
			case 13: 
 				 vtemp=iter.readnum;
				 break;
			case 14: 
 				 vtemp=iter.review;
				 break;
			case 18: 
 				 vtemp=iter.isopen;
				 break;
			case 19: 
 				 vtemp=iter.ishome;
				 break;
			case 20: 
 				 vtemp=iter.iscomment;
				 break;
			case 21: 
 				 vtemp=iter.showtype;
				 break;
			 }

			switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 vtemp=iter.topicname;
				 break;
			case 6: 
 				 vtemp=iter.title;
				 break;
			case 7: 
 				 vtemp=iter.keywords;
				 break;
			case 8: 
 				 vtemp=iter.fromsource;
				 break;
			case 9: 
 				 vtemp=iter.author;
				 break;
			case 10: 
 				 vtemp=iter.addip;
				 break;
			case 11: 
 				 vtemp=iter.createtime;
				 break;
			case 15: 
 				 vtemp=iter.icoimg;
				 break;
			case 16: 
 				 vtemp=iter.content;
				 break;
			case 17: 
 				 vtemp=iter.mdcontent;
				 break;
			case 22: 
 				 vtemp=iter.fromlocal;
				 break;
			case 23: 
 				 vtemp=iter.texturl;
				 break;
			case 24: 
 				 vtemp=iter.summary;
				 break;
			case 25: 
 				 vtemp=iter.editauthor;
				 break;
			case 26: 
 				 vtemp=iter.relatecontent;
				 break;
			  }

			 switch(dpos){
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp][vtemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp][vtemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp][vtemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.showtype);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp].emplace_back(iter.topicname);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.title);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.keywords);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.fromsource);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.author);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.addip);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.createtime);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.icoimg);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.content);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.mdcontent);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.fromlocal);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.texturl);
				 break;
			case 24: 
 				 a[ktemp].emplace_back(iter.summary);
				 break;
			case 25: 
 				 a[ktemp].emplace_back(iter.editauthor);
				 break;
			case 26: 
 				 a[ktemp].emplace_back(iter.relatecontent);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.aid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.classtype);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.sortid);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.readnum);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.review);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.ishome);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.iscomment);
				 break;
			case 21: 
 				 a[ktemp].emplace_back(iter.showtype);
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
 				 a[iter.aid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.topicid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.classtype].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.sortid].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.addtime].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.readnum].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.review].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.ishome].emplace_back(iter);
				 break;
			case 20: 
 				 a[iter.iscomment].emplace_back(iter);
				 break;
			case 21: 
 				 a[iter.showtype].emplace_back(iter);
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
 				 a[iter.topicname].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.title].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.keywords].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.fromsource].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.author].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.addip].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.createtime].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.icoimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.mdcontent].emplace_back(iter);
				 break;
			case 22: 
 				 a[iter.fromlocal].emplace_back(iter);
				 break;
			case 23: 
 				 a[iter.texturl].emplace_back(iter);
				 break;
			case 24: 
 				 a[iter.summary].emplace_back(iter);
				 break;
			case 25: 
 				 a[iter.editauthor].emplace_back(iter);
				 break;
			case 26: 
 				 a[iter.relatecontent].emplace_back(iter);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
			 }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.topicname].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.keywords].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.fromsource].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.author].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.addip].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.createtime].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.icoimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.mdcontent].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.fromlocal].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.texturl].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.summary].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.editauthor].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.relatecontent].emplace_back(iter);
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
 				 ktemp=iter.topicname;
				 break;
			case 6: 
 				 ktemp=iter.title;
				 break;
			case 7: 
 				 ktemp=iter.keywords;
				 break;
			case 8: 
 				 ktemp=iter.fromsource;
				 break;
			case 9: 
 				 ktemp=iter.author;
				 break;
			case 10: 
 				 ktemp=iter.addip;
				 break;
			case 11: 
 				 ktemp=iter.createtime;
				 break;
			case 15: 
 				 ktemp=iter.icoimg;
				 break;
			case 16: 
 				 ktemp=iter.content;
				 break;
			case 17: 
 				 ktemp=iter.mdcontent;
				 break;
			case 22: 
 				 ktemp=iter.fromlocal;
				 break;
			case 23: 
 				 ktemp=iter.texturl;
				 break;
			case 24: 
 				 ktemp=iter.summary;
				 break;
			case 25: 
 				 ktemp=iter.editauthor;
				 break;
			case 26: 
 				 ktemp=iter.relatecontent;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.aid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.classtype].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.readnum].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.review].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.ishome].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.iscomment].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.showtype].emplace_back(iter);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.aid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.classtype].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.sortid].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.readnum].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.review].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.ishome].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.iscomment].emplace_back(iter);
				 break;
			case 21: 
 				 a[ktemp][iter.showtype].emplace_back(iter);
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
 				 ktemp=iter.aid;
				 break;
			case 1: 
 				 ktemp=iter.topicid;
				 break;
			case 2: 
 				 ktemp=iter.classtype;
				 break;
			case 3: 
 				 ktemp=iter.userid;
				 break;
			case 4: 
 				 ktemp=iter.sortid;
				 break;
			case 12: 
 				 ktemp=iter.addtime;
				 break;
			case 13: 
 				 ktemp=iter.readnum;
				 break;
			case 14: 
 				 ktemp=iter.review;
				 break;
			case 18: 
 				 ktemp=iter.isopen;
				 break;
			case 19: 
 				 ktemp=iter.ishome;
				 break;
			case 20: 
 				 ktemp=iter.iscomment;
				 break;
			case 21: 
 				 ktemp=iter.showtype;
				 break;
			  }

			 switch(vpos){
			case 5: 
 				 a[ktemp][iter.topicname].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.title].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.keywords].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.fromsource].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.author].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.addip].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.createtime].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.icoimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.content].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.mdcontent].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.fromlocal].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.texturl].emplace_back(iter);
				 break;
			case 24: 
 				 a[ktemp][iter.summary].emplace_back(iter);
				 break;
			case 25: 
 				 a[ktemp][iter.editauthor].emplace_back(iter);
				 break;
			case 26: 
 				 a[ktemp][iter.relatecontent].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   