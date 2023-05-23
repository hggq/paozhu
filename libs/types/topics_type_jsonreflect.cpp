
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "types/topics_type.h"
#include "json_reflect_headers.h"
#include "unicode.h"


namespace psy
{


std::string json_encode(const topics_tree_outjson_t &json_reflectobj)
	{
 
    std::stringstream _stream;
    _stream << "{"; 
    _stream << "\"id\":" << std::to_string(json_reflectobj.id);
                	_stream <<",";
	_stream << "\"parentid\":" << std::to_string(json_reflectobj.parentid);
                	_stream <<",";
	_stream << "\"cateid\":" << std::to_string(json_reflectobj.cateid);
                	_stream <<",";
	_stream << "\"value\":\"" << http::utf8_to_jsonstring(json_reflectobj.value)<< "\"";
                	_stream <<",";
	_stream << "\"urlpath\":\"" << http::utf8_to_jsonstring(json_reflectobj.urlpath)<< "\"";
                	_stream <<",";
	_stream << "\"isview\":" << (json_reflectobj.isview ? "true" : "false");
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


std::string json_encode(const std::vector<topics_tree_outjson_t> &json_reflectobj)
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
              
     unsigned int json_decode(topics_tree_outjson_t &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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

                    
                            if (strcasecmp(_json_key_name.c_str(), "children") == 0)
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
                    if (strcasecmp(_json_key_name.c_str(), "id") == 0)
                            {
                                try{
                                    json_reflectobj.id=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.id=0;
                                }
                                 
                            }
                            else if (strcasecmp(_json_key_name.c_str(), "parentid") == 0)
                            {
                                try{
                                    json_reflectobj.parentid=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.parentid=0;
                                }
                                 
                            }
                            else if (strcasecmp(_json_key_name.c_str(), "cateid") == 0)
                            {
                                try{
                                    json_reflectobj.cateid=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.cateid=0;
                                }
                                 
                            }
                            			else if (strcasecmp(_json_key_name.c_str(), "value") == 0)
                            {

                                json_reflectobj.value=_json_value_name;     
                            }
                            			else if (strcasecmp(_json_key_name.c_str(), "urlpath") == 0)
                            {

                                json_reflectobj.urlpath=_json_value_name;     
                            }
                            else if (strcasecmp(_json_key_name.c_str(), "_level") == 0)
                            {
                                try{
                                    json_reflectobj._level=std::stoul(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj._level=0;
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
  
unsigned int json_decode(std::vector<topics_tree_outjson_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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
                    topics_tree_outjson_t temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            topics_tree_outjson_t temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         
}
