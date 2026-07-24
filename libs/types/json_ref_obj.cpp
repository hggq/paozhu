#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "types/json_ref_obj.h"
#include "json_reflect_headers.h"
#include "unicode.h"
#include "func.h"
//This file is automatically created, do not edit it

namespace json
{

static unsigned int _paozhu_json_skip_value(const std::string &_json_data, unsigned int _offset)
{
    _offset = http::json_string_trim(_json_data, _offset);
    if (_offset >= _json_data.size())
    {
        return _offset;
    }
    char _c = _json_data[_offset];
    if (_c == '"')
    {
        _offset += 1;
        for (; _offset < _json_data.size(); _offset++)
        {
            if (_json_data[_offset] == 0x5C)
            {
                _offset += 1;
                continue;
            }
            if (_json_data[_offset] == '"')
            {
                _offset += 1;
                break;
            }
        }
        return _offset;
    }
    if (_c == '{' || _c == '[')
    {
        int _depth = 0;
        bool _instr = false;
        for (; _offset < _json_data.size(); _offset++)
        {
            char _ch = _json_data[_offset];
            if (_instr)
            {
                if (_ch == 0x5C)
                {
                    _offset += 1;
                    continue;
                }
                if (_ch == '"')
                {
                    _instr = false;
                }
                continue;
            }
            if (_ch == '"')
            {
                _instr = true;
                continue;
            }
            if (_ch == '{' || _ch == '[')
            {
                _depth += 1;
            }
            else if (_ch == '}' || _ch == ']')
            {
                _depth -= 1;
                if (_depth <= 0)
                {
                    _offset += 1;
                    break;
                }
            }
        }
        return _offset;
    }
    for (; _offset < _json_data.size(); _offset++)
    {
        char _ch = _json_data[_offset];
        if (_ch == ',' || _ch == '}' || _ch == ']' || _ch == 0x20 || _ch == 0x0A || _ch == 0x0D || _ch == '\t')
        {
            break;
        }
    }
    return _offset;
}


std::string json_encode(const json_ref_obj_recent_orders_t &json_reflectobj)
	{
 
    std::stringstream _stream;
    _stream << "{"; 
    _stream << "\"id\":\"" << http::utf8_to_jsonstring(json_reflectobj.id)<< "\"";
                	_stream <<",";
	_stream << "\"amount\":";
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<json_reflectobj.amount;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
	_stream <<",";
	_stream << "\"items\":[";
      for (unsigned int i = 0; i <json_reflectobj.items.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<"\""<<http::utf8_to_jsonstring(json_reflectobj.items[i])<<"\"";
      }
      _stream << "]";
        	_stream <<",";
	_stream << "\"remark\":\"" << http::utf8_to_jsonstring(json_reflectobj.remark)<< "\"";
                
    _stream << "}";

    return _stream.str();
            
	}


std::string json_encode(const std::vector<json_ref_obj_recent_orders_t> &json_reflectobj)
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
              
     unsigned int json_decode(json_ref_obj_recent_orders_t &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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

                    
                            if (http::str_casecmp(_json_key_name, "items"))
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
                
                                    json_reflectobj.items.push_back(_json_value_name);  
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
                    if (http::str_casecmp(_json_key_name, "id"))
                            {

                                json_reflectobj.id=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "amount"))
                            {
                                try{
                                    json_reflectobj.amount=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.amount=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "remark"))
                            {

                                json_reflectobj.remark=_json_value_name;     
                            }
                            
                    ////////////////////////////////////////////////////////
                    //继续循环下一个键值
                    continue;
                }
            }
        }
        return _offset;
    }
  
unsigned int json_decode(std::vector<json_ref_obj_recent_orders_t> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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
                    json_ref_obj_recent_orders_t temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            json_ref_obj_recent_orders_t temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         

std::string json_encode(const json_ref_obj &json_reflectobj)
	{
 
    std::stringstream _stream;
    _stream << "{"; 
    _stream << "\"status\":\"" << http::utf8_to_jsonstring(json_reflectobj.status)<< "\"";
                	_stream <<",";
	_stream << "\"code\":" << std::to_string(json_reflectobj.code);
                	_stream <<",";
	_stream << "\"api_version\":\"" << http::utf8_to_jsonstring(json_reflectobj.api_version)<< "\"";
                	_stream <<",";
	_stream <<"\"config\":"; 
    _stream << "{"; 
    _stream << "\"debug\":" << (json_reflectobj.config.debug ? "true" : "false");
                	_stream <<",";
	_stream << "\"timeout\":";
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<json_reflectobj.config.timeout;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
	_stream <<",";
	_stream << "\"retry\":" << std::to_string(json_reflectobj.config.retry);
                	_stream <<",";
	_stream << "\"nullable_field\":" << (json_reflectobj.config.nullable_field.empty() ? std::string("null") : json_reflectobj.config.nullable_field);
                 
            _stream << "}";
                	_stream <<",";
	_stream <<"\"user\":"; 
    _stream << "{"; 
    _stream << "\"id\":" << std::to_string(json_reflectobj.user.id);
                	_stream <<",";
	_stream <<"\"name\":"; 
    _stream << "{"; 
    _stream << "\"zh\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.name.zh)<< "\"";
                	_stream <<",";
	_stream << "\"en\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.name.en)<< "\"";
                	_stream <<",";
	_stream << "\"ar\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.name.ar)<< "\"";
                 
            _stream << "}";
                	_stream <<",";
	_stream << "\"bio\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.bio)<< "\"";
                	_stream <<",";
	_stream <<"\"address\":"; 
    _stream << "{"; 
    _stream << "\"street\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.address.street)<< "\"";
                	_stream <<",";
	_stream << "\"city\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.address.city)<< "\"";
                	_stream <<",";
	_stream <<"\"geo\":"; 
    _stream << "{"; 
    _stream << "\"lat\":";
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<json_reflectobj.user.address.geo.lat;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
	_stream <<",";
	_stream << "\"lng\":";
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<json_reflectobj.user.address.geo.lng;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
 
            _stream << "}";
                 
            _stream << "}";
                	_stream <<",";
	_stream << "\"tags\":[";
      for (unsigned int i = 0; i <json_reflectobj.user.tags.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<"\""<<http::utf8_to_jsonstring(json_reflectobj.user.tags[i])<<"\"";
      }
      _stream << "]";
        	_stream <<",";
	_stream <<"\"settings\":"; 
    _stream << "{"; 
    _stream << "\"notifications\":" << (json_reflectobj.user.settings.notifications ? "true" : "false");
                	_stream <<",";
	_stream << "\"language\":\"" << http::utf8_to_jsonstring(json_reflectobj.user.settings.language)<< "\"";
                 
            _stream << "}";
                 
            _stream << "}";
                	_stream <<",";
	_stream << "\"recent_orders\":[";
      for (unsigned int i = 0; i <json_reflectobj.recent_orders.size(); i++)
      {
        if (i > 0)
        {
          _stream << ",";
        }
        _stream <<json_encode(json_reflectobj.recent_orders[i]);
      }
      _stream << "]";
        	_stream <<",";
	_stream <<"\"escape_sequence_test\":"; 
    _stream << "{"; 
    _stream << "\"quotes\":\"" << http::utf8_to_jsonstring(json_reflectobj.escape_sequence_test.quotes)<< "\"";
                	_stream <<",";
	_stream << "\"backslash\":\"" << http::utf8_to_jsonstring(json_reflectobj.escape_sequence_test.backslash)<< "\"";
                	_stream <<",";
	_stream << "\"json_boundary\":\"" << http::utf8_to_jsonstring(json_reflectobj.escape_sequence_test.json_boundary)<< "\"";
                	_stream <<",";
	_stream << "\"unicode_emoji_raw\":\"" << http::utf8_to_jsonstring(json_reflectobj.escape_sequence_test.unicode_emoji_raw)<< "\"";
                 
            _stream << "}";
                	_stream <<",";
	_stream << "\"nested_arrays_test\":" << (json_reflectobj.nested_arrays_test.empty() ? std::string("null") : json_reflectobj.nested_arrays_test);
                	_stream <<",";
	_stream <<"\"metadata\":"; 
    _stream << "{"; 
    _stream << "\"generated_at\":\"" << http::utf8_to_jsonstring(json_reflectobj.metadata.generated_at)<< "\"";
                	_stream <<",";
	_stream << "\"source\":\"" << http::utf8_to_jsonstring(json_reflectobj.metadata.source)<< "\"";
                	_stream <<",";
	_stream << "\"rnd_seed\":";
        
        {
            std::string temp;
            std::stringstream _floatstream;
            _floatstream.precision(15);
            _floatstream <<json_reflectobj.metadata.rnd_seed;
            temp = _floatstream.str();
            unsigned int i = 0;
            for (; i < temp.length(); i++)
            {
                if (temp[i] == '.')
                {
                break;
                }
            }
            if (i == temp.length())
            {
                temp.append(".0");
            }
            _stream<<temp;
        }
        
 
            _stream << "}";
                
    _stream << "}";

    return _stream.str();
            
	}


std::string json_encode(const std::vector<json_ref_obj> &json_reflectobj)
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
              
     unsigned int json_decode(json_ref_obj &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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
                    
                    
                    if (http::str_casecmp(_json_key_name, "nested_arrays_test"))
                    {
                        unsigned int _raw_begin=_offset;
                        unsigned int _raw_end=_paozhu_json_skip_value(_json_data,_offset);
                        if(_raw_end>_raw_begin && _raw_end<=_json_data.size())
                        {
                            json_reflectobj.nested_arrays_test.assign(_json_data,_raw_begin,_raw_end-_raw_begin);
                        }
                        _offset=_raw_end;
                        if(_offset>0){_offset-=1;}
                        continue;
                    }
                    
                    if(_offset < _json_data.size() &&_json_data[_offset]=='{')
                    {   //还是一个对象，表示有嵌套对象
                        //1 内置 struct map<std::string,*>
                        //递归代码
                    
                            if (http::str_casecmp(_json_key_name, "config"))
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

                    
                    if (http::str_casecmp(_json_key_name, "nullable_field"))
                    {
                        unsigned int _raw_begin=_offset;
                        unsigned int _raw_end=_paozhu_json_skip_value(_json_data,_offset);
                        if(_raw_end>_raw_begin && _raw_end<=_json_data.size())
                        {
                            json_reflectobj.config.nullable_field.assign(_json_data,_raw_begin,_raw_end-_raw_begin);
                        }
                        _offset=_raw_end;
                        if(_offset>0){_offset-=1;}
                        continue;
                    }
                    
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
                    if (http::str_casecmp(_json_key_name, "debug"))
                            {
                                json_reflectobj.config.debug = (_json_value_name=="true"||_json_value_name=="1"||_json_value_name=="TRUE"||_json_value_name=="True");     
                            }
                            			else if (http::str_casecmp(_json_key_name, "timeout"))
                            {
                                try{
                                    json_reflectobj.config.timeout=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.config.timeout=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "retry"))
                            {
                                try{
                                    json_reflectobj.config.retry=std::stoll(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.config.retry=0;
                                }
                                 
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                            
                            if (http::str_casecmp(_json_key_name, "user"))
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
                    
                            if (http::str_casecmp(_json_key_name, "name"))
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
                    if (http::str_casecmp(_json_key_name, "zh"))
                            {

                                json_reflectobj.user.name.zh=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "en"))
                            {

                                json_reflectobj.user.name.en=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "ar"))
                            {

                                json_reflectobj.user.name.ar=_json_value_name;     
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                            
                            if (http::str_casecmp(_json_key_name, "address"))
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
                    
                            if (http::str_casecmp(_json_key_name, "geo"))
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
                    if (http::str_casecmp(_json_key_name, "lat"))
                            {
                                try{
                                    json_reflectobj.user.address.geo.lat=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.user.address.geo.lat=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "lng"))
                            {
                                try{
                                    json_reflectobj.user.address.geo.lng=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.user.address.geo.lng=0;
                                }
                                 
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

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
                    if (http::str_casecmp(_json_key_name, "street"))
                            {

                                json_reflectobj.user.address.street=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "city"))
                            {

                                json_reflectobj.user.address.city=_json_value_name;     
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                            
                            if (http::str_casecmp(_json_key_name, "settings"))
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
                    if (http::str_casecmp(_json_key_name, "notifications"))
                            {
                                json_reflectobj.user.settings.notifications = (_json_value_name=="true"||_json_value_name=="1"||_json_value_name=="TRUE"||_json_value_name=="True");     
                            }
                            			else if (http::str_casecmp(_json_key_name, "language"))
                            {

                                json_reflectobj.user.settings.language=_json_value_name;     
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

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
                    
                            if (http::str_casecmp(_json_key_name, "tags"))
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
                
                                    json_reflectobj.user.tags.push_back(_json_value_name);  
                /////////////////////////////
                continue;   
            }
             
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
                    if (http::str_casecmp(_json_key_name, "id"))
                            {
                                try{
                                    json_reflectobj.user.id=std::stoll(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.user.id=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "bio"))
                            {

                                json_reflectobj.user.bio=_json_value_name;     
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                            
                            if (http::str_casecmp(_json_key_name, "escape_sequence_test"))
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
                    if (http::str_casecmp(_json_key_name, "quotes"))
                            {

                                json_reflectobj.escape_sequence_test.quotes=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "backslash"))
                            {

                                json_reflectobj.escape_sequence_test.backslash=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "json_boundary"))
                            {

                                json_reflectobj.escape_sequence_test.json_boundary=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "unicode_emoji_raw"))
                            {

                                json_reflectobj.escape_sequence_test.unicode_emoji_raw=_json_value_name;     
                            }
                              
                    //直接下一个，不用处理键值
                    continue;   

                }
            }

                 
                            }
                            
                            if (http::str_casecmp(_json_key_name, "metadata"))
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
                    if (http::str_casecmp(_json_key_name, "generated_at"))
                            {

                                json_reflectobj.metadata.generated_at=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "source"))
                            {

                                json_reflectobj.metadata.source=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "rnd_seed"))
                            {
                                try{
                                    json_reflectobj.metadata.rnd_seed=std::stod(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.metadata.rnd_seed=0;
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

                    
                            if (http::str_casecmp(_json_key_name, "recent_orders"))
                            {
                                _offset=json_decode(json_reflectobj.recent_orders,_json_data,_offset);
                                 
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
                    if (http::str_casecmp(_json_key_name, "status"))
                            {

                                json_reflectobj.status=_json_value_name;     
                            }
                            			else if (http::str_casecmp(_json_key_name, "code"))
                            {
                                try{
                                    json_reflectobj.code=std::stoll(_json_value_name);
                                }catch (...) 
                                { 
                                    json_reflectobj.code=0;
                                }
                                 
                            }
                            			else if (http::str_casecmp(_json_key_name, "api_version"))
                            {

                                json_reflectobj.api_version=_json_value_name;     
                            }
                            
                    ////////////////////////////////////////////////////////
                    //继续循环下一个键值
                    continue;
                }
            }
        }
        return _offset;
    }
  
unsigned int json_decode(std::vector<json_ref_obj> &json_reflectobj,const std::string &_json_data,unsigned int _offset)
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
                    json_ref_obj temp;
                    _offset=json_decode(temp,_json_data,_offset);
                    json_reflectobj.push_back(temp);
                }

            }
            
        }
        else
        {
            json_ref_obj temp;
            _offset=json_decode(temp,_json_data,_offset);
            json_reflectobj.push_back(temp);
            
        }

        return _offset; 
    }
         
}
