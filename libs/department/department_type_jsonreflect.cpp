#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "department/department_type.h"
#include "json_reflect_headers.h"
#include "unicode.h"
#include "func.h"

namespace psy
{


std::string json_encode(const department_outjson_t &json_reflectobj)
	{
 
    std::stringstream _stream;
    _stream << "{"; 
    _stream << "\"id\":" << std::to_string(json_reflectobj.id);
                	_stream <<",";
	_stream << "\"key\":" << std::to_string(json_reflectobj.key);
                	_stream <<",";
	_stream << "\"value\":" << std::to_string(json_reflectobj.value);
                	_stream <<",";
	_stream << "\"parentid\":" << std::to_string(json_reflectobj.parentid);
                	_stream <<",";
	_stream << "\"bianzhi_num\":" << std::to_string(json_reflectobj.bianzhi_num);
                	_stream <<",";
	_stream << "\"isopen\":" << (json_reflectobj.isopen ? "true" : "false");
                	_stream <<",";
	_stream << "\"title\":\"" << http::utf8_to_jsonstring(json_reflectobj.title)<< "\"";
                	_stream <<",";
	_stream << "\"desc\":\"" << http::utf8_to_jsonstring(json_reflectobj.desc)<< "\"";
                	_stream <<",";
	_stream << "\"children\":[";
      for (unsigned int i = 0; i <json_reflectobj.children.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<json_encode(json_reflectobj.children[i]);
      }
      _stream << "]";
        
    _stream << "}";

    return _stream.str();
            
	}


std::string json_encode(const std::vector<department_outjson_t> &json_reflectobj)
        {
        std::stringstream _stream;
        _stream << "["; 
    
            for(unsigned int i=0;i<json_reflectobj.size();i++) 
            {
                if(i>0)
                {
                    _stream <<",";
                }
                _stream <<json_encode(json_reflectobj[i]);
            }
    
        _stream << "]";

        return _stream.str();
        }
              
     unsigned int json_decode(department_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset)
     {           
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            for(;_offset<_json_data.size();_offset++)
            {
                if(_json_data[_offset]=='{')
                {
                    _isarray=false;
                    break;
                }   
            }
        }

        if(_isarray==false)
        {
            if(_json_data[_offset]=='{')
            {
                _offset++;
            }
            std::string _json_key_name,_json_value_name;
            for(;_offset<_json_data.size();_offset++)
            {

                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧括号表示这个对象已经结束
                if(_json_data[_offset]==0x7D)
                {
                    return _offset;
                }
                //直到引号
                if(_json_data[_offset]==0x22)
                {
                    unsigned int temp_offset=_offset;
                    _json_value_name.clear();
                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);

                    _offset=temp_offset;
                    if(_offset < _json_data.size() &&_json_data[_offset]==0x22)
                    {
                        _offset+=1;
                    }
                    //键名 后就是键值类型 循环去除空格
                    _offset=http::json_string_trim(_json_data,_offset);
                    if(_offset < _json_data.size() &&_json_data[_offset]!=':')
                    {
                        return _offset; 
                    }
                    _offset++;
                    _offset=http::json_string_trim(_json_data,_offset);
                    
                    if(_offset < _json_data.size() &&_json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                        //1 内置 struct map<std::string,*>
                        //递归代码
                    
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                                                

                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        //直接下一个，不用处理键值
                        continue;   
                    }
                    else if(_json_data[_offset]=='[')
                    {   //表示有数组 
                        //////////////////////////////////////////////////////////////////////
                        //begin level1 []
                        //vector<std::string> vector<std::pair<std::string,*>>   vector<vector<int|long|float|double>>
                        //如果是非内置类型 直接使用json_decode<>

                        //递归代码     

                    
                            if (http::str_casecmp(_json_key_name, "children"))
                            {
                                _offset=json_decode(json_reflectobj.children,_json_data,_offset);
                                 
                            }
                            
                        //直接下一个，不用处理键值
                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        continue;    
                        //end level1[]
                        ////////////////////////////////////////////////////////////////////
                    }
                    else if(_json_data[_offset]==0x22)
                    {
                        //如果键值也是字符串
                        temp_offset=_offset;
                        _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                        _offset=temp_offset;
                        if(_json_data[_offset]==0x22)
                        {
                            if((_offset+1)<_json_data.size())
                            {
                                if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                                {
                                    _offset+=1;
                                }
                            }
                        }
                    }
                    else
                    {
                        //表示是数字 bool NULL
                        for(;_offset<_json_data.size();_offset++)
                        {
                            //结束条件
                            if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                            {
                                break;
                            }
                            _json_value_name.push_back(_json_data[_offset]);
                        }
                        //让前面循环退出或返回
                        if(_offset < _json_data.size() && _json_data[_offset]=='}')
                        {
                            _offset-=1;
                        }
                    }
                    ////////////////////////////////////////////////////////
                    // level1
                    //处理对象赋值
                    if (http::str_casecmp(_json_key_name, "id"))
                            {
                                try{
                                    json_reflectobj.id=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.id=0;
                                }
                                 
                            }
                            else if (http::str_casecmp(_json_key_name, "key"))
                            {
                                try{
                                    json_reflectobj.key=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.key=0;
                                }
                                 
                            }
                            else if (http::str_casecmp(_json_key_name, "value"))
                            {
                                try{
                                    json_reflectobj.value=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.value=0;
                                }
                                 
                            }
                            else if (http::str_casecmp(_json_key_name, "parentid"))
                            {
                                try{
                                    json_reflectobj.parentid=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.parentid=0;
                                }
                                 
                            }
                            else if (http::str_casecmp(_json_key_name, "bianzhi_num"))
                            {
                                try{
                                    json_reflectobj.bianzhi_num=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.bianzhi_num=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "title"))
                            {

                                json_reflectobj.title=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "desc"))
                            {

                                json_reflectobj.desc=_json_value_name;     
                            }
                            
                    ////////////////////////////////////////////////////////
                    //继续循环下一个键值
                    continue;
                }
            }
        }
        return _offset;
    }
  
unsigned int json_decode(std::vector<department_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
      {          
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            if(_json_data[_offset]=='[')
            {
                _offset+=1;
            }
            for(;_offset<_json_data.size();_offset++)
            {
                _offset=http::json_string_trim(_json_data,_offset);
                //直接返回，这样可以防插入空的对象
                if(_json_data[_offset]==0x5D)
                {
                    return _offset;
                }else if(_json_data[_offset]=='{')
                {
                    department_outjson_t temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            department_outjson_t temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         

std::string json_encode(const department_listoutjson_t &json_reflectobj)
	{
 
    std::stringstream _stream;
    _stream << "{"; 
    _stream << "\"code\":" << std::to_string(json_reflectobj.code);
                	_stream <<",";
	_stream <<"\"data\":"; 
    _stream << "{"; 
    _stream << "\"list\":[";
      for (unsigned int i = 0; i <json_reflectobj.data.list.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<json_encode(json_reflectobj.data.list[i]);
      }
      _stream << "]";
        	_stream <<",";
	_stream << "\"total\":" << std::to_string(json_reflectobj.data.total);
                 
            _stream << "}";
                	_stream <<",";
	_stream << "\"names\":[";
      for (unsigned int i = 0; i <json_reflectobj.names.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream << "[";
         for (unsigned int j = 0; j <json_reflectobj.names[i].size(); i++)
       {
        if (j > 0)
        {
          _stream << ",";
        }
        _stream <<"\""<<http::utf8_to_jsonstring(json_reflectobj.names[i][j])<<"\"";
        }
        _stream << "]";
      }
      _stream << "]";
        
    _stream << "}";

    return _stream.str();
            
	}


std::string json_encode(const std::vector<department_listoutjson_t> &json_reflectobj)
        {
        std::stringstream _stream;
        _stream << "["; 
    
            for(unsigned int i=0;i<json_reflectobj.size();i++) 
            {
                if(i>0)
                {
                    _stream <<",";
                }
                _stream <<json_encode(json_reflectobj[i]);
            }
    
        _stream << "]";

        return _stream.str();
        }
              
     unsigned int json_decode(department_listoutjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset)
     {           
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            for(;_offset<_json_data.size();_offset++)
            {
                if(_json_data[_offset]=='{')
                {
                    _isarray=false;
                    break;
                }   
            }
        }

        if(_isarray==false)
        {
            if(_json_data[_offset]=='{')
            {
                _offset++;
            }
            std::string _json_key_name,_json_value_name;
            for(;_offset<_json_data.size();_offset++)
            {

                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧括号表示这个对象已经结束
                if(_json_data[_offset]==0x7D)
                {
                    return _offset;
                }
                //直到引号
                if(_json_data[_offset]==0x22)
                {
                    unsigned int temp_offset=_offset;
                    _json_value_name.clear();
                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);

                    _offset=temp_offset;
                    if(_offset < _json_data.size() &&_json_data[_offset]==0x22)
                    {
                        _offset+=1;
                    }
                    //键名 后就是键值类型 循环去除空格
                    _offset=http::json_string_trim(_json_data,_offset);
                    if(_offset < _json_data.size() &&_json_data[_offset]!=':')
                    {
                        return _offset; 
                    }
                    _offset++;
                    _offset=http::json_string_trim(_json_data,_offset);
                    
                    if(_offset < _json_data.size() &&_json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                        //1 内置 struct map<std::string,*>
                        //递归代码
                    
                            if (http::str_casecmp(_json_key_name, "data"))
                            {  

            for(;_offset<_json_data.size();_offset++)
            {
                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧括号表示这个对象已经结束
                if(_json_data[_offset]==0x7D)
                {
                    _offset+=1;
                    break;
                }

                //直到引号
                if(_json_data[_offset]==0x22)
                {
                    unsigned int temp_offset=_offset;
                    _json_value_name.clear();
                    _json_key_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                    _offset=temp_offset;
                    if(_json_data[_offset]==0x22)
                    {
                        if((_offset+1)<_json_data.size())
                        {
                            if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                            {
                                _offset+=1;
                            }
                        }
                    }
                    //键名 后就是键值类型 循环去除空格
                    _offset=http::json_string_trim(_json_data,_offset);
                    if(_offset < _json_data.size() && _json_data[_offset]!=':')
                    {
                        return _offset; 
                    }
                    _offset++;
                    _offset=http::json_string_trim(_json_data,_offset);

                    if(_offset < _json_data.size() && _json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                    
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        
                        if( _offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }  
                        continue;    
                    }
                    else if(_json_data[_offset]=='[')
                    {   //表示有数组 
                    
                            if (http::str_casecmp(_json_key_name, "list"))
                            {
                                _offset=json_decode(json_reflectobj.data.list,_json_data,_offset);
                                 
                            }
                              
                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        //直接下一个，不用处理键值
                        continue;    
                    }
                    else if(_json_data[_offset]==0x22)
                    {
                        //如果键值也是字符串
                        temp_offset=_offset;
                        _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                        _offset=temp_offset;
                        if(_json_data[_offset]==0x22)
                        {
                            if((_offset+1)<_json_data.size())
                            {
                                if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                                {
                                    _offset+=1;
                                }
                            }
                        }
                    }
                    else
                    {
                        //表示是数字 bool NULL
                        for(;_offset<_json_data.size();_offset++)
                        {
                            //结束条件
                            if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                            {
                                break;
                            }
                            _json_value_name.push_back(_json_data[_offset]);
                        }
                        //让前面循环退出或返回 主要与数组不同这里是大括号
                        if(_offset < _json_data.size() && _json_data[_offset]=='}')
                        {
                            _offset-=1;
                        }
                    }
                    if (http::str_casecmp(_json_key_name, "total"))
                            {
                                try{
                                    json_reflectobj.data.total=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.data.total=0;
                                }
                                 
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                                                    

                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        //直接下一个，不用处理键值
                        continue;   
                    }
                    else if(_json_data[_offset]=='[')
                    {   //表示有数组 
                        //////////////////////////////////////////////////////////////////////
                        //begin level1 []
                        //vector<std::string> vector<std::pair<std::string,*>>   vector<vector<int|long|float|double>>
                        //如果是非内置类型 直接使用json_decode<>

                        //递归代码     

                    
                            if (http::str_casecmp(_json_key_name, "names"))
                            {  
            if(_json_data[_offset]==0x5B)
            {
                _offset+=1;
            }

            for(;_offset<_json_data.size();_offset++)
            {
                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧中括号表示这个对象已经结束
                if(_json_data[_offset]==0x5D)
                {
                    _offset+=1;
                    break;
                }
                //直到引号
                if(_json_data[_offset]=='{')
                {   //还是一个对象，表示有嵌套对象
                    //_offset=psy::json_decode(_json_data,_offset);
                    //直接下一个，不用处理键值
                    //需要动态生成 std::stol bool string
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                    
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                          
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]=='[')
                {   //表示有数组 
                    ///////////////////////////////////////////////////////
                 std::vector<std::string> a;
                                    json_reflectobj.names.push_back(a);
                        unsigned int temp_size=json_reflectobj.names.size();
                        temp_size-=1;
                          
            if(_json_data[_offset]==0x5B)
            {
                _offset+=1;
            }

            for(;_offset<_json_data.size();_offset++)
            {
                //去除空格
                _offset=http::json_string_trim(_json_data,_offset);
                //如果是右侧中括号表示这个对象已经结束
                if(_json_data[_offset]==0x5D)
                {
                    _offset+=1;
                    break;
                }
                //直到引号
                if(_json_data[_offset]=='{')
                {   //还是一个对象，表示有嵌套对象
                    //_offset=psy::json_decode(_json_data,_offset);
                    //直接下一个，不用处理键值
                    //需要动态生成 std::stol bool string
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                    
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == '}')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                          
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]=='[')
                {   //表示有数组 
                    ///////////////////////////////////////////////////////
                
                            _offset++;
                            for ( ; _offset < _json_data.size(); _offset++)
                            {
                                if (_json_data[_offset] == ']')
                                {
                                    //offset++;
                                    break;
                                }
                                if (_json_data[_offset] == '"')
                                {
                                    _offset++;
                                    for ( ; _offset < _json_data.size(); _offset++)
                                    {
                                        if (_json_data[_offset] == '"'&&_json_data[_offset-1]!=0x5C)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                          
                    ///////////////////////////////////////////////////////
                    //直接下一个，不用处理键值
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]==0x22)
                {
                    //如果键值也是字符串
                    temp_offset=_offset;
                    _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                    _offset=temp_offset;
                    if(_json_data[_offset]==0x22)
                    {
                        if((_offset+1)<_json_data.size())
                        {
                            if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                            {
                                _offset+=1;
                            }
                        }
                        
                    }
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                }
                else
                {
                    //表示是数字 bool NULL
                    for(;_offset<_json_data.size();_offset++)
                    {
                        //结束条件
                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                        {
                            break;
                        }
                        _json_value_name.push_back(_json_data[_offset]);
                    }
                    //让前面循环退出或返回 主要与对象不同这里是中括号
                    if(_offset < _json_data.size() && _json_data[_offset]==']')
                    {
                        _offset-=1;
                    }
                }
                // 数字  bool std::string 列表
                //直接下一个，不用处理键值
                /////////////////////////////
                
                                    json_reflectobj.names[temp_size].push_back(_json_value_name);  
                /////////////////////////////
                continue;   
            }
          
                    ///////////////////////////////////////////////////////
                    //直接下一个，不用处理键值
                    if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                    {
                        _offset-=1;
                    }
                    continue;    
                }
                else if(_json_data[_offset]==0x22)
                {
                    //如果键值也是字符串
                    temp_offset=_offset;
                    _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                    _offset=temp_offset;
                    if(_json_data[_offset]==0x22)
                    {
                        if((_offset+1)<_json_data.size())
                        {
                            if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                            {
                                _offset+=1;
                            }
                        }
                        
                    }
                    //json_reflectobj.children.push_back(_json_key_name,_json_value_name);
                }
                else
                {
                    //表示是数字 bool NULL
                    for(;_offset<_json_data.size();_offset++)
                    {
                        //结束条件
                        if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                        {
                            break;
                        }
                        _json_value_name.push_back(_json_data[_offset]);
                    }
                    //让前面循环退出或返回 主要与对象不同这里是中括号
                    if(_offset < _json_data.size() && _json_data[_offset]==']')
                    {
                        _offset-=1;
                    }
                }
                // 数字  bool std::string 列表
                //直接下一个，不用处理键值
                /////////////////////////////
                  
                /////////////////////////////
                continue;   
            }
             
                            }
                            
                        //直接下一个，不用处理键值
                        if(_offset < _json_data.size() && (_json_data[_offset]==']'||_json_data[_offset]=='}'))
                        {
                            _offset-=1;
                        }
                        continue;    
                        //end level1[]
                        ////////////////////////////////////////////////////////////////////
                    }
                    else if(_json_data[_offset]==0x22)
                    {
                        //如果键值也是字符串
                        temp_offset=_offset;
                        _json_value_name=http::jsonstring_to_utf8(&_json_data[_offset],_json_data.size()-_offset,temp_offset);
                        _offset=temp_offset;
                        if(_json_data[_offset]==0x22)
                        {
                            if((_offset+1)<_json_data.size())
                            {
                                if(_json_data[_offset+1]!=']'&&_json_data[_offset+1]!='}')
                                {
                                    _offset+=1;
                                }
                            }
                        }
                    }
                    else
                    {
                        //表示是数字 bool NULL
                        for(;_offset<_json_data.size();_offset++)
                        {
                            //结束条件
                            if(_json_data[_offset]==','||_json_data[_offset]==']'||_json_data[_offset]=='}'||_json_data[_offset]==0x20||_json_data[_offset]==0x0A||_json_data[_offset]==0x0D||_json_data[_offset]=='\t')
                            {
                                break;
                            }
                            _json_value_name.push_back(_json_data[_offset]);
                        }
                        //让前面循环退出或返回
                        if(_offset < _json_data.size() && _json_data[_offset]=='}')
                        {
                            _offset-=1;
                        }
                    }
                    ////////////////////////////////////////////////////////
                    // level1
                    //处理对象赋值
                    if (http::str_casecmp(_json_key_name, "code"))
                            {
                                try{
                                    json_reflectobj.code=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.code=0;
                                }
                                 
                            }
                            
                    ////////////////////////////////////////////////////////
                    //继续循环下一个键值
                    continue;
                }
            }
        }
        return _offset;
    }
  
unsigned int json_decode(std::vector<department_listoutjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
      {          
             bool _isarray=false;
        for(;_offset<_json_data.size();_offset++)
        {
            if(_json_data[_offset]=='{')
            {
                break;
            }
            if(_json_data[_offset]=='[')
            {
                _isarray=true;
                break;
            }
        }

        if(_isarray)
        {
            if(_json_data[_offset]=='[')
            {
                _offset+=1;
            }
            for(;_offset<_json_data.size();_offset++)
            {
                _offset=http::json_string_trim(_json_data,_offset);
                //直接返回，这样可以防插入空的对象
                if(_json_data[_offset]==0x5D)
                {
                    return _offset;
                }else if(_json_data[_offset]=='{')
                {
                    department_listoutjson_t temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            department_listoutjson_t temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         
}
