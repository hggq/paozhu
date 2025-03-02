#ifndef ORM_CMS_PRODUCTBASEMATA_H
#define ORM_CMS_PRODUCTBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Sat, 01 Mar 2025 06:42:20 GMT
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
namespace orm { 
   
     namespace cms { 

struct productbase
{
    struct meta{
     unsigned  int  pid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  topicid = 0; ///*栏目id*/
 unsigned  int  bigid = 0; ///*大类*/
 unsigned  int  smallid = 0; ///*小类*/
 unsigned  int  brandid = 0; ///*品牌id*/
 char  isview = 0; ///*是否显示*/
 char  isstore = 0; ///*精品*/
 unsigned  char  ishome = 0; ///*是否首页*/
 char  showtype = 0; ///*page layout*/
 std::string  sntype = ""; ///*型号*/
 std::string  name = ""; ///*产品名称*/
 std::string  keywords = ""; ///**/
 std::string  introduce = ""; ///*介绍*/
 std::string  listimg = ""; ///*列表图片*/
 std::string  bigimg = ""; ///*主图*/
 std::string  maincontent = ""; ///*产品主内容*/
 std::string  paracontent = ""; ///*参数*/
 std::string  samepro = ""; ///*类似*/
 std::string  attatchfiles = ""; ///*附件*/
 unsigned  int  price = 0; ///*价格*/
 unsigned  int  sortid = 0; ///**/
 std::string  adddate = ""; ///*产品添加日期*/
 std::string  editdate = ""; ///*产品修改时间*/
 } data;
  
        struct meta_tree{
         unsigned  int  pid = 0; ///**/
 unsigned  int  userid = 0; ///**/
 unsigned  int  topicid = 0; ///*栏目id*/
 unsigned  int  bigid = 0; ///*大类*/
 unsigned  int  smallid = 0; ///*小类*/
 unsigned  int  brandid = 0; ///*品牌id*/
 char  isview = 0; ///*是否显示*/
 char  isstore = 0; ///*精品*/
 unsigned  char  ishome = 0; ///*是否首页*/
 char  showtype = 0; ///*page layout*/
 std::string  sntype = ""; ///*型号*/
 std::string  name = ""; ///*产品名称*/
 std::string  keywords = ""; ///**/
 std::string  introduce = ""; ///*介绍*/
 std::string  listimg = ""; ///*列表图片*/
 std::string  bigimg = ""; ///*主图*/
 std::string  maincontent = ""; ///*产品主内容*/
 std::string  paracontent = ""; ///*参数*/
 std::string  samepro = ""; ///*类似*/
 std::string  attatchfiles = ""; ///*附件*/
 unsigned  int  price = 0; ///*价格*/
 unsigned  int  sortid = 0; ///**/
 std::string  adddate = ""; ///*产品添加日期*/
 std::string  editdate = ""; ///*产品修改时间*/

	std::vector<meta_tree> children;
 };
 std::vector<productbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
unsigned int _offset=0;
std::vector<productbase::meta>::iterator begin(){     return record.begin(); }
std::vector<productbase::meta>::iterator end(){     return record.end(); }
std::vector<productbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<productbase::meta>::const_iterator end() const{     return record.end(); }
static constexpr std::array<std::string,24> col_names={"pid","userid","topicid","bigid","smallid","brandid","isview","isstore","ishome","showtype","sntype","name","keywords","introduce","listimg","bigimg","maincontent","paracontent","samepro","attatchfiles","price","sortid","adddate","editdate"};
static constexpr std::array<unsigned char,24> col_types= {3,3,3,3,3,3,1,1,1,1,253,253,253,252,252,253,252,252,253,252,3,3,253,253};
static constexpr std::array<unsigned char,24> col_length= {0,0,0,0,0,0,0,0,0,0,128,253,0,0,0,254,0,0,254,0,0,0,20,20};
static constexpr std::array<unsigned char,24> col_decimals= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
std::string tablename="product";
static constexpr std::string modelname="Product";

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
   	 return 22;
break;
case 12:
   	 return 19;
break;
 }
 break;
case 'b':
 switch(coln.size()){  
case 5:
   	 return 3;
break;
case 6:
   	 return 15;
break;
case 7:
   	 return 5;
break;
 }
 break;
case 'e':
   	 return 23;
break;
case 'i':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='e'){ return 8; }
 if(colpospppc=='w'){ return 6; }
   	 break;
case 7:
   	 return 7;
break;
case 9:
   	 return 13;
break;
 }
 break;
case 'k':
   	 return 12;
break;
case 'l':
   	 return 14;
break;
case 'm':
   	 return 16;
break;
case 'n':
   	 return 11;
break;
case 'p':
 switch(coln.size()){  
case 3:
   	 return 0;
break;
case 5:
   	 return 20;
break;
case 11:
   	 return 17;
break;
 }
 break;
case 's':
 switch(coln.size()){  
case 6:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 21; }
 if(colpospppc=='e'){ return 10; }
   	 break;
case 7:
  colpospppc=coln.back();
    if(colpospppc<91){ colpospppc+=32; }
 if(colpospppc=='d'){ return 4; }
 if(colpospppc=='o'){ return 18; }
   	 break;
case 8:
   	 return 9;
break;
 }
 break;
case 't':
   	 return 2;
break;
case 'u':
   	 return 1;
break;

             }
             return 255;
           }
         
    int size(){ return record.size(); }   

    std::string getPKname(){ 
       return "pid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     productbase::meta metatemp;    
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

        if(data.pid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.pid);
}
if(data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.topicid);
}
if(data.bigid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.showtype);
}
tempsql<<",'"<<stringaddslash(data.sntype)<<"'";
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(data.listimg)<<"'";
tempsql<<",'"<<stringaddslash(data.bigimg)<<"'";
tempsql<<",'"<<stringaddslash(data.maincontent)<<"'";
tempsql<<",'"<<stringaddslash(data.paracontent)<<"'";
tempsql<<",'"<<stringaddslash(data.samepro)<<"'";
tempsql<<",'"<<stringaddslash(data.attatchfiles)<<"'";
if(data.price==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.sortid);
}
tempsql<<",'"<<stringaddslash(data.adddate)<<"'";
tempsql<<",'"<<stringaddslash(data.editdate)<<"'";
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

        if(insert_data.pid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.pid);
}
if(insert_data.userid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.userid);
}
if(insert_data.topicid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.topicid);
}
if(insert_data.bigid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.bigid);
}
if(insert_data.smallid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.smallid);
}
if(insert_data.brandid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.brandid);
}
if(insert_data.isview==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isview);
}
if(insert_data.isstore==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isstore);
}
if(insert_data.ishome==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.ishome);
}
if(insert_data.showtype==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.showtype);
}
tempsql<<",'"<<stringaddslash(insert_data.sntype)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.keywords)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.introduce)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.listimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.bigimg)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.maincontent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.paracontent)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.samepro)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.attatchfiles)<<"'";
if(insert_data.price==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.price);
}
if(insert_data.sortid==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.sortid);
}
tempsql<<",'"<<stringaddslash(insert_data.adddate)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.editdate)<<"'";
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


            	if(insert_data[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].pid);
	}
	if(insert_data[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].userid);
	}
	if(insert_data[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].topicid);
	}
	if(insert_data[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].bigid);
	}
	if(insert_data[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].smallid);
	}
	if(insert_data[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].brandid);
	}
	if(insert_data[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isview);
	}
	if(insert_data[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isstore);
	}
	if(insert_data[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].ishome);
	}
	if(insert_data[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].showtype);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].sntype)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].keywords)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].introduce)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].listimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].bigimg)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].maincontent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].paracontent)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].samepro)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].attatchfiles)<<"'";
	if(insert_data[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].price);
	}
	if(insert_data[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].sortid);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].adddate)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].editdate)<<"'";
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

        if(data.pid==0){
	tempsql<<"`pid`=0";
 }else{ 
	tempsql<<"`pid`="<<std::to_string(data.pid);
}
if(data.userid==0){
	tempsql<<",`userid`=0";
 }else{ 
	tempsql<<",`userid`="<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",`topicid`=0";
 }else{ 
	tempsql<<",`topicid`="<<std::to_string(data.topicid);
}
if(data.bigid==0){
	tempsql<<",`bigid`=0";
 }else{ 
	tempsql<<",`bigid`="<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",`smallid`=0";
 }else{ 
	tempsql<<",`smallid`="<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",`brandid`=0";
 }else{ 
	tempsql<<",`brandid`="<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",`isview`=0";
 }else{ 
	tempsql<<",`isview`="<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",`isstore`=0";
 }else{ 
	tempsql<<",`isstore`="<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",`ishome`=0";
 }else{ 
	tempsql<<",`ishome`="<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",`showtype`=0";
 }else{ 
	tempsql<<",`showtype`="<<std::to_string(data.showtype);
}
tempsql<<",`sntype`='"<<stringaddslash(data.sntype)<<"'";
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`keywords`='"<<stringaddslash(data.keywords)<<"'";
tempsql<<",`introduce`='"<<stringaddslash(data.introduce)<<"'";
tempsql<<",`listimg`='"<<stringaddslash(data.listimg)<<"'";
tempsql<<",`bigimg`='"<<stringaddslash(data.bigimg)<<"'";
tempsql<<",`maincontent`='"<<stringaddslash(data.maincontent)<<"'";
tempsql<<",`paracontent`='"<<stringaddslash(data.paracontent)<<"'";
tempsql<<",`samepro`='"<<stringaddslash(data.samepro)<<"'";
tempsql<<",`attatchfiles`='"<<stringaddslash(data.attatchfiles)<<"'";
if(data.price==0){
	tempsql<<",`price`=0";
 }else{ 
	tempsql<<",`price`="<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",`sortid`=0";
 }else{ 
	tempsql<<",`sortid`="<<std::to_string(data.sortid);
}
tempsql<<",`adddate`='"<<stringaddslash(data.adddate)<<"'";
tempsql<<",`editdate`='"<<stringaddslash(data.editdate)<<"'";
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
if(data.pid==0){
	tempsql<<"`pid`=0";
 }else{ 
	tempsql<<"`pid`="<<std::to_string(data.pid);
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
if(data.topicid==0){
	tempsql<<"`topicid`=0";
 }else{ 
	tempsql<<"`topicid`="<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.bigid==0){
	tempsql<<"`bigid`=0";
 }else{ 
	tempsql<<"`bigid`="<<std::to_string(data.bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.smallid==0){
	tempsql<<"`smallid`=0";
 }else{ 
	tempsql<<"`smallid`="<<std::to_string(data.smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.brandid==0){
	tempsql<<"`brandid`=0";
 }else{ 
	tempsql<<"`brandid`="<<std::to_string(data.brandid);
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
if(data.isstore==0){
	tempsql<<"`isstore`=0";
 }else{ 
	tempsql<<"`isstore`="<<std::to_string(data.isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"`ishome`=0";
 }else{ 
	tempsql<<"`ishome`="<<std::to_string(data.ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"`showtype`=0";
 }else{ 
	tempsql<<"`showtype`="<<std::to_string(data.showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`sntype`='"<<stringaddslash(data.sntype)<<"'";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`keywords`='"<<stringaddslash(data.keywords)<<"'";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`introduce`='"<<stringaddslash(data.introduce)<<"'";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`listimg`='"<<stringaddslash(data.listimg)<<"'";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`bigimg`='"<<stringaddslash(data.bigimg)<<"'";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`maincontent`='"<<stringaddslash(data.maincontent)<<"'";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`paracontent`='"<<stringaddslash(data.paracontent)<<"'";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`samepro`='"<<stringaddslash(data.samepro)<<"'";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`attatchfiles`='"<<stringaddslash(data.attatchfiles)<<"'";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.price==0){
	tempsql<<"`price`=0";
 }else{ 
	tempsql<<"`price`="<<std::to_string(data.price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"`sortid`=0";
 }else{ 
	tempsql<<"`sortid`="<<std::to_string(data.sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`adddate`='"<<stringaddslash(data.adddate)<<"'";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`editdate`='"<<stringaddslash(data.editdate)<<"'";
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
            	if(record[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].pid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bigid);
	}
	if(record[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].smallid);
	}
	if(record[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].brandid);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isstore);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sntype)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].listimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bigimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].maincontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].paracontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].samepro)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].attatchfiles)<<"'";
	if(record[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].price);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].adddate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editdate)<<"'";
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
            	if(record[i].pid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].pid);
	}
	if(record[i].userid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].userid);
	}
	if(record[i].topicid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].topicid);
	}
	if(record[i].bigid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bigid);
	}
	if(record[i].smallid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].smallid);
	}
	if(record[i].brandid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].brandid);
	}
	if(record[i].isview==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isview);
	}
	if(record[i].isstore==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isstore);
	}
	if(record[i].ishome==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].ishome);
	}
	if(record[i].showtype==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].showtype);
	}
	tempsql<<",'"<<stringaddslash(record[i].sntype)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].keywords)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].introduce)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].listimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].bigimg)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].maincontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].paracontent)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].samepro)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].attatchfiles)<<"'";
	if(record[i].price==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].price);
	}
	if(record[i].sortid==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].sortid);
	}
	tempsql<<",'"<<stringaddslash(record[i].adddate)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].editdate)<<"'";
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
if(data.pid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.pid));
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
if(data.topicid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.topicid));
}
 break;
 case 3:
if(data.bigid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bigid));
}
 break;
 case 4:
if(data.smallid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.smallid));
}
 break;
 case 5:
if(data.brandid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.brandid));
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
if(data.isstore==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isstore));
}
 break;
 case 8:
if(data.ishome==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.ishome));
}
 break;
 case 9:
if(data.showtype==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.showtype));
}
 break;
 case 10:
	temparray.push_back(data.sntype);
 break;
 case 11:
	temparray.push_back(data.name);
 break;
 case 12:
	temparray.push_back(data.keywords);
 break;
 case 13:
	temparray.push_back(data.introduce);
 break;
 case 14:
	temparray.push_back(data.listimg);
 break;
 case 15:
	temparray.push_back(data.bigimg);
 break;
 case 16:
	temparray.push_back(data.maincontent);
 break;
 case 17:
	temparray.push_back(data.paracontent);
 break;
 case 18:
	temparray.push_back(data.samepro);
 break;
 case 19:
	temparray.push_back(data.attatchfiles);
 break;
 case 20:
if(data.price==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.price));
}
 break;
 case 21:
if(data.sortid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.sortid));
}
 break;
 case 22:
	temparray.push_back(data.adddate);
 break;
 case 23:
	temparray.push_back(data.editdate);
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
if(data.pid==0){
	tempsql.insert({"pid","0"});
 }else{ 
	tempsql.insert({"pid",std::to_string(data.pid)});
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
if(data.topicid==0){
	tempsql.insert({"topicid","0"});
 }else{ 
	tempsql.insert({"topicid",std::to_string(data.topicid)});
}
 break;
 case 3:
if(data.bigid==0){
	tempsql.insert({"bigid","0"});
 }else{ 
	tempsql.insert({"bigid",std::to_string(data.bigid)});
}
 break;
 case 4:
if(data.smallid==0){
	tempsql.insert({"smallid","0"});
 }else{ 
	tempsql.insert({"smallid",std::to_string(data.smallid)});
}
 break;
 case 5:
if(data.brandid==0){
	tempsql.insert({"brandid","0"});
 }else{ 
	tempsql.insert({"brandid",std::to_string(data.brandid)});
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
if(data.isstore==0){
	tempsql.insert({"isstore","0"});
 }else{ 
	tempsql.insert({"isstore",std::to_string(data.isstore)});
}
 break;
 case 8:
if(data.ishome==0){
	tempsql.insert({"ishome","0"});
 }else{ 
	tempsql.insert({"ishome",std::to_string(data.ishome)});
}
 break;
 case 9:
if(data.showtype==0){
	tempsql.insert({"showtype","0"});
 }else{ 
	tempsql.insert({"showtype",std::to_string(data.showtype)});
}
 break;
 case 10:
	tempsql.insert({"sntype",data.sntype});
 break;
 case 11:
	tempsql.insert({"name",data.name});
 break;
 case 12:
	tempsql.insert({"keywords",data.keywords});
 break;
 case 13:
	tempsql.insert({"introduce",data.introduce});
 break;
 case 14:
	tempsql.insert({"listimg",data.listimg});
 break;
 case 15:
	tempsql.insert({"bigimg",data.bigimg});
 break;
 case 16:
	tempsql.insert({"maincontent",data.maincontent});
 break;
 case 17:
	tempsql.insert({"paracontent",data.paracontent});
 break;
 case 18:
	tempsql.insert({"samepro",data.samepro});
 break;
 case 19:
	tempsql.insert({"attatchfiles",data.attatchfiles});
 break;
 case 20:
if(data.price==0){
	tempsql.insert({"price","0"});
 }else{ 
	tempsql.insert({"price",std::to_string(data.price)});
}
 break;
 case 21:
if(data.sortid==0){
	tempsql.insert({"sortid","0"});
 }else{ 
	tempsql.insert({"sortid",std::to_string(data.sortid)});
}
 break;
 case 22:
	tempsql.insert({"adddate",data.adddate});
 break;
 case 23:
	tempsql.insert({"editdate",data.editdate});
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
if(data.pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(data.pid);
}
if(data.userid==0){
	tempsql<<",\"userid\":0";
 }else{ 
	tempsql<<",\"userid\":"<<std::to_string(data.userid);
}
if(data.topicid==0){
	tempsql<<",\"topicid\":0";
 }else{ 
	tempsql<<",\"topicid\":"<<std::to_string(data.topicid);
}
if(data.bigid==0){
	tempsql<<",\"bigid\":0";
 }else{ 
	tempsql<<",\"bigid\":"<<std::to_string(data.bigid);
}
if(data.smallid==0){
	tempsql<<",\"smallid\":0";
 }else{ 
	tempsql<<",\"smallid\":"<<std::to_string(data.smallid);
}
if(data.brandid==0){
	tempsql<<",\"brandid\":0";
 }else{ 
	tempsql<<",\"brandid\":"<<std::to_string(data.brandid);
}
if(data.isview==0){
	tempsql<<",\"isview\":0";
 }else{ 
	tempsql<<",\"isview\":"<<std::to_string(data.isview);
}
if(data.isstore==0){
	tempsql<<",\"isstore\":0";
 }else{ 
	tempsql<<",\"isstore\":"<<std::to_string(data.isstore);
}
if(data.ishome==0){
	tempsql<<",\"ishome\":0";
 }else{ 
	tempsql<<",\"ishome\":"<<std::to_string(data.ishome);
}
if(data.showtype==0){
	tempsql<<",\"showtype\":0";
 }else{ 
	tempsql<<",\"showtype\":"<<std::to_string(data.showtype);
}
tempsql<<",\"sntype\":\""<<http::utf8_to_jsonstring(data.sntype);
tempsql<<"\"";
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords);
tempsql<<"\"";
tempsql<<",\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce);
tempsql<<"\"";
tempsql<<",\"listimg\":\""<<http::utf8_to_jsonstring(data.listimg);
tempsql<<"\"";
tempsql<<",\"bigimg\":\""<<http::utf8_to_jsonstring(data.bigimg);
tempsql<<"\"";
tempsql<<",\"maincontent\":\""<<http::utf8_to_jsonstring(data.maincontent);
tempsql<<"\"";
tempsql<<",\"paracontent\":\""<<http::utf8_to_jsonstring(data.paracontent);
tempsql<<"\"";
tempsql<<",\"samepro\":\""<<http::utf8_to_jsonstring(data.samepro);
tempsql<<"\"";
tempsql<<",\"attatchfiles\":\""<<http::utf8_to_jsonstring(data.attatchfiles);
tempsql<<"\"";
if(data.price==0){
	tempsql<<",\"price\":0";
 }else{ 
	tempsql<<",\"price\":"<<std::to_string(data.price);
}
if(data.sortid==0){
	tempsql<<",\"sortid\":0";
 }else{ 
	tempsql<<",\"sortid\":"<<std::to_string(data.sortid);
}
tempsql<<",\"adddate\":\""<<http::utf8_to_jsonstring(data.adddate);
tempsql<<"\"";
tempsql<<",\"editdate\":\""<<http::utf8_to_jsonstring(data.editdate);
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
if(data.pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(data.pid);
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
if(data.topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(data.topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(data.bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(data.bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(data.smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(data.smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(data.brandid);
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
if(data.isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(data.isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(data.ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(data.showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(data.showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(data.sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(data.keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(data.introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(data.listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(data.bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(data.maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(data.paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(data.samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(data.attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(data.price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(data.price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(data.sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(data.sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(data.adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(data.editdate)<<"\"";
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
        productbase::meta metatemp; 
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
			data.pid=std::stoul(set_value_name);
		}catch (...) { 
			data.pid=0;
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
			data.topicid=std::stoul(set_value_name);
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.bigid=std::stoul(set_value_name);
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=std::stoul(set_value_name);
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=std::stoul(set_value_name);
		}catch (...) { 
			data.brandid=0;
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
			data.isstore=std::stoi(set_value_name);
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=std::stoi(set_value_name);
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=std::stoi(set_value_name);
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype.append(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords.append(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce.append(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg.append(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg.append(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent.append(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent.append(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro.append(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles.append(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=std::stoul(set_value_name);
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=std::stoul(set_value_name);
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate.append(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate.append(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
			data.pid=set_value_name;
		}catch (...) { 
			data.pid=0;
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
			data.topicid=set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.bigid=set_value_name;
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=set_value_name;
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=set_value_name;
		}catch (...) { 
			data.brandid=0;
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
			data.isstore=set_value_name;
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype=std::to_string(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg=std::to_string(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg=std::to_string(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent=std::to_string(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent=std::to_string(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro=std::to_string(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=set_value_name;
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate=std::to_string(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate=std::to_string(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
			data.pid=(unsigned int)set_value_name;
		}catch (...) { 
			data.pid=0;
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
			data.topicid=(unsigned int)set_value_name;
		}catch (...) { 
			data.topicid=0;
			 }
			break;
		case 3:
		 try{
			data.bigid=(unsigned int)set_value_name;
		}catch (...) { 
			data.bigid=0;
			 }
			break;
		case 4:
		 try{
			data.smallid=(unsigned int)set_value_name;
		}catch (...) { 
			data.smallid=0;
			 }
			break;
		case 5:
		 try{
			data.brandid=(unsigned int)set_value_name;
		}catch (...) { 
			data.brandid=0;
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
			data.isstore=(int)set_value_name;
		}catch (...) { 
			data.isstore=0;
			 }
			break;
		case 8:
		 try{
			data.ishome=(int)set_value_name;
		}catch (...) { 
			data.ishome=0;
			 }
			break;
		case 9:
		 try{
			data.showtype=(int)set_value_name;
		}catch (...) { 
			data.showtype=0;
			 }
			break;
		case 10:
		 try{
			data.sntype=std::to_string(set_value_name);
		}catch (...) { 
			data.sntype.clear();
			 }
			break;
		case 11:
		 try{
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 12:
		 try{
			data.keywords=std::to_string(set_value_name);
		}catch (...) { 
			data.keywords.clear();
			 }
			break;
		case 13:
		 try{
			data.introduce=std::to_string(set_value_name);
		}catch (...) { 
			data.introduce.clear();
			 }
			break;
		case 14:
		 try{
			data.listimg=std::to_string(set_value_name);
		}catch (...) { 
			data.listimg.clear();
			 }
			break;
		case 15:
		 try{
			data.bigimg=std::to_string(set_value_name);
		}catch (...) { 
			data.bigimg.clear();
			 }
			break;
		case 16:
		 try{
			data.maincontent=std::to_string(set_value_name);
		}catch (...) { 
			data.maincontent.clear();
			 }
			break;
		case 17:
		 try{
			data.paracontent=std::to_string(set_value_name);
		}catch (...) { 
			data.paracontent.clear();
			 }
			break;
		case 18:
		 try{
			data.samepro=std::to_string(set_value_name);
		}catch (...) { 
			data.samepro.clear();
			 }
			break;
		case 19:
		 try{
			data.attatchfiles=std::to_string(set_value_name);
		}catch (...) { 
			data.attatchfiles.clear();
			 }
			break;
		case 20:
		 try{
			data.price=(unsigned int)set_value_name;
		}catch (...) { 
			data.price=0;
			 }
			break;
		case 21:
		 try{
			data.sortid=(unsigned int)set_value_name;
		}catch (...) { 
			data.sortid=0;
			 }
			break;
		case 22:
		 try{
			data.adddate=std::to_string(set_value_name);
		}catch (...) { 
			data.adddate.clear();
			 }
			break;
		case 23:
		 try{
			data.editdate=std::to_string(set_value_name);
		}catch (...) { 
			data.editdate.clear();
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
if(record[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(record[n].pid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(record[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(record[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(record[n].brandid);
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
if(record[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(record[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(record[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(record[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(record[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(record[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(record[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(record[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(record[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(record[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(record[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(record[n].editdate)<<"\"";
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
if(record[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(record[n].pid);
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
if(record[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(record[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(record[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(record[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(record[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(record[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(record[n].brandid);
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
if(record[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(record[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(record[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(record[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(record[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(record[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(record[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(record[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(record[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(record[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(record[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(record[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(record[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(record[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(record[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(record[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(record[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(record[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(record[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(record[n].editdate)<<"\"";
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
   long long getPK(){  return data.pid; } 
 void setPK(long long val){  data.pid=val;} 
 unsigned  int  getPid(){  return data.pid; } 
 void setPid( unsigned  int  val){  data.pid=val;} 

 unsigned  int  getUserid(){  return data.userid; } 
 void setUserid( unsigned  int  val){  data.userid=val;} 

 unsigned  int  getTopicid(){  return data.topicid; } 
 void setTopicid( unsigned  int  val){  data.topicid=val;} 

 unsigned  int  getBigid(){  return data.bigid; } 
 void setBigid( unsigned  int  val){  data.bigid=val;} 

 unsigned  int  getSmallid(){  return data.smallid; } 
 void setSmallid( unsigned  int  val){  data.smallid=val;} 

 unsigned  int  getBrandid(){  return data.brandid; } 
 void setBrandid( unsigned  int  val){  data.brandid=val;} 

 char  getIsview(){  return data.isview; } 
 void setIsview( char  val){  data.isview=val;} 

 char  getIsstore(){  return data.isstore; } 
 void setIsstore( char  val){  data.isstore=val;} 

 unsigned  char  getIshome(){  return data.ishome; } 
 void setIshome( unsigned  char  val){  data.ishome=val;} 

 char  getShowtype(){  return data.showtype; } 
 void setShowtype( char  val){  data.showtype=val;} 

 std::string  getSntype(){  return data.sntype; } 
 std::string & getRefSntype(){  return std::ref(data.sntype); } 
 void setSntype( std::string  &val){  data.sntype=val;} 
 void setSntype(std::string_view val){  data.sntype=val;} 

 std::string  getName(){  return data.name; } 
 std::string & getRefName(){  return std::ref(data.name); } 
 void setName( std::string  &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

 std::string  getKeywords(){  return data.keywords; } 
 std::string & getRefKeywords(){  return std::ref(data.keywords); } 
 void setKeywords( std::string  &val){  data.keywords=val;} 
 void setKeywords(std::string_view val){  data.keywords=val;} 

 std::string  getIntroduce(){  return data.introduce; } 
 std::string & getRefIntroduce(){  return std::ref(data.introduce); } 
 void setIntroduce( std::string  &val){  data.introduce=val;} 
 void setIntroduce(std::string_view val){  data.introduce=val;} 

 std::string  getListimg(){  return data.listimg; } 
 std::string & getRefListimg(){  return std::ref(data.listimg); } 
 void setListimg( std::string  &val){  data.listimg=val;} 
 void setListimg(std::string_view val){  data.listimg=val;} 

 std::string  getBigimg(){  return data.bigimg; } 
 std::string & getRefBigimg(){  return std::ref(data.bigimg); } 
 void setBigimg( std::string  &val){  data.bigimg=val;} 
 void setBigimg(std::string_view val){  data.bigimg=val;} 

 std::string  getMaincontent(){  return data.maincontent; } 
 std::string & getRefMaincontent(){  return std::ref(data.maincontent); } 
 void setMaincontent( std::string  &val){  data.maincontent=val;} 
 void setMaincontent(std::string_view val){  data.maincontent=val;} 

 std::string  getParacontent(){  return data.paracontent; } 
 std::string & getRefParacontent(){  return std::ref(data.paracontent); } 
 void setParacontent( std::string  &val){  data.paracontent=val;} 
 void setParacontent(std::string_view val){  data.paracontent=val;} 

 std::string  getSamepro(){  return data.samepro; } 
 std::string & getRefSamepro(){  return std::ref(data.samepro); } 
 void setSamepro( std::string  &val){  data.samepro=val;} 
 void setSamepro(std::string_view val){  data.samepro=val;} 

 std::string  getAttatchfiles(){  return data.attatchfiles; } 
 std::string & getRefAttatchfiles(){  return std::ref(data.attatchfiles); } 
 void setAttatchfiles( std::string  &val){  data.attatchfiles=val;} 
 void setAttatchfiles(std::string_view val){  data.attatchfiles=val;} 

 unsigned  int  getPrice(){  return data.price; } 
 void setPrice( unsigned  int  val){  data.price=val;} 

 unsigned  int  getSortid(){  return data.sortid; } 
 void setSortid( unsigned  int  val){  data.sortid=val;} 

 std::string  getAdddate(){  return data.adddate; } 
 std::string & getRefAdddate(){  return std::ref(data.adddate); } 
 void setAdddate( std::string  &val){  data.adddate=val;} 
 void setAdddate(std::string_view val){  data.adddate=val;} 

 std::string  getEditdate(){  return data.editdate; } 
 std::string & getRefEditdate(){  return std::ref(data.editdate); } 
 void setEditdate( std::string  &val){  data.editdate=val;} 
 void setEditdate(std::string_view val){  data.editdate=val;} 

productbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
productbase::meta getData(){
 	 return data; 
} 
std::vector<productbase::meta> getRecord(){
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
if(tree_data[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(tree_data[n].pid);
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(tree_data[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(tree_data[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(tree_data[n].brandid);
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
if(tree_data[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(tree_data[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(tree_data[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(tree_data[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(tree_data[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(tree_data[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(tree_data[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(tree_data[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(tree_data[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(tree_data[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(tree_data[n].editdate)<<"\"";
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
if(tree_data[n].pid==0){
	tempsql<<"\"pid\":0";
 }else{ 
	tempsql<<"\"pid\":"<<std::to_string(tree_data[n].pid);
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
if(tree_data[n].topicid==0){
	tempsql<<"\"topicid\":0";
 }else{ 
	tempsql<<"\"topicid\":"<<std::to_string(tree_data[n].topicid);
}
 break;
 case 3:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bigid==0){
	tempsql<<"\"bigid\":0";
 }else{ 
	tempsql<<"\"bigid\":"<<std::to_string(tree_data[n].bigid);
}
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].smallid==0){
	tempsql<<"\"smallid\":0";
 }else{ 
	tempsql<<"\"smallid\":"<<std::to_string(tree_data[n].smallid);
}
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].brandid==0){
	tempsql<<"\"brandid\":0";
 }else{ 
	tempsql<<"\"brandid\":"<<std::to_string(tree_data[n].brandid);
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
if(tree_data[n].isstore==0){
	tempsql<<"\"isstore\":0";
 }else{ 
	tempsql<<"\"isstore\":"<<std::to_string(tree_data[n].isstore);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].ishome==0){
	tempsql<<"\"ishome\":0";
 }else{ 
	tempsql<<"\"ishome\":"<<std::to_string(tree_data[n].ishome);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].showtype==0){
	tempsql<<"\"showtype\":0";
 }else{ 
	tempsql<<"\"showtype\":"<<std::to_string(tree_data[n].showtype);
}
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"sntype\":\""<<http::utf8_to_jsonstring(tree_data[n].sntype)<<"\"";
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"keywords\":\""<<http::utf8_to_jsonstring(tree_data[n].keywords)<<"\"";
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"introduce\":\""<<http::utf8_to_jsonstring(tree_data[n].introduce)<<"\"";
 break;
 case 14:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"listimg\":\""<<http::utf8_to_jsonstring(tree_data[n].listimg)<<"\"";
 break;
 case 15:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"bigimg\":\""<<http::utf8_to_jsonstring(tree_data[n].bigimg)<<"\"";
 break;
 case 16:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"maincontent\":\""<<http::utf8_to_jsonstring(tree_data[n].maincontent)<<"\"";
 break;
 case 17:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"paracontent\":\""<<http::utf8_to_jsonstring(tree_data[n].paracontent)<<"\"";
 break;
 case 18:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"samepro\":\""<<http::utf8_to_jsonstring(tree_data[n].samepro)<<"\"";
 break;
 case 19:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"attatchfiles\":\""<<http::utf8_to_jsonstring(tree_data[n].attatchfiles)<<"\"";
 break;
 case 20:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].price==0){
	tempsql<<"\"price\":0";
 }else{ 
	tempsql<<"\"price\":"<<std::to_string(tree_data[n].price);
}
 break;
 case 21:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].sortid==0){
	tempsql<<"\"sortid\":0";
 }else{ 
	tempsql<<"\"sortid\":"<<std::to_string(tree_data[n].sortid);
}
 break;
 case 22:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"adddate\":\""<<http::utf8_to_jsonstring(tree_data[n].adddate)<<"\"";
 break;
 case 23:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"editdate\":\""<<http::utf8_to_jsonstring(tree_data[n].editdate)<<"\"";
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
   		 if(key_name=="sntype")
		{
			return data.sntype;
		}
		 if(key_name=="name")
		{
			return data.name;
		}
		 if(key_name=="keywords")
		{
			return data.keywords;
		}
		 if(key_name=="introduce")
		{
			return data.introduce;
		}
		 if(key_name=="listimg")
		{
			return data.listimg;
		}
		 if(key_name=="bigimg")
		{
			return data.bigimg;
		}
		 if(key_name=="maincontent")
		{
			return data.maincontent;
		}
		 if(key_name=="paracontent")
		{
			return data.paracontent;
		}
		 if(key_name=="samepro")
		{
			return data.samepro;
		}
		 if(key_name=="attatchfiles")
		{
			return data.attatchfiles;
		}
		 if(key_name=="adddate")
		{
			return data.adddate;
		}
		 if(key_name=="editdate")
		{
			return data.editdate;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="pid")
		{
			return data.pid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="topicid")
		{
			return data.topicid;
		}
		 if(key_name=="bigid")
		{
			return data.bigid;
		}
		 if(key_name=="smallid")
		{
			return data.smallid;
		}
		 if(key_name=="brandid")
		{
			return data.brandid;
		}
		 if(key_name=="isview")
		{
			return data.isview;
		}
		 if(key_name=="isstore")
		{
			return data.isstore;
		}
		 if(key_name=="ishome")
		{
			return data.ishome;
		}
		 if(key_name=="showtype")
		{
			return data.showtype;
		}
		 if(key_name=="price")
		{
			return data.price;
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
 				 a.emplace_back(iter.pid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a.emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a.emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a.emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a.emplace_back(iter.isview);
				 break;
			case 7: 
 				 a.emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a.emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a.emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a.emplace_back(iter.price);
				 break;
			case 21: 
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
 				 return data.pid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.topicid;
				 break;
			case 3: 
 				 return data.bigid;
				 break;
			case 4: 
 				 return data.smallid;
				 break;
			case 5: 
 				 return data.brandid;
				 break;
			case 6: 
 				 return data.isview;
				 break;
			case 7: 
 				 return data.isstore;
				 break;
			case 8: 
 				 return data.ishome;
				 break;
			case 9: 
 				 return data.showtype;
				 break;
			case 20: 
 				 return data.price;
				 break;
			case 21: 
 				 return data.sortid;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] productbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.pid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.topicid;
				 break;
			case 3: 
 				 return iter.bigid;
				 break;
			case 4: 
 				 return iter.smallid;
				 break;
			case 5: 
 				 return iter.brandid;
				 break;
			case 6: 
 				 return iter.isview;
				 break;
			case 7: 
 				 return iter.isstore;
				 break;
			case 8: 
 				 return iter.ishome;
				 break;
			case 9: 
 				 return iter.showtype;
				 break;
			case 20: 
 				 return iter.price;
				 break;
			case 21: 
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
            T getVal([[maybe_unused]] productbase::meta & iter,std::string keyname)
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

   			case 10: 
 				 return data.sntype;
				 break;
			case 11: 
 				 return data.name;
				 break;
			case 12: 
 				 return data.keywords;
				 break;
			case 13: 
 				 return data.introduce;
				 break;
			case 14: 
 				 return data.listimg;
				 break;
			case 15: 
 				 return data.bigimg;
				 break;
			case 16: 
 				 return data.maincontent;
				 break;
			case 17: 
 				 return data.paracontent;
				 break;
			case 18: 
 				 return data.samepro;
				 break;
			case 19: 
 				 return data.attatchfiles;
				 break;
			case 22: 
 				 return data.adddate;
				 break;
			case 23: 
 				 return data.editdate;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] productbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 10: 
 				 return iter.sntype;
				 break;
			case 11: 
 				 return iter.name;
				 break;
			case 12: 
 				 return iter.keywords;
				 break;
			case 13: 
 				 return iter.introduce;
				 break;
			case 14: 
 				 return iter.listimg;
				 break;
			case 15: 
 				 return iter.bigimg;
				 break;
			case 16: 
 				 return iter.maincontent;
				 break;
			case 17: 
 				 return iter.paracontent;
				 break;
			case 18: 
 				 return iter.samepro;
				 break;
			case 19: 
 				 return iter.attatchfiles;
				 break;
			case 22: 
 				 return iter.adddate;
				 break;
			case 23: 
 				 return iter.editdate;
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

    			case 10: 
 				 a.emplace_back(iter.sntype);
					 break;
			case 11: 
 				 a.emplace_back(iter.name);
					 break;
			case 12: 
 				 a.emplace_back(iter.keywords);
					 break;
			case 13: 
 				 a.emplace_back(iter.introduce);
					 break;
			case 14: 
 				 a.emplace_back(iter.listimg);
					 break;
			case 15: 
 				 a.emplace_back(iter.bigimg);
					 break;
			case 16: 
 				 a.emplace_back(iter.maincontent);
					 break;
			case 17: 
 				 a.emplace_back(iter.paracontent);
					 break;
			case 18: 
 				 a.emplace_back(iter.samepro);
					 break;
			case 19: 
 				 a.emplace_back(iter.attatchfiles);
					 break;
			case 22: 
 				 a.emplace_back(iter.adddate);
					 break;
			case 23: 
 				 a.emplace_back(iter.editdate);
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
 				 a<<std::to_string(iter.pid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.topicid);
				 break;
			case 3: 
 				 a<<std::to_string(iter.bigid);
				 break;
			case 4: 
 				 a<<std::to_string(iter.smallid);
				 break;
			case 5: 
 				 a<<std::to_string(iter.brandid);
				 break;
			case 6: 
 				 a<<std::to_string(iter.isview);
				 break;
			case 7: 
 				 a<<std::to_string(iter.isstore);
				 break;
			case 8: 
 				 a<<std::to_string(iter.ishome);
				 break;
			case 9: 
 				 a<<std::to_string(iter.showtype);
				 break;
			case 10: 
 				 if(isyinhao){ a<<jsonaddslash(iter.sntype); 
				 }else{
				 a<<iter.sntype;
				 }
				 break;
			case 11: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 12: 
 				 if(isyinhao){ a<<jsonaddslash(iter.keywords); 
				 }else{
				 a<<iter.keywords;
				 }
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.introduce); 
				 }else{
				 a<<iter.introduce;
				 }
				 break;
			case 14: 
 				 if(isyinhao){ a<<jsonaddslash(iter.listimg); 
				 }else{
				 a<<iter.listimg;
				 }
				 break;
			case 15: 
 				 if(isyinhao){ a<<jsonaddslash(iter.bigimg); 
				 }else{
				 a<<iter.bigimg;
				 }
				 break;
			case 16: 
 				 if(isyinhao){ a<<jsonaddslash(iter.maincontent); 
				 }else{
				 a<<iter.maincontent;
				 }
				 break;
			case 17: 
 				 if(isyinhao){ a<<jsonaddslash(iter.paracontent); 
				 }else{
				 a<<iter.paracontent;
				 }
				 break;
			case 18: 
 				 if(isyinhao){ a<<jsonaddslash(iter.samepro); 
				 }else{
				 a<<iter.samepro;
				 }
				 break;
			case 19: 
 				 if(isyinhao){ a<<jsonaddslash(iter.attatchfiles); 
				 }else{
				 a<<iter.attatchfiles;
				 }
				 break;
			case 20: 
 				 a<<std::to_string(iter.price);
				 break;
			case 21: 
 				 a<<std::to_string(iter.sortid);
				 break;
			case 22: 
 				 if(isyinhao){ a<<jsonaddslash(iter.adddate); 
				 }else{
				 a<<iter.adddate;
				 }
				 break;
			case 23: 
 				 if(isyinhao){ a<<jsonaddslash(iter.editdate); 
				 }else{
				 a<<iter.editdate;
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
				 } 
			switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
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
 
       			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
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
 	 ktemp=iter.pid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.topicid;
	 break;
case 3: 
 	 ktemp=iter.bigid;
	 break;
case 4: 
 	 ktemp=iter.smallid;
	 break;
case 5: 
 	 ktemp=iter.brandid;
	 break;
case 6: 
 	 ktemp=iter.isview;
	 break;
case 7: 
 	 ktemp=iter.isstore;
	 break;
case 8: 
 	 ktemp=iter.ishome;
	 break;
case 9: 
 	 ktemp=iter.showtype;
	 break;
case 20: 
 	 ktemp=iter.price;
	 break;
case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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
 				 a.emplace(iter.pid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.topicid,iter);
				 break;
			case 3: 
 				 a.emplace(iter.bigid,iter);
				 break;
			case 4: 
 				 a.emplace(iter.smallid,iter);
				 break;
			case 5: 
 				 a.emplace(iter.brandid,iter);
				 break;
			case 6: 
 				 a.emplace(iter.isview,iter);
				 break;
			case 7: 
 				 a.emplace(iter.isstore,iter);
				 break;
			case 8: 
 				 a.emplace(iter.ishome,iter);
				 break;
			case 9: 
 				 a.emplace(iter.showtype,iter);
				 break;
			case 20: 
 				 a.emplace(iter.price,iter);
				 break;
			case 21: 
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

   			case 10: 
 				 a.emplace(iter.sntype,iter);
			 break;
			case 11: 
 				 a.emplace(iter.name,iter);
			 break;
			case 12: 
 				 a.emplace(iter.keywords,iter);
			 break;
			case 13: 
 				 a.emplace(iter.introduce,iter);
			 break;
			case 14: 
 				 a.emplace(iter.listimg,iter);
			 break;
			case 15: 
 				 a.emplace(iter.bigimg,iter);
			 break;
			case 16: 
 				 a.emplace(iter.maincontent,iter);
			 break;
			case 17: 
 				 a.emplace(iter.paracontent,iter);
			 break;
			case 18: 
 				 a.emplace(iter.samepro,iter);
			 break;
			case 19: 
 				 a.emplace(iter.attatchfiles,iter);
			 break;
			case 22: 
 				 a.emplace(iter.adddate,iter);
			 break;
			case 23: 
 				 a.emplace(iter.editdate,iter);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
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
 				 ktemp=iter.pid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.topicid;
			 break;
			case 3: 
 				 ktemp=iter.bigid;
			 break;
			case 4: 
 				 ktemp=iter.smallid;
			 break;
			case 5: 
 				 ktemp=iter.brandid;
			 break;
			case 6: 
 				 ktemp=iter.isview;
			 break;
			case 7: 
 				 ktemp=iter.isstore;
			 break;
			case 8: 
 				 ktemp=iter.ishome;
			 break;
			case 9: 
 				 ktemp=iter.showtype;
			 break;
			case 20: 
 				 ktemp=iter.price;
			 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }
 			switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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

   case 10: 
 	 ktemp=iter.sntype;
	 break;
case 11: 
 	 ktemp=iter.name;
	 break;
case 12: 
 	 ktemp=iter.keywords;
	 break;
case 13: 
 	 ktemp=iter.introduce;
	 break;
case 14: 
 	 ktemp=iter.listimg;
	 break;
case 15: 
 	 ktemp=iter.bigimg;
	 break;
case 16: 
 	 ktemp=iter.maincontent;
	 break;
case 17: 
 	 ktemp=iter.paracontent;
	 break;
case 18: 
 	 ktemp=iter.samepro;
	 break;
case 19: 
 	 ktemp=iter.attatchfiles;
	 break;
case 22: 
 	 ktemp=iter.adddate;
	 break;
case 23: 
 	 ktemp=iter.editdate;
	 break;
	  }
 switch(vpos){
case 10: 
 	 vtemp=iter.sntype;
	 break;
case 11: 
 	 vtemp=iter.name;
	 break;
case 12: 
 	 vtemp=iter.keywords;
	 break;
case 13: 
 	 vtemp=iter.introduce;
	 break;
case 14: 
 	 vtemp=iter.listimg;
	 break;
case 15: 
 	 vtemp=iter.bigimg;
	 break;
case 16: 
 	 vtemp=iter.maincontent;
	 break;
case 17: 
 	 vtemp=iter.paracontent;
	 break;
case 18: 
 	 vtemp=iter.samepro;
	 break;
case 19: 
 	 vtemp=iter.attatchfiles;
	 break;
case 22: 
 	 vtemp=iter.adddate;
	 break;
case 23: 
 	 vtemp=iter.editdate;
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
 	 a.emplace_back(iter.pid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.topicid,iter);
	 break;
case 3: 
 	 a.emplace_back(iter.bigid,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.smallid,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.brandid,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.isview,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.isstore,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.ishome,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.showtype,iter);
	 break;
case 20: 
 	 a.emplace_back(iter.price,iter);
	 break;
case 21: 
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

   case 10: 
 	 a.emplace_back(iter.sntype,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.name,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.keywords,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.introduce,iter);
	 break;
case 14: 
 	 a.emplace_back(iter.listimg,iter);
	 break;
case 15: 
 	 a.emplace_back(iter.bigimg,iter);
	 break;
case 16: 
 	 a.emplace_back(iter.maincontent,iter);
	 break;
case 17: 
 	 a.emplace_back(iter.paracontent,iter);
	 break;
case 18: 
 	 a.emplace_back(iter.samepro,iter);
	 break;
case 19: 
 	 a.emplace_back(iter.attatchfiles,iter);
	 break;
case 22: 
 	 a.emplace_back(iter.adddate,iter);
	 break;
case 23: 
 	 a.emplace_back(iter.editdate,iter);
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
 				 vtemp=iter.sortid;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.price);
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.editdate);
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.price);
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
				 break;
			  }

			 switch(dpos){
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.editdate);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
 				 vtemp=iter.sortid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.price);
				 break;
			case 21: 
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.pid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.topicid;
				 break;
			case 3: 
 				 vtemp=iter.bigid;
				 break;
			case 4: 
 				 vtemp=iter.smallid;
				 break;
			case 5: 
 				 vtemp=iter.brandid;
				 break;
			case 6: 
 				 vtemp=iter.isview;
				 break;
			case 7: 
 				 vtemp=iter.isstore;
				 break;
			case 8: 
 				 vtemp=iter.ishome;
				 break;
			case 9: 
 				 vtemp=iter.showtype;
				 break;
			case 20: 
 				 vtemp=iter.price;
				 break;
			case 21: 
 				 vtemp=iter.sortid;
				 break;
			 }

			switch(dpos){
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.editdate);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			 }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp][vtemp].emplace_back(iter.price);
				 break;
			case 21: 
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }

			 switch(vpos){
			case 10: 
 				 vtemp=iter.sntype;
				 break;
			case 11: 
 				 vtemp=iter.name;
				 break;
			case 12: 
 				 vtemp=iter.keywords;
				 break;
			case 13: 
 				 vtemp=iter.introduce;
				 break;
			case 14: 
 				 vtemp=iter.listimg;
				 break;
			case 15: 
 				 vtemp=iter.bigimg;
				 break;
			case 16: 
 				 vtemp=iter.maincontent;
				 break;
			case 17: 
 				 vtemp=iter.paracontent;
				 break;
			case 18: 
 				 vtemp=iter.samepro;
				 break;
			case 19: 
 				 vtemp=iter.attatchfiles;
				 break;
			case 22: 
 				 vtemp=iter.adddate;
				 break;
			case 23: 
 				 vtemp=iter.editdate;
				 break;
			  }

			 switch(dpos){
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp][vtemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp][vtemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp][vtemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp][vtemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp][vtemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp][vtemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp][vtemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp][vtemp].emplace_back(iter.editdate);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }

			 switch(vpos){
			case 10: 
 				 a[ktemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.editdate);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.price);
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 10: 
 				 a[ktemp].emplace_back(iter.sntype);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.keywords);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.introduce);
				 break;
			case 14: 
 				 a[ktemp].emplace_back(iter.listimg);
				 break;
			case 15: 
 				 a[ktemp].emplace_back(iter.bigimg);
				 break;
			case 16: 
 				 a[ktemp].emplace_back(iter.maincontent);
				 break;
			case 17: 
 				 a[ktemp].emplace_back(iter.paracontent);
				 break;
			case 18: 
 				 a[ktemp].emplace_back(iter.samepro);
				 break;
			case 19: 
 				 a[ktemp].emplace_back(iter.attatchfiles);
				 break;
			case 22: 
 				 a[ktemp].emplace_back(iter.adddate);
				 break;
			case 23: 
 				 a[ktemp].emplace_back(iter.editdate);
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.pid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.topicid);
				 break;
			case 3: 
 				 a[ktemp].emplace_back(iter.bigid);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.smallid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.brandid);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.isview);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.isstore);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.ishome);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.showtype);
				 break;
			case 20: 
 				 a[ktemp].emplace_back(iter.price);
				 break;
			case 21: 
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
 				 a[iter.pid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[iter.bigid].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.smallid].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.brandid].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.isstore].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.ishome].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.showtype].emplace_back(iter);
				 break;
			case 20: 
 				 a[iter.price].emplace_back(iter);
				 break;
			case 21: 
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

   			case 10: 
 				 a[iter.sntype].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.name].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.keywords].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.introduce].emplace_back(iter);
				 break;
			case 14: 
 				 a[iter.listimg].emplace_back(iter);
				 break;
			case 15: 
 				 a[iter.bigimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[iter.maincontent].emplace_back(iter);
				 break;
			case 17: 
 				 a[iter.paracontent].emplace_back(iter);
				 break;
			case 18: 
 				 a[iter.samepro].emplace_back(iter);
				 break;
			case 19: 
 				 a[iter.attatchfiles].emplace_back(iter);
				 break;
			case 22: 
 				 a[iter.adddate].emplace_back(iter);
				 break;
			case 23: 
 				 a[iter.editdate].emplace_back(iter);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
			 }

			 switch(vpos){
			case 10: 
 				 a[ktemp][iter.sntype].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.keywords].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.listimg].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.bigimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.maincontent].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.paracontent].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.samepro].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.attatchfiles].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.adddate].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.editdate].emplace_back(iter);
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

   			case 10: 
 				 ktemp=iter.sntype;
				 break;
			case 11: 
 				 ktemp=iter.name;
				 break;
			case 12: 
 				 ktemp=iter.keywords;
				 break;
			case 13: 
 				 ktemp=iter.introduce;
				 break;
			case 14: 
 				 ktemp=iter.listimg;
				 break;
			case 15: 
 				 ktemp=iter.bigimg;
				 break;
			case 16: 
 				 ktemp=iter.maincontent;
				 break;
			case 17: 
 				 ktemp=iter.paracontent;
				 break;
			case 18: 
 				 ktemp=iter.samepro;
				 break;
			case 19: 
 				 ktemp=iter.attatchfiles;
				 break;
			case 22: 
 				 ktemp=iter.adddate;
				 break;
			case 23: 
 				 ktemp=iter.editdate;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.pid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.bigid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.smallid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.brandid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.isstore].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.ishome].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.showtype].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.price].emplace_back(iter);
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.pid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.topicid].emplace_back(iter);
				 break;
			case 3: 
 				 a[ktemp][iter.bigid].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.smallid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.brandid].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.isview].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.isstore].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.ishome].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.showtype].emplace_back(iter);
				 break;
			case 20: 
 				 a[ktemp][iter.price].emplace_back(iter);
				 break;
			case 21: 
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
 				 ktemp=iter.pid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.topicid;
				 break;
			case 3: 
 				 ktemp=iter.bigid;
				 break;
			case 4: 
 				 ktemp=iter.smallid;
				 break;
			case 5: 
 				 ktemp=iter.brandid;
				 break;
			case 6: 
 				 ktemp=iter.isview;
				 break;
			case 7: 
 				 ktemp=iter.isstore;
				 break;
			case 8: 
 				 ktemp=iter.ishome;
				 break;
			case 9: 
 				 ktemp=iter.showtype;
				 break;
			case 20: 
 				 ktemp=iter.price;
				 break;
			case 21: 
 				 ktemp=iter.sortid;
				 break;
			  }

			 switch(vpos){
			case 10: 
 				 a[ktemp][iter.sntype].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.keywords].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.introduce].emplace_back(iter);
				 break;
			case 14: 
 				 a[ktemp][iter.listimg].emplace_back(iter);
				 break;
			case 15: 
 				 a[ktemp][iter.bigimg].emplace_back(iter);
				 break;
			case 16: 
 				 a[ktemp][iter.maincontent].emplace_back(iter);
				 break;
			case 17: 
 				 a[ktemp][iter.paracontent].emplace_back(iter);
				 break;
			case 18: 
 				 a[ktemp][iter.samepro].emplace_back(iter);
				 break;
			case 19: 
 				 a[ktemp][iter.attatchfiles].emplace_back(iter);
				 break;
			case 22: 
 				 a[ktemp][iter.adddate].emplace_back(iter);
				 break;
			case 23: 
 				 a[ktemp][iter.editdate].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   