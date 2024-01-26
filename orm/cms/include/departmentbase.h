#ifndef ORM_CMS_DEPARTMENTBASEMATA_H
#define ORM_CMS_DEPARTMENTBASEMATA_H
/*
*This file is auto create from cli
*本文件为自动生成 Fri, 26 Jan 2024 02:59:40 GMT
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

struct departmentbase
{
    struct meta{
    unsigned  int dpid= 0; //
unsigned  int userid= 0; //用户id
unsigned  int parentid= 0; //父id
 std::string name=""; //部门名称
 std::string depart_code=""; //部门代号
 int bianzhi_num= 0; //编制人数
 int real_num= 0; //实际人数
 int quan_weight= 0; //部门权重
 char isopen=0; //是否开放
 std::string memo=""; //备注
unsigned  int addtime= 0; //添加时间
unsigned  int edittime= 0; //修改时间
 char isvirtual=0; //是否虚拟部门
 std::string linkdpid=""; //关联部门
 } data;
  
        struct meta_tree{
        unsigned  int dpid= 0; //
unsigned  int userid= 0; //用户id
unsigned  int parentid= 0; //父id
 std::string name=""; //部门名称
 std::string depart_code=""; //部门代号
 int bianzhi_num= 0; //编制人数
 int real_num= 0; //实际人数
 int quan_weight= 0; //部门权重
 char isopen=0; //是否开放
 std::string memo=""; //备注
unsigned  int addtime= 0; //添加时间
unsigned  int edittime= 0; //修改时间
 char isvirtual=0; //是否虚拟部门
 std::string linkdpid=""; //关联部门

	std::vector<meta_tree> children;
 };
 std::vector<departmentbase::meta> record;
std::string _rmstag="cms";//this value must be default or tag value, tag in mysqlconnect config file .
std::vector<unsigned char> _keypos{0x00};
MYSQL_ROW _row;
std::vector<departmentbase::meta>::iterator begin(){     return record.begin(); }
std::vector<departmentbase::meta>::iterator end(){     return record.end(); }
std::vector<departmentbase::meta>::const_iterator begin() const{     return record.begin(); }
std::vector<departmentbase::meta>::const_iterator end() const{     return record.end(); }
const std::array<std::string,14> colnames={"dpid","userid","parentid","name","depart_code","bianzhi_num","real_num","quan_weight","isopen","memo","addtime","edittime","isvirtual","linkdpid"};
const std::array<unsigned char,14> colnamestype= {3,3,3,253,253,3,3,3,1,252,3,3,1,253};
std::string tablename="department";
std::string modelname="Department";

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
   	 return 10;
break;
case 'b':
   	 return 5;
break;
case 'd':
 switch(coln.size()){  
case 4:
   	 return 0;
break;
case 11:
   	 return 4;
break;
 }
 break;
case 'e':
   	 return 11;
break;
case 'i':
 switch(coln.size()){  
case 6:
   	 return 8;
break;
case 9:
   	 return 12;
break;
 }
 break;
case 'l':
   	 return 13;
break;
case 'm':
   	 return 9;
break;
case 'n':
   	 return 3;
break;
case 'p':
   	 return 2;
break;
case 'q':
   	 return 7;
break;
case 'r':
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
       return "dpid";
}

      void record_reset()
      {
            record.clear();     
      }
      void data_reset(){
     departmentbase::meta metatemp;    
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
			data.dpid=std::stoul(_row[i]);
		}catch (...) { 
			data.dpid=0;
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
			data.name.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.name.clear();
			 }
			break;
	case 4:
		 try{
			data.depart_code.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
	case 5:
		 try{
			data.bianzhi_num=std::stoi(_row[i]);
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
	case 6:
		 try{
			data.real_num=std::stoi(_row[i]);
		}catch (...) { 
			data.real_num=0;
			 }
			break;
	case 7:
		 try{
			data.quan_weight=std::stoi(_row[i]);
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
	case 8:
		 try{
			data.isopen=std::stoi(_row[i]);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
	case 9:
		 try{
			data.memo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.memo.clear();
			 }
			break;
	case 10:
		 try{
			data.addtime=std::stoul(_row[i]);
		}catch (...) { 
			data.addtime=0;
			 }
			break;
	case 11:
		 try{
			data.edittime=std::stoul(_row[i]);
		}catch (...) { 
			data.edittime=0;
			 }
			break;
	case 12:
		 try{
			data.isvirtual=std::stoi(_row[i]);
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
	case 13:
		 try{
			data.linkdpid.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			data.linkdpid.clear();
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
           departmentbase::meta metatemp;   

          for(unsigned char i=0;i<_keypos.size();i++){
 
                 switch(_keypos[i]){

        	case 0:
		 try{
			metatemp.dpid=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.dpid=0;
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
			metatemp.name.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.name.clear();
			 }
			break;
	case 4:
		 try{
			metatemp.depart_code.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.depart_code.clear();
			 }
			break;
	case 5:
		 try{
			metatemp.bianzhi_num=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.bianzhi_num=0;
			 }
			break;
	case 6:
		 try{
			metatemp.real_num=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.real_num=0;
			 }
			break;
	case 7:
		 try{
			metatemp.quan_weight=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.quan_weight=0;
			 }
			break;
	case 8:
		 try{
			metatemp.isopen=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.isopen=0;
			 }
			break;
	case 9:
		 try{
			metatemp.memo.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.memo.clear();
			 }
			break;
	case 10:
		 try{
			metatemp.addtime=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.addtime=0;
			 }
			break;
	case 11:
		 try{
			metatemp.edittime=std::stoul(_row[i]);
		}catch (...) { 
			metatemp.edittime=0;
			 }
			break;
	case 12:
		 try{
			metatemp.isvirtual=std::stoi(_row[i]);
		}catch (...) { 
			metatemp.isvirtual=0;
			 }
			break;
	case 13:
		 try{
			metatemp.linkdpid.append((_row[i]==NULL?"":_row[i]));
		}catch (...) { 
			metatemp.linkdpid.clear();
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

        if(data.dpid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(data.dpid);
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
tempsql<<",'"<<stringaddslash(data.name)<<"'";
tempsql<<",'"<<stringaddslash(data.depart_code)<<"'";
if(data.bianzhi_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isopen);
}
tempsql<<",'"<<stringaddslash(data.memo)<<"'";
if(data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.addtime);
}
if(data.edittime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.edittime);
}
if(data.isvirtual==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(data.isvirtual);
}
tempsql<<",'"<<stringaddslash(data.linkdpid)<<"'";
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

        if(insert_data.dpid==0){
tempsql<<"null";
 }else{ 
	tempsql<<std::to_string(insert_data.dpid);
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
tempsql<<",'"<<stringaddslash(insert_data.name)<<"'";
tempsql<<",'"<<stringaddslash(insert_data.depart_code)<<"'";
if(insert_data.bianzhi_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.bianzhi_num);
}
if(insert_data.real_num==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.real_num);
}
if(insert_data.quan_weight==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.quan_weight);
}
if(insert_data.isopen==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isopen);
}
tempsql<<",'"<<stringaddslash(insert_data.memo)<<"'";
if(insert_data.addtime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.addtime);
}
if(insert_data.edittime==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.edittime);
}
if(insert_data.isvirtual==0){
	tempsql<<",0";
 }else{ 
	tempsql<<","<<std::to_string(insert_data.isvirtual);
}
tempsql<<",'"<<stringaddslash(insert_data.linkdpid)<<"'";
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


        	if(insert_data[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(insert_data[i].dpid);
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
		tempsql<<",'"<<stringaddslash(insert_data[i].name)<<"'";
		tempsql<<",'"<<stringaddslash(insert_data[i].depart_code)<<"'";
	if(insert_data[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].bianzhi_num);
	}
	if(insert_data[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].real_num);
	}
	if(insert_data[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].quan_weight);
	}
	if(insert_data[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isopen);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].memo)<<"'";
	if(insert_data[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].addtime);
	}
	if(insert_data[i].edittime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].edittime);
	}
	if(insert_data[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(insert_data[i].isvirtual);
	}
		tempsql<<",'"<<stringaddslash(insert_data[i].linkdpid)<<"'";
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

        if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
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
tempsql<<",`name`='"<<stringaddslash(data.name)<<"'";
tempsql<<",`depart_code`='"<<stringaddslash(data.depart_code)<<"'";
if(data.bianzhi_num==0){
	tempsql<<",`bianzhi_num`=0";
 }else{ 
	tempsql<<",`bianzhi_num`="<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",`real_num`=0";
 }else{ 
	tempsql<<",`real_num`="<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",`quan_weight`=0";
 }else{ 
	tempsql<<",`quan_weight`="<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",`isopen`=0";
 }else{ 
	tempsql<<",`isopen`="<<std::to_string(data.isopen);
}
tempsql<<",`memo`='"<<stringaddslash(data.memo)<<"'";
if(data.addtime==0){
	tempsql<<",`addtime`=0";
 }else{ 
	tempsql<<",`addtime`="<<std::to_string(data.addtime);
}
if(data.edittime==0){
	tempsql<<",`edittime`=0";
 }else{ 
	tempsql<<",`edittime`="<<std::to_string(data.edittime);
}
if(data.isvirtual==0){
	tempsql<<",`isvirtual`=0";
 }else{ 
	tempsql<<",`isvirtual`="<<std::to_string(data.isvirtual);
}
tempsql<<",`linkdpid`='"<<stringaddslash(data.linkdpid)<<"'";
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
if(data.dpid==0){
	tempsql<<"`dpid`=0";
 }else{ 
	tempsql<<"`dpid`="<<std::to_string(data.dpid);
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
tempsql<<"`name`='"<<stringaddslash(data.name)<<"'";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`depart_code`='"<<stringaddslash(data.depart_code)<<"'";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.bianzhi_num==0){
	tempsql<<"`bianzhi_num`=0";
 }else{ 
	tempsql<<"`bianzhi_num`="<<std::to_string(data.bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.real_num==0){
	tempsql<<"`real_num`=0";
 }else{ 
	tempsql<<"`real_num`="<<std::to_string(data.real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.quan_weight==0){
	tempsql<<"`quan_weight`=0";
 }else{ 
	tempsql<<"`quan_weight`="<<std::to_string(data.quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"`isopen`=0";
 }else{ 
	tempsql<<"`isopen`="<<std::to_string(data.isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`memo`='"<<stringaddslash(data.memo)<<"'";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"`addtime`=0";
 }else{ 
	tempsql<<"`addtime`="<<std::to_string(data.addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.edittime==0){
	tempsql<<"`edittime`=0";
 }else{ 
	tempsql<<"`edittime`="<<std::to_string(data.edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.isvirtual==0){
	tempsql<<"`isvirtual`=0";
 }else{ 
	tempsql<<"`isvirtual`="<<std::to_string(data.isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"`linkdpid`='"<<stringaddslash(data.linkdpid)<<"'";
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
            	if(record[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].dpid);
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
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].depart_code)<<"'";
	if(record[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bianzhi_num);
	}
	if(record[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].real_num);
	}
	if(record[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].quan_weight);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].edittime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].edittime);
	}
	if(record[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isvirtual);
	}
	tempsql<<",'"<<stringaddslash(record[i].linkdpid)<<"'";
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
            	if(record[i].dpid==0){
	tempsql<<"null";
	 }else{ 
	tempsql<<std::to_string(record[i].dpid);
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
	tempsql<<",'"<<stringaddslash(record[i].name)<<"'";
	tempsql<<",'"<<stringaddslash(record[i].depart_code)<<"'";
	if(record[i].bianzhi_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].bianzhi_num);
	}
	if(record[i].real_num==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].real_num);
	}
	if(record[i].quan_weight==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].quan_weight);
	}
	if(record[i].isopen==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isopen);
	}
	tempsql<<",'"<<stringaddslash(record[i].memo)<<"'";
	if(record[i].addtime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].addtime);
	}
	if(record[i].edittime==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].edittime);
	}
	if(record[i].isvirtual==0){
	tempsql<<",0";
	 }else{ 
	tempsql<<","<<std::to_string(record[i].isvirtual);
	}
	tempsql<<",'"<<stringaddslash(record[i].linkdpid)<<"'";
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
if(data.dpid==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.dpid));
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
	temparray.push_back(data.name);
 break;
 case 4:
	temparray.push_back(data.depart_code);
 break;
 case 5:
if(data.bianzhi_num==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.bianzhi_num));
}
 break;
 case 6:
if(data.real_num==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.real_num));
}
 break;
 case 7:
if(data.quan_weight==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.quan_weight));
}
 break;
 case 8:
if(data.isopen==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isopen));
}
 break;
 case 9:
	temparray.push_back(data.memo);
 break;
 case 10:
if(data.addtime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.addtime));
}
 break;
 case 11:
if(data.edittime==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.edittime));
}
 break;
 case 12:
if(data.isvirtual==0){
	temparray.push_back("0");
 }else{ 
	temparray.push_back(std::to_string(data.isvirtual));
}
 break;
 case 13:
	temparray.push_back(data.linkdpid);
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
if(data.dpid==0){
	tempsql.insert({"dpid","0"});
 }else{ 
	tempsql.insert({"dpid",std::to_string(data.dpid)});
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
	tempsql.insert({"name",data.name});
 break;
 case 4:
	tempsql.insert({"depart_code",data.depart_code});
 break;
 case 5:
if(data.bianzhi_num==0){
	tempsql.insert({"bianzhi_num","0"});
 }else{ 
	tempsql.insert({"bianzhi_num",std::to_string(data.bianzhi_num)});
}
 break;
 case 6:
if(data.real_num==0){
	tempsql.insert({"real_num","0"});
 }else{ 
	tempsql.insert({"real_num",std::to_string(data.real_num)});
}
 break;
 case 7:
if(data.quan_weight==0){
	tempsql.insert({"quan_weight","0"});
 }else{ 
	tempsql.insert({"quan_weight",std::to_string(data.quan_weight)});
}
 break;
 case 8:
if(data.isopen==0){
	tempsql.insert({"isopen","0"});
 }else{ 
	tempsql.insert({"isopen",std::to_string(data.isopen)});
}
 break;
 case 9:
	tempsql.insert({"memo",data.memo});
 break;
 case 10:
if(data.addtime==0){
	tempsql.insert({"addtime","0"});
 }else{ 
	tempsql.insert({"addtime",std::to_string(data.addtime)});
}
 break;
 case 11:
if(data.edittime==0){
	tempsql.insert({"edittime","0"});
 }else{ 
	tempsql.insert({"edittime",std::to_string(data.edittime)});
}
 break;
 case 12:
if(data.isvirtual==0){
	tempsql.insert({"isvirtual","0"});
 }else{ 
	tempsql.insert({"isvirtual",std::to_string(data.isvirtual)});
}
 break;
 case 13:
	tempsql.insert({"linkdpid",data.linkdpid});
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
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
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
tempsql<<",\"name\":\""<<http::utf8_to_jsonstring(data.name);
tempsql<<"\"";
tempsql<<",\"depart_code\":\""<<http::utf8_to_jsonstring(data.depart_code);
tempsql<<"\"";
if(data.bianzhi_num==0){
	tempsql<<",\"bianzhi_num\":0";
 }else{ 
	tempsql<<",\"bianzhi_num\":"<<std::to_string(data.bianzhi_num);
}
if(data.real_num==0){
	tempsql<<",\"real_num\":0";
 }else{ 
	tempsql<<",\"real_num\":"<<std::to_string(data.real_num);
}
if(data.quan_weight==0){
	tempsql<<",\"quan_weight\":0";
 }else{ 
	tempsql<<",\"quan_weight\":"<<std::to_string(data.quan_weight);
}
if(data.isopen==0){
	tempsql<<",\"isopen\":0";
 }else{ 
	tempsql<<",\"isopen\":"<<std::to_string(data.isopen);
}
tempsql<<",\"memo\":\""<<http::utf8_to_jsonstring(data.memo);
tempsql<<"\"";
if(data.addtime==0){
	tempsql<<",\"addtime\":0";
 }else{ 
	tempsql<<",\"addtime\":"<<std::to_string(data.addtime);
}
if(data.edittime==0){
	tempsql<<",\"edittime\":0";
 }else{ 
	tempsql<<",\"edittime\":"<<std::to_string(data.edittime);
}
if(data.isvirtual==0){
	tempsql<<",\"isvirtual\":0";
 }else{ 
	tempsql<<",\"isvirtual\":"<<std::to_string(data.isvirtual);
}
tempsql<<",\"linkdpid\":\""<<http::utf8_to_jsonstring(data.linkdpid);
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
                     for(jj=0;jj<colnames.size();jj++){
                         keypos.emplace_back(jj); 
                     }
                 }
                 tempsql<<"{";
                 for(jj=0;jj<keypos.size();jj++){
                       switch(keypos[jj]){
         case 0:
 if(jj>0){ tempsql<<","; } 
if(data.dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(data.dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(data.name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(data.depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(data.bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(data.bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(data.real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(data.real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(data.quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(data.quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(data.isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(data.isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(data.memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(data.addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(data.addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(data.edittime==0){
	tempsql<<"\"edittime\":0";
 }else{ 
	tempsql<<"\"edittime\":"<<std::to_string(data.edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(data.isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(data.isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(data.linkdpid)<<"\"";
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
        departmentbase::meta metatemp; 
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
			data.dpid=std::stoul(set_value_name);
		}catch (...) { 
			data.dpid=0;
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
			data.name.append(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code.append(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=std::stoi(set_value_name);
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=std::stoi(set_value_name);
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=std::stoi(set_value_name);
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=std::stoi(set_value_name);
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo.append(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.addtime=std::stoul(set_value_name);
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 11:
		 try{
			data.edittime=std::stoul(set_value_name);
		}catch (...) { 
			data.edittime=0;
			 }
			break;
		case 12:
		 try{
			data.isvirtual=std::stoi(set_value_name);
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 13:
		 try{
			data.linkdpid.append(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
			data.dpid=set_value_name;
		}catch (...) { 
			data.dpid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code=std::to_string(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=set_value_name;
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=set_value_name;
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=set_value_name;
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.addtime=set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 11:
		 try{
			data.edittime=set_value_name;
		}catch (...) { 
			data.edittime=0;
			 }
			break;
		case 12:
		 try{
			data.isvirtual=set_value_name;
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 13:
		 try{
			data.linkdpid=std::to_string(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
			data.dpid=(unsigned int)set_value_name;
		}catch (...) { 
			data.dpid=0;
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
			data.name=std::to_string(set_value_name);
		}catch (...) { 
			data.name.clear();
			 }
			break;
		case 4:
		 try{
			data.depart_code=std::to_string(set_value_name);
		}catch (...) { 
			data.depart_code.clear();
			 }
			break;
		case 5:
		 try{
			data.bianzhi_num=(int)set_value_name;
		}catch (...) { 
			data.bianzhi_num=0;
			 }
			break;
		case 6:
		 try{
			data.real_num=(int)set_value_name;
		}catch (...) { 
			data.real_num=0;
			 }
			break;
		case 7:
		 try{
			data.quan_weight=(int)set_value_name;
		}catch (...) { 
			data.quan_weight=0;
			 }
			break;
		case 8:
		 try{
			data.isopen=(int)set_value_name;
		}catch (...) { 
			data.isopen=0;
			 }
			break;
		case 9:
		 try{
			data.memo=std::to_string(set_value_name);
		}catch (...) { 
			data.memo.clear();
			 }
			break;
		case 10:
		 try{
			data.addtime=(unsigned int)set_value_name;
		}catch (...) { 
			data.addtime=0;
			 }
			break;
		case 11:
		 try{
			data.edittime=(unsigned int)set_value_name;
		}catch (...) { 
			data.edittime=0;
			 }
			break;
		case 12:
		 try{
			data.isvirtual=(int)set_value_name;
		}catch (...) { 
			data.isvirtual=0;
			 }
			break;
		case 13:
		 try{
			data.linkdpid=std::to_string(set_value_name);
		}catch (...) { 
			data.linkdpid.clear();
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
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(record[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(record[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(record[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(record[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].edittime==0){
	tempsql<<"\"edittime\":0";
 }else{ 
	tempsql<<"\"edittime\":"<<std::to_string(record[n].edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(record[n].isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(record[n].linkdpid)<<"\"";
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
if(record[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(record[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(record[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(record[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(record[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(record[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(record[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(record[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(record[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(record[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(record[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(record[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(record[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(record[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(record[n].addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(record[n].edittime==0){
	tempsql<<"\"edittime\":0";
 }else{ 
	tempsql<<"\"edittime\":"<<std::to_string(record[n].edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(record[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(record[n].isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(record[n].linkdpid)<<"\"";
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
   long long getPK(){  return data.dpid; } 
 void setPK(long long val){  data.dpid=val;} 
unsigned  int  getDpid(){  return data.dpid; } 
 void setDpid(unsigned  int  val){  data.dpid=val;} 

unsigned  int  getUserid(){  return data.userid; } 
 void setUserid(unsigned  int  val){  data.userid=val;} 

unsigned  int  getParentid(){  return data.parentid; } 
 void setParentid(unsigned  int  val){  data.parentid=val;} 

std::string getName(){  return data.name; } 
std::string& getRefName(){  return std::ref(data.name); } 
 void setName(std::string &val){  data.name=val;} 
 void setName(std::string_view val){  data.name=val;} 

std::string getDepartCode(){  return data.depart_code; } 
std::string& getRefDepartCode(){  return std::ref(data.depart_code); } 
 void setDepartCode(std::string &val){  data.depart_code=val;} 
 void setDepartCode(std::string_view val){  data.depart_code=val;} 

 int  getBianzhiNum(){  return data.bianzhi_num; } 
 void setBianzhiNum( int  val){  data.bianzhi_num=val;} 

 int  getRealNum(){  return data.real_num; } 
 void setRealNum( int  val){  data.real_num=val;} 

 int  getQuanWeight(){  return data.quan_weight; } 
 void setQuanWeight( int  val){  data.quan_weight=val;} 

 int  getIsopen(){  return data.isopen; } 
 void setIsopen( int  val){  data.isopen=val;} 

std::string getMemo(){  return data.memo; } 
std::string& getRefMemo(){  return std::ref(data.memo); } 
 void setMemo(std::string &val){  data.memo=val;} 
 void setMemo(std::string_view val){  data.memo=val;} 

unsigned  int  getAddtime(){  return data.addtime; } 
 void setAddtime(unsigned  int  val){  data.addtime=val;} 

unsigned  int  getEdittime(){  return data.edittime; } 
 void setEdittime(unsigned  int  val){  data.edittime=val;} 

 int  getIsvirtual(){  return data.isvirtual; } 
 void setIsvirtual( int  val){  data.isvirtual=val;} 

std::string getLinkdpid(){  return data.linkdpid; } 
std::string& getRefLinkdpid(){  return std::ref(data.linkdpid); } 
 void setLinkdpid(std::string &val){  data.linkdpid=val;} 
 void setLinkdpid(std::string_view val){  data.linkdpid=val;} 

departmentbase::meta getnewData(){
 	 struct meta newdata;
	 return newdata; 
} 
departmentbase::meta getData(){
 	 return data; 
} 
std::vector<departmentbase::meta> getRecord(){
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
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(tree_data[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(tree_data[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(tree_data[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(tree_data[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].edittime==0){
	tempsql<<"\"edittime\":0";
 }else{ 
	tempsql<<"\"edittime\":"<<std::to_string(tree_data[n].edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(tree_data[n].isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(tree_data[n].linkdpid)<<"\"";
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
if(tree_data[n].dpid==0){
	tempsql<<"\"dpid\":0";
 }else{ 
	tempsql<<"\"dpid\":"<<std::to_string(tree_data[n].dpid);
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
tempsql<<"\"name\":\""<<http::utf8_to_jsonstring(tree_data[n].name)<<"\"";
 break;
 case 4:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"depart_code\":\""<<http::utf8_to_jsonstring(tree_data[n].depart_code)<<"\"";
 break;
 case 5:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].bianzhi_num==0){
	tempsql<<"\"bianzhi_num\":0";
 }else{ 
	tempsql<<"\"bianzhi_num\":"<<std::to_string(tree_data[n].bianzhi_num);
}
 break;
 case 6:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].real_num==0){
	tempsql<<"\"real_num\":0";
 }else{ 
	tempsql<<"\"real_num\":"<<std::to_string(tree_data[n].real_num);
}
 break;
 case 7:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].quan_weight==0){
	tempsql<<"\"quan_weight\":0";
 }else{ 
	tempsql<<"\"quan_weight\":"<<std::to_string(tree_data[n].quan_weight);
}
 break;
 case 8:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isopen==0){
	tempsql<<"\"isopen\":0";
 }else{ 
	tempsql<<"\"isopen\":"<<std::to_string(tree_data[n].isopen);
}
 break;
 case 9:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"memo\":\""<<http::utf8_to_jsonstring(tree_data[n].memo)<<"\"";
 break;
 case 10:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].addtime==0){
	tempsql<<"\"addtime\":0";
 }else{ 
	tempsql<<"\"addtime\":"<<std::to_string(tree_data[n].addtime);
}
 break;
 case 11:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].edittime==0){
	tempsql<<"\"edittime\":0";
 }else{ 
	tempsql<<"\"edittime\":"<<std::to_string(tree_data[n].edittime);
}
 break;
 case 12:
 if(jj>0){ tempsql<<","; } 
if(tree_data[n].isvirtual==0){
	tempsql<<"\"isvirtual\":0";
 }else{ 
	tempsql<<"\"isvirtual\":"<<std::to_string(tree_data[n].isvirtual);
}
 break;
 case 13:
 if(jj>0){ tempsql<<","; } 
tempsql<<"\"linkdpid\":\""<<http::utf8_to_jsonstring(tree_data[n].linkdpid)<<"\"";
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
   		 if(key_name=="name")
		{
			return data.name;
		}
		 if(key_name=="depart_code")
		{
			return data.depart_code;
		}
		 if(key_name=="memo")
		{
			return data.memo;
		}
		 if(key_name=="linkdpid")
		{
			return data.linkdpid;
		}
		return nullptr; 
	}


    template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true>
    T& ref_meta([[maybe_unused]] std::string key_name)
    {
   		 if(key_name=="dpid")
		{
			return data.dpid;
		}
		 if(key_name=="userid")
		{
			return data.userid;
		}
		 if(key_name=="parentid")
		{
			return data.parentid;
		}
		 if(key_name=="bianzhi_num")
		{
			return data.bianzhi_num;
		}
		 if(key_name=="real_num")
		{
			return data.real_num;
		}
		 if(key_name=="quan_weight")
		{
			return data.quan_weight;
		}
		 if(key_name=="isopen")
		{
			return data.isopen;
		}
		 if(key_name=="addtime")
		{
			return data.addtime;
		}
		 if(key_name=="edittime")
		{
			return data.edittime;
		}
		 if(key_name=="isvirtual")
		{
			return data.isvirtual;
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
 				 a.emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a.emplace_back(iter.userid);
				 break;
			case 2: 
 				 a.emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a.emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a.emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a.emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a.emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a.emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a.emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a.emplace_back(iter.isvirtual);
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
 				 return data.dpid;
				 break;
			case 1: 
 				 return data.userid;
				 break;
			case 2: 
 				 return data.parentid;
				 break;
			case 5: 
 				 return data.bianzhi_num;
				 break;
			case 6: 
 				 return data.real_num;
				 break;
			case 7: 
 				 return data.quan_weight;
				 break;
			case 8: 
 				 return data.isopen;
				 break;
			case 10: 
 				 return data.addtime;
				 break;
			case 11: 
 				 return data.edittime;
				 break;
			case 12: 
 				 return data.isvirtual;
				 break;
			}
                return 0;
            }  
    
        template<typename T, typename std::enable_if<std::is_integral_v<T>,bool>::type = true > 
        T getVal([[maybe_unused]] departmentbase::meta & iter,[[maybe_unused]] std::string keyname)
        {

          
            unsigned char kpos;
            kpos=findcolpos(keyname);   
            switch(kpos)
            {
   			case 0: 
 				 return iter.dpid;
				 break;
			case 1: 
 				 return iter.userid;
				 break;
			case 2: 
 				 return iter.parentid;
				 break;
			case 5: 
 				 return iter.bianzhi_num;
				 break;
			case 6: 
 				 return iter.real_num;
				 break;
			case 7: 
 				 return iter.quan_weight;
				 break;
			case 8: 
 				 return iter.isopen;
				 break;
			case 10: 
 				 return iter.addtime;
				 break;
			case 11: 
 				 return iter.edittime;
				 break;
			case 12: 
 				 return iter.isvirtual;
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
            T getVal([[maybe_unused]] departmentbase::meta & iter,std::string keyname)
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

   			case 3: 
 				 return data.name;
				 break;
			case 4: 
 				 return data.depart_code;
				 break;
			case 9: 
 				 return data.memo;
				 break;
			case 13: 
 				 return data.linkdpid;
				 break;

                }
                return "";
            }  
   
            template<typename T, typename std::enable_if<std::is_same<T,std::string>::value,bool>::type = true > 
            std::string getVal([[maybe_unused]] departmentbase::meta & iter,std::string keyname)
            {
         
                unsigned char kpos;
                kpos=findcolpos(keyname);
       
                switch(kpos)
                {

   			case 3: 
 				 return iter.name;
				 break;
			case 4: 
 				 return iter.depart_code;
				 break;
			case 9: 
 				 return iter.memo;
				 break;
			case 13: 
 				 return iter.linkdpid;
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

    			case 3: 
 				 a.emplace_back(iter.name);
					 break;
			case 4: 
 				 a.emplace_back(iter.depart_code);
					 break;
			case 9: 
 				 a.emplace_back(iter.memo);
					 break;
			case 13: 
 				 a.emplace_back(iter.linkdpid);
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
 				 a<<std::to_string(iter.dpid);
				 break;
			case 1: 
 				 a<<std::to_string(iter.userid);
				 break;
			case 2: 
 				 a<<std::to_string(iter.parentid);
				 break;
			case 3: 
 				 if(isyinhao){ a<<jsonaddslash(iter.name); 
				 }else{
				 a<<iter.name;
				 }
				 break;
			case 4: 
 				 if(isyinhao){ a<<jsonaddslash(iter.depart_code); 
				 }else{
				 a<<iter.depart_code;
				 }
				 break;
			case 5: 
 				 a<<std::to_string(iter.bianzhi_num);
				 break;
			case 6: 
 				 a<<std::to_string(iter.real_num);
				 break;
			case 7: 
 				 a<<std::to_string(iter.quan_weight);
				 break;
			case 8: 
 				 a<<std::to_string(iter.isopen);
				 break;
			case 9: 
 				 if(isyinhao){ a<<jsonaddslash(iter.memo); 
				 }else{
				 a<<iter.memo;
				 }
				 break;
			case 10: 
 				 a<<std::to_string(iter.addtime);
				 break;
			case 11: 
 				 a<<std::to_string(iter.edittime);
				 break;
			case 12: 
 				 a<<std::to_string(iter.isvirtual);
				 break;
			case 13: 
 				 if(isyinhao){ a<<jsonaddslash(iter.linkdpid); 
				 }else{
				 a<<iter.linkdpid;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
				 } 
			switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
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
 
       			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
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
 	 ktemp=iter.dpid;
	 break;
case 1: 
 	 ktemp=iter.userid;
	 break;
case 2: 
 	 ktemp=iter.parentid;
	 break;
case 5: 
 	 ktemp=iter.bianzhi_num;
	 break;
case 6: 
 	 ktemp=iter.real_num;
	 break;
case 7: 
 	 ktemp=iter.quan_weight;
	 break;
case 8: 
 	 ktemp=iter.isopen;
	 break;
case 10: 
 	 ktemp=iter.addtime;
	 break;
case 11: 
 	 ktemp=iter.edittime;
	 break;
case 12: 
 	 ktemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }
 			switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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
 				 a.emplace(iter.dpid,iter);
				 break;
			case 1: 
 				 a.emplace(iter.userid,iter);
				 break;
			case 2: 
 				 a.emplace(iter.parentid,iter);
				 break;
			case 5: 
 				 a.emplace(iter.bianzhi_num,iter);
				 break;
			case 6: 
 				 a.emplace(iter.real_num,iter);
				 break;
			case 7: 
 				 a.emplace(iter.quan_weight,iter);
				 break;
			case 8: 
 				 a.emplace(iter.isopen,iter);
				 break;
			case 10: 
 				 a.emplace(iter.addtime,iter);
				 break;
			case 11: 
 				 a.emplace(iter.edittime,iter);
				 break;
			case 12: 
 				 a.emplace(iter.isvirtual,iter);
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

   			case 3: 
 				 a.emplace(iter.name,iter);
			 break;
			case 4: 
 				 a.emplace(iter.depart_code,iter);
			 break;
			case 9: 
 				 a.emplace(iter.memo,iter);
			 break;
			case 13: 
 				 a.emplace(iter.linkdpid,iter);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
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
 				 ktemp=iter.dpid;
			 break;
			case 1: 
 				 ktemp=iter.userid;
			 break;
			case 2: 
 				 ktemp=iter.parentid;
			 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
			 break;
			case 6: 
 				 ktemp=iter.real_num;
			 break;
			case 7: 
 				 ktemp=iter.quan_weight;
			 break;
			case 8: 
 				 ktemp=iter.isopen;
			 break;
			case 10: 
 				 ktemp=iter.addtime;
			 break;
			case 11: 
 				 ktemp=iter.edittime;
			 break;
			case 12: 
 				 ktemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }
 			switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }
 			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }
			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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

   case 3: 
 	 ktemp=iter.name;
	 break;
case 4: 
 	 ktemp=iter.depart_code;
	 break;
case 9: 
 	 ktemp=iter.memo;
	 break;
case 13: 
 	 ktemp=iter.linkdpid;
	 break;
	  }
 switch(vpos){
case 3: 
 	 vtemp=iter.name;
	 break;
case 4: 
 	 vtemp=iter.depart_code;
	 break;
case 9: 
 	 vtemp=iter.memo;
	 break;
case 13: 
 	 vtemp=iter.linkdpid;
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
 	 a.emplace_back(iter.dpid,iter);
	 break;
case 1: 
 	 a.emplace_back(iter.userid,iter);
	 break;
case 2: 
 	 a.emplace_back(iter.parentid,iter);
	 break;
case 5: 
 	 a.emplace_back(iter.bianzhi_num,iter);
	 break;
case 6: 
 	 a.emplace_back(iter.real_num,iter);
	 break;
case 7: 
 	 a.emplace_back(iter.quan_weight,iter);
	 break;
case 8: 
 	 a.emplace_back(iter.isopen,iter);
	 break;
case 10: 
 	 a.emplace_back(iter.addtime,iter);
	 break;
case 11: 
 	 a.emplace_back(iter.edittime,iter);
	 break;
case 12: 
 	 a.emplace_back(iter.isvirtual,iter);
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

   case 3: 
 	 a.emplace_back(iter.name,iter);
	 break;
case 4: 
 	 a.emplace_back(iter.depart_code,iter);
	 break;
case 9: 
 	 a.emplace_back(iter.memo,iter);
	 break;
case 13: 
 	 a.emplace_back(iter.linkdpid,iter);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
				 break;
			  }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.isvirtual);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
				  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
				 break;
			 }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.linkdpid);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.isvirtual);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.linkdpid);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.isvirtual);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			 }

			switch(vpos){
			case 0: 
 				 vtemp=iter.dpid;
				 break;
			case 1: 
 				 vtemp=iter.userid;
				 break;
			case 2: 
 				 vtemp=iter.parentid;
				 break;
			case 5: 
 				 vtemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 vtemp=iter.real_num;
				 break;
			case 7: 
 				 vtemp=iter.quan_weight;
				 break;
			case 8: 
 				 vtemp=iter.isopen;
				 break;
			case 10: 
 				 vtemp=iter.addtime;
				 break;
			case 11: 
 				 vtemp=iter.edittime;
				 break;
			case 12: 
 				 vtemp=iter.isvirtual;
				 break;
			 }

			switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.linkdpid);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
				 break;
			 }

			 switch(dpos){
			case 0: 
 				 a[ktemp][vtemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp][vtemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp][vtemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp][vtemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp][vtemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp][vtemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp][vtemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp][vtemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp][vtemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp][vtemp].emplace_back(iter.isvirtual);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 vtemp=iter.name;
				 break;
			case 4: 
 				 vtemp=iter.depart_code;
				 break;
			case 9: 
 				 vtemp=iter.memo;
				 break;
			case 13: 
 				 vtemp=iter.linkdpid;
				 break;
			  }

			 switch(dpos){
			case 3: 
 				 a[ktemp][vtemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp][vtemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp][vtemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp][vtemp].emplace_back(iter.linkdpid);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.linkdpid);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			  }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.isvirtual);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp].emplace_back(iter.name);
				 break;
			case 4: 
 				 a[ktemp].emplace_back(iter.depart_code);
				 break;
			case 9: 
 				 a[ktemp].emplace_back(iter.memo);
				 break;
			case 13: 
 				 a[ktemp].emplace_back(iter.linkdpid);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp].emplace_back(iter.dpid);
				 break;
			case 1: 
 				 a[ktemp].emplace_back(iter.userid);
				 break;
			case 2: 
 				 a[ktemp].emplace_back(iter.parentid);
				 break;
			case 5: 
 				 a[ktemp].emplace_back(iter.bianzhi_num);
				 break;
			case 6: 
 				 a[ktemp].emplace_back(iter.real_num);
				 break;
			case 7: 
 				 a[ktemp].emplace_back(iter.quan_weight);
				 break;
			case 8: 
 				 a[ktemp].emplace_back(iter.isopen);
				 break;
			case 10: 
 				 a[ktemp].emplace_back(iter.addtime);
				 break;
			case 11: 
 				 a[ktemp].emplace_back(iter.edittime);
				 break;
			case 12: 
 				 a[ktemp].emplace_back(iter.isvirtual);
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
 				 a[iter.dpid].emplace_back(iter);
				 break;
			case 1: 
 				 a[iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[iter.bianzhi_num].emplace_back(iter);
				 break;
			case 6: 
 				 a[iter.real_num].emplace_back(iter);
				 break;
			case 7: 
 				 a[iter.quan_weight].emplace_back(iter);
				 break;
			case 8: 
 				 a[iter.isopen].emplace_back(iter);
				 break;
			case 10: 
 				 a[iter.addtime].emplace_back(iter);
				 break;
			case 11: 
 				 a[iter.edittime].emplace_back(iter);
				 break;
			case 12: 
 				 a[iter.isvirtual].emplace_back(iter);
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

   			case 3: 
 				 a[iter.name].emplace_back(iter);
				 break;
			case 4: 
 				 a[iter.depart_code].emplace_back(iter);
				 break;
			case 9: 
 				 a[iter.memo].emplace_back(iter);
				 break;
			case 13: 
 				 a[iter.linkdpid].emplace_back(iter);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
			 }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.depart_code].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.memo].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.linkdpid].emplace_back(iter);
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

   			case 3: 
 				 ktemp=iter.name;
				 break;
			case 4: 
 				 ktemp=iter.depart_code;
				 break;
			case 9: 
 				 ktemp=iter.memo;
				 break;
			case 13: 
 				 ktemp=iter.linkdpid;
				 break;
	  }

 switch(vpos){
			case 0: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.bianzhi_num].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.real_num].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.quan_weight].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.edittime].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.isvirtual].emplace_back(iter);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			 }

			 switch(vpos){
			case 0: 
 				 a[ktemp][iter.dpid].emplace_back(iter);
				 break;
			case 1: 
 				 a[ktemp][iter.userid].emplace_back(iter);
				 break;
			case 2: 
 				 a[ktemp][iter.parentid].emplace_back(iter);
				 break;
			case 5: 
 				 a[ktemp][iter.bianzhi_num].emplace_back(iter);
				 break;
			case 6: 
 				 a[ktemp][iter.real_num].emplace_back(iter);
				 break;
			case 7: 
 				 a[ktemp][iter.quan_weight].emplace_back(iter);
				 break;
			case 8: 
 				 a[ktemp][iter.isopen].emplace_back(iter);
				 break;
			case 10: 
 				 a[ktemp][iter.addtime].emplace_back(iter);
				 break;
			case 11: 
 				 a[ktemp][iter.edittime].emplace_back(iter);
				 break;
			case 12: 
 				 a[ktemp][iter.isvirtual].emplace_back(iter);
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
 				 ktemp=iter.dpid;
				 break;
			case 1: 
 				 ktemp=iter.userid;
				 break;
			case 2: 
 				 ktemp=iter.parentid;
				 break;
			case 5: 
 				 ktemp=iter.bianzhi_num;
				 break;
			case 6: 
 				 ktemp=iter.real_num;
				 break;
			case 7: 
 				 ktemp=iter.quan_weight;
				 break;
			case 8: 
 				 ktemp=iter.isopen;
				 break;
			case 10: 
 				 ktemp=iter.addtime;
				 break;
			case 11: 
 				 ktemp=iter.edittime;
				 break;
			case 12: 
 				 ktemp=iter.isvirtual;
				 break;
			  }

			 switch(vpos){
			case 3: 
 				 a[ktemp][iter.name].emplace_back(iter);
				 break;
			case 4: 
 				 a[ktemp][iter.depart_code].emplace_back(iter);
				 break;
			case 9: 
 				 a[ktemp][iter.memo].emplace_back(iter);
				 break;
			case 13: 
 				 a[ktemp][iter.linkdpid].emplace_back(iter);
				 break;

                }
            }       
    
            return a;
        }
    
  };
    
} 
}
#endif
   