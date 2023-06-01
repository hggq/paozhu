#include "json.h"
#include <cstddef>
#include <stdexcept>
#include <sstream>// ostringstream 类型
// #include <strstream>// ostrstream 类型
#include <string>
#include <cstdlib>
#include <charconv>
#include <cstring>
#include <utility>
#include <charconv>

namespace json
{

Value::Value() : type_t(NIL) {}

Value::Value(const long long int i) : int_v(i), type_t(INT) {}

Value::Value(const long int i) : int_v(static_cast<long long int>(i)), type_t(INT) {}
Value::Value(const unsigned long long i) : int_v(static_cast<unsigned long long>(i)), type_t(INT) {}

Value::Value(const int i) : int_v(static_cast<int>(i)), type_t(INT) {}
Value::Value(const unsigned int i) : int_v(static_cast<unsigned int>(i)), type_t(INT) {}
Value::Value(const long double f) : float_v(f), type_t(FLOAT) {}

Value::Value(const double f) : float_v(f), type_t(FLOAT) {}

Value::Value(const bool b) : bool_v(b), type_t(BOOL) {}

Value::Value(const char *s) : string_v(s), type_t(STRING) {}

Value::Value(const std::string &s) : string_v(s), type_t(STRING) {}

Value::Value(const Object &o) : object_v(o), type_t(OBJECT) {}

Value::Value(const Array &o) : array_v(o), type_t(ARRAY) {}

Value::Value(std::string &&s) : string_v(std::move(s)), type_t(STRING) {}

Value::Value(Object &&o) : object_v(std::move(o)), type_t(OBJECT) {}

Value::Value(Array &&o) : array_v(std::move(o)), type_t(ARRAY) {}
Value::Value(std::initializer_list<std::string> nsl)
{
    type_t = ARRAY;
    for (auto v : nsl)
    {
        Value vv(v);
        array_v.push_back(vv);
    }
}

Value::Value(const Value &v)
{
    switch (v.type())
    {
    /** Base types */
    case INT:
        int_v  = v.int_v;
        type_t = INT;
        break;

    case FLOAT:
        float_v = v.float_v;
        type_t  = FLOAT;
        break;

    case BOOL:
        bool_v = v.bool_v;
        type_t = BOOL;
        break;

    case NIL: type_t = NIL; break;

    case STRING:
        string_v = v.string_v;
        type_t   = STRING;
        break;

        /** Compound types */
    case ARRAY:
        array_v = v.array_v;
        type_t  = ARRAY;
        break;

    case OBJECT:
        object_v = v.object_v;
        type_t   = OBJECT;
        break;
    }
}

Value::Value(Value &&v)
{
    switch (v.type())
    {
    /** Base types */
    case INT:
        int_v  = std::move(v.int_v);
        type_t = INT;
        break;

    case FLOAT:
        float_v = std::move(v.float_v);
        type_t  = FLOAT;
        break;

    case BOOL:
        bool_v = std::move(v.bool_v);
        type_t = BOOL;
        break;

    case NIL: type_t = NIL; break;

    case STRING:
        string_v = std::move(v.string_v);
        type_t   = STRING;
        break;

        /** Compound types */
    case ARRAY:
        array_v = std::move(v.array_v);
        type_t  = ARRAY;
        break;

    case OBJECT:
        object_v = std::move(v.object_v);
        type_t   = OBJECT;
        break;
    }
}

Value &Value::operator=(const Value &v)
{
    switch (v.type())
    {
    /** Base types */
    case INT:
        int_v  = v.int_v;
        type_t = INT;
        break;

    case FLOAT:
        float_v = v.float_v;
        type_t  = FLOAT;
        break;

    case BOOL:
        bool_v = v.bool_v;
        type_t = BOOL;
        break;

    case NIL: type_t = NIL; break;

    case STRING:
        string_v = v.string_v;
        type_t   = STRING;
        break;

        /** Compound types */
    case ARRAY:
        array_v = v.array_v;
        type_t  = ARRAY;
        break;

    case OBJECT:
        object_v = v.object_v;
        type_t   = OBJECT;
        break;
    }

    return *this;
}

Value &Value::operator=(Value &&v)
{

    switch (v.type())
    {
    /** Base types */
    case INT:
        int_v  = std::move(v.int_v);
        type_t = INT;
        break;

    case FLOAT:
        float_v = std::move(v.float_v);
        type_t  = FLOAT;
        break;

    case BOOL:
        bool_v = std::move(v.bool_v);
        type_t = BOOL;
        break;

    case NIL: type_t = NIL; break;

    case STRING:
        string_v = std::move(v.string_v);
        type_t   = STRING;
        break;

        /** Compound types */
    case ARRAY:
        array_v = std::move(v.array_v);
        type_t  = ARRAY;
        break;

    case OBJECT:
        object_v = std::move(v.object_v);
        type_t   = OBJECT;
        break;
    }

    return *this;
}

// Value& Value::operator=(std::string&& v)
// {

//     string_v = std::move(v);
//     type_t = STRING;
//     return *this;

// }

// Value& Value::operator=(long long && v)
// {

//     int_v = std::move(v);
//     type_t = INT;
//     return *this;

// }

// Value& Value::operator=(double&& v)
// {

//     float_v = std::move(v);
//     type_t = FLOAT;
//     return *this;

// }
// Value& Value::operator=(bool&& v)
// {

//    bool_v = std::move(v);
//    type_t = BOOL;
//     return *this;

// }

// Value& Value::operator=(std::string&& v)
// {

//     string_v = std::move(v);
//     type_t = STRING;
//     return *this;

// }

Value &Value::operator[](const std::string &key)
{
    if (type() != OBJECT)
    {
        Object temp;
        Value ev;

        temp[key] = ev;
        return temp[key];
        throw std::logic_error("Value not an object");
    }

    return object_v[key];
}

const Value &Value::operator[](const std::string &key) const
{
    if (type() != OBJECT)
    {
        Object temp;
        Value ev;

        temp[key] = ev;
        return temp[key];
        throw std::logic_error("Value not an object");
    }
    return object_v[key];
}

Value &Value::operator[](std::size_t i)
{
    if (type() != ARRAY)
    {
        type_t = ARRAY;
        Value ev;
        array_v.push_back(ev);
        return array_v[array_v.size() - 1];
        // throw std::logic_error("Value not an array");
    }

    return array_v[i];
}

const Value &Value::operator[](std::size_t i) const
{
    if (type() != ARRAY)
    {
        // type_t=ARRAY;
        //  Value ev;
        //  return std::move(ev);
        //  array_v.push_back(ev);
        return *this;
        // return std::nullptr_t;
        // throw std::logic_error("Value not an array");
    }
    return array_v[i];
}

void Value::clear()
{

    if (type_t == STRING)
    {
        std::string empty;
        std::swap(string_v, empty);
    }
    else if (type_t == ARRAY)
    {
        array_v.clear();
    }
    else if (type_t == OBJECT)
    {
        object_v.clear();
    }
}

Object::Object() {}

Object::~Object() {}
Object::Object(std::initializer_list<std::list<std::pair<std::string, json::Value>>::value_type> nsl)
{
    for (auto v : nsl)
    {
        const auto it = _mp.find(v.first);
        if (it != _mp.cend())
        {
            it->second->second = v.second;
        }
        else
        {
            _object.emplace_back(std::make_pair(v.first, v.second));
            _mp[v.first] = --_object.end();
        }
        // _object[v.first]=v.second;
    }
}

Object::Object(const Object &o) : _object(o._object), _mp(o._mp) {}

Object::Object(Object &&o) : _object(move(o._object)), _mp(move(o._mp)) {}

Object &Object::operator=(const Object &o)
{
    // _data=o._data;
    _mp     = o._mp;
    _object = o._object;
    return *this;
}

Object &Object::operator=(Object &&o)
{
    //_data=std::move(o._data);
    _mp     = std::move(o._mp);
    _object = std::move(o._object);
    return *this;
}

Value &Object::operator[](const std::string &key)
{
    auto it = _mp.find(key);
    if (it == _mp.cend())
    {
        Value v;
        _object.emplace_back(std::make_pair(key, v));
        _mp[key] = --_object.end();
        return _mp[key]->second;
    }
    else
    {
        return it->second->second;
    }
    // return _object[key];
}

const Value &Object::operator[](const std::string &key) const
{
    const auto it = _mp.find(key);
    if (it == _mp.cend())
    {
        // Value v;
        //_object.emplace_back(std::move(std::make_pair(key,v)));
        //   _mp[key]=--_object.end();
        return (--_object.end())->second;
    }
    else
    {
        return it->second->second;
    }
    // return _object.at(key);
}

void Object::insert(std::string key, Value value)
{
    const auto it = _mp.find(key);

    if (it != _mp.cend())
    {
        it->second->second = value;
    }
    else
    {
        _object.emplace_back(std::make_pair(key, value));
        _mp[key] = --_object.end();
    }
}

void Object::insert(const std::pair<std::string, Value> &v)
{
    const auto it = _mp.find(v.first);

    if (it != _mp.cend())
    {
        it->second->second = v.second;
    }
    else
    {
        // 新建一个节点将其插入到链表头部
        _object.emplace_back(v);

        _mp[v.first] = --_object.end();
    }
}

std::list<std::pair<std::string, json::Value>>::const_iterator Object::begin() const { return _object.begin(); }

std::list<std::pair<std::string, json::Value>>::const_iterator Object::end() const { return _object.end(); }

std::list<std::pair<std::string, json::Value>>::iterator Object::begin() { return _object.begin(); }

std::list<std::pair<std::string, json::Value>>::iterator Object::end() { return _object.end(); }
std::map<std::string, std::list<std::pair<std::string, Value>>::iterator>::iterator Object::order_begin()
{
    return _mp.begin();
}
std::map<std::string, std::list<std::pair<std::string, Value>>::iterator>::iterator Object::order_end()
{
    return _mp.end();
}

bool Object::isset(std::string &key) const
{
    const auto it = _mp.find(key);

    if (it == _mp.cend())
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool Object::isset(std::string &&key) const
{
    const auto it = _mp.find(key);

    if (it == _mp.cend())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Object::unset(std::string &key)
{
    const auto it = _mp.find(key);

    if (it == _mp.cend())
    {
        return false;
    }
    else
    {

        for (std::list<std::pair<std::string, Value>>::iterator ot = _object.begin(); ot != _object.end();)
        {
            if (ot->first == key)
            {
                _object.erase(ot);
                _mp.erase(it);
                return true;
            }
            else
            {
                ++ot;
            }
        }

        return false;
    }
}

bool Object::unset(std::string &&key)
{
    const auto it = _mp.find(key);

    if (it == _mp.cend())
    {
        return false;
    }
    else
    {

        for (std::list<std::pair<std::string, Value>>::iterator ot = _object.begin(); ot != _object.end();)
        {
            if (ot->first == key)
            {
                _object.erase(ot);
                _mp.erase(it);
                return true;
            }
            else
            {
                ++ot;
            }
        }

        return false;
    }
}

std::size_t Object::size() const { return _mp.size(); }

void Object::clear()
{

    for (std::list<std::pair<std::string, Value>>::iterator ot = _object.begin(); ot != _object.end();)
    {
        std::string empty;
        std::swap(ot->first, empty);
        ot->second.clear();
    }
    _object.clear();
    _mp.clear();
}

Array::Array() {}

Array::~Array() {}

Array::Array(const Array &a) : _array(a._array) {}

Array::Array(Array &&a) : _array(move(a._array)) {}
Array::Array(std::initializer_list<Value> nsl)
{
    for (auto v : nsl)
    {
        _array.push_back(v);
    }
}
Array::Array(std::initializer_list<std::map<std::string, Value>::value_type> nsl)
{
    for (auto v : nsl)
    {
        Object obj;
        // obj._object.emplace_back(std::make_pair(v.first,v.second));
        // obj._mp[v.first]=--obj._object.end();
        obj.insert(v.first, v.second);
        _array.push_back(obj);
    }
}

Array &Array::operator=(const Array &a)
{
    _array = a._array;
    return *this;
}

Array &Array::operator=(Array &&a)
{
    _array = move(a._array);
    return *this;
}

Value &Array::operator[](std::size_t i)
{
    if (i >= _array.size())
    {
        Value v;
        _array.push_back(v);
        return _array[_array.size() - 1];
    }
    return _array.at(i);
}

const Value &Array::operator[](std::size_t i) const
{
    if (i >= _array.size())
    {
        // Value v;
        // return std::move(v);
        throw std::logic_error("Value not an array");
    }
    return _array.at(i);
}

std::vector<Value>::const_iterator Array::begin() const { return _array.begin(); }

std::vector<Value>::const_iterator Array::end() const { return _array.end(); }

std::vector<Value>::iterator Array::begin() { return _array.begin(); }

std::vector<Value>::iterator Array::end() { return _array.end(); }

std::size_t Array::size() const { return _array.size(); }

void Array::push_back(const Value &v) { _array.push_back(v); }

void Array::clear()
{

    for (Value a : _array)
    {

        a.clear();
    }
    _array.clear();
}

// 数字转为字符16进制显示
/* void numtohex( unsigned char num,unsigned char *p){
        unsigned char t=0;
        unsigned char str[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
        t=num&0xF;
        p[1]=str[t];
        num=num>>4;
        t=num&0xF;
        p[0]=str[t];
    }
    */
// utf8转为asii 包括emoji转为ascii表示
std::string JSON_UTF8_TO_ASCII(const std::string &source)
{

    unsigned int text_length = source.length(), psize, j = 0;
    const char *text = source.data();
    std::string ptext;
    unsigned char c, t = 0;
    unsigned char str[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    psize = text_length * 4 + 4;
    ptext.resize(psize);

    for (unsigned int i = 0; i < text_length; i++)
    {
        c = *(text + i);
        if (c < 0x80)
        {
            switch (c)
            {
            case 0x22://"
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x22;
                break;
            case 0x60://''
                // ptext[j]=0x5c;
                // j++;
                ptext[j] = 0x60;
                break;
            case 0x27://'\'
                // ptext[j]=0x5c;
                // j++;
                ptext[j] = 0x27;
                break;
            case 0x2f://\/
                // ptext[j]=0x5c;
                // j++;
                ptext[j] = '/';
                break;
            case 0x08://\b
                      // ptext[j]=0x5c;
                // j++;
                // ptext[j]='b';
                break;
            case 0x0c://\f
                      // ptext[j]=0x5c;
                // j++;
                // ptext[j]='f';
                break;
            case 0x0a://\n
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 'n';
                break;
            case 0x0d://\r
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 'r';
                break;
            case 0x09://\t
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 't';
                break;
            case 0x5c://
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x5c;
                break;
            default: ptext[j] = c;
            }
        }
        else if (c >= 0x80 && c < 0xC0)
        {
            ptext[j] = c;
        }
        else if (c >= 0xC0 && c < 0xE0)
        {
            unsigned char b1, b2, cp[2], temp;
            b1 = c;
            b2 = *(text + i + 1);
            if ((b2 & 0xE0) != 0x80)
            {
                ptext[j] = c;
            }
            else
            {
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x75;
                j++;
                temp = (b1 >> 2) & 0x07;
                // numtohex(temp,cp);

                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                j++;
                temp = (b1 << 6) + (b2 & 0x3F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];

                i++;
            }
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            unsigned char b1, b2, b3, cp[2], temp;
            b1 = c;
            b2 = *(text + i + 1);
            b3 = *(text + i + 2);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80))
            {
                ptext[j] = c;
            }
            else
            {
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x75;
                j++;
                temp = (b1 << 4) + ((b2 >> 2) & 0x0F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                j++;
                temp = (b2 << 6) + (b3 & 0x3F);
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                i += 2;
            }
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            unsigned char b1, b2, b3, b4, cp[2], temp, *pOutput;
            unsigned int out, untext;

            b1 = c;
            b2 = *(text + i + 1);
            b3 = *(text + i + 2);
            b4 = *(text + i + 3);

            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80))
            {
                ptext[j] = c;
            }
            else
            {
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x75;
                j++;
                // utf8转为unicode
                pOutput        = (unsigned char *)&out;
                *pOutput       = (b3 << 6) + (b4 & 0x3F);
                *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
                *(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
                untext         = (((((out - 0x10000) >> 10) | 0xD800) << 16) | (((out - 0x10000) & 0x3FF) | 0xDC00));

                pOutput = (unsigned char *)&untext;
                // 高端字节在前
                temp = pOutput[3];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                j++;
                temp = pOutput[2];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                // 加上\u
                j++;
                ptext[j] = 0x5c;
                j++;
                ptext[j] = 0x75;
                j++;
                // 处理低两个字节
                temp = pOutput[1];
                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];
                j++;
                temp = pOutput[0];

                // numtohex(temp,cp);
                t     = temp & 0xF;
                cp[1] = str[t];
                temp  = temp >> 4;
                t     = temp & 0xF;
                cp[0] = str[t];

                ptext[j] = cp[0];
                j++;
                ptext[j] = cp[1];

                i += 3;
            }
        }
        else if (c >= 0xF8 && c < 0xFC)
        {
            ptext[j] = c;
        }
        else
        {
            ptext[j] = c;
        };
        j++;
        if (j > psize)
        {
            psize += psize / 5;
            ptext.resize(psize);
        }
    }
    ptext[j] = '\0';
    ptext.resize(j);
    return ptext;
}

std::string to_json(json::Value &v)
{
    std::ostringstream os;
    switch (v.type())
    {
    /** Base types */
    case INT: os << (long long int)v; break;

    case FLOAT:
    {

        // char temp[33];
        // memset(temp,'\0',33);

        // std::to_chars(temp, temp+32,(long double)v);
        //  unsigned int i=0;

        // for(;i<32;i++){
        //     if(temp[i]=='.'){

        //         break;
        //     }else if(temp[i]=='\0'){
        //         temp[i]='.';
        //         i++;
        //         temp[i]='0';
        //         i++;
        //         temp[i]='\0';

        //         break;
        //     }
        // }

        std::stringstream stream;
        std::string temp;

        stream.precision(15);
        // stream << std::fixed;
        stream << v.as_float();

        temp           = stream.str();
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
            temp.push_back('.');
            temp.push_back('0');
        }

        os << temp;
    }
    break;

    case BOOL: os << ((bool)v ? "true" : "false"); break;

    case NIL: os << "null"; break;

    case STRING: os << '"' << JSON_UTF8_TO_ASCII((std::string)v) << '"'; break;

    /** Compound types */
    case ARRAY: os << (Array)v; break;

    case OBJECT: os << (Object)v; break;
    }
    return os.str();
}

std::string to_json(json::Object &o)
{

    std::ostringstream os;
    os << "{";
    for (auto e = o.begin(); e != o.end();)
    {
        if (e->second.isNum())
        {
            std::stringstream stream;
            std::string temp;
            if (e->second.isFloat())
            {
                stream.precision(15);
                // stream << std::fixed;
                stream << e->second.as_float();

                temp           = stream.str();
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
                    temp.push_back('.');
                    temp.push_back('0');
                }
            }
            else
            {
                stream << (long long int)e->second;
                temp = stream.str();
            }

            os << '"' << e->first << '"' << ":" << temp;
        }
        else
        {
            os << '"' << e->first << '"' << ":" << e->second;
        }

        if (++e != o.end())
            os << ",";
    }
    os << "}";
    return os.str();
}
std::string to_json(json::Array &a)
{
    std::ostringstream os;
    os << "[";
    for (auto e = a.begin(); e != a.end();)
    {
        os << (*e);
        if (++e != a.end())
            os << ",";
    }
    os << "]";
    return os.str();
}

std::string JSON_VALUE(const std::string &jsonstr, unsigned int &offset)
{
    unsigned int j = 0;
    std::string temp;
    if (jsonstr[offset] == 0x3a)
    {//:
        offset++;
    }
    for (j = offset; j < jsonstr.length(); j++)
    {
        // } ],
        if (jsonstr[j] == 0x7d || jsonstr[j] == 0x5d || jsonstr[j] == 0x2c)
        {
            // 因为没有分隔所以要退一步 is no separation, so to step back
            j--;
            break;
        }
        else if (jsonstr[j] != 0x0D && jsonstr[j] != 0x0A && jsonstr[j] != 0x20 && jsonstr[j] != 0x09)
        {
            temp.push_back(jsonstr[j]);
        }
    }
    offset = j;
    return temp;
}

std::string JSON_STR(const std::string &jsonstr, unsigned int &offset)
{
    unsigned int j = 0;
    std::string str;
    j = jsonstr.length();
    if (j > 200)
    {
        str.reserve(1024);
    }

    j = 0;
    if (jsonstr[offset] == 0x22)
    {
        offset++;
    }
    for (j = offset; j < jsonstr.length(); j++)
    {

        if (jsonstr[j] == 0x5c)//'\'
        {
            // 处理有斜杠情况 not slash
            switch (jsonstr[j + 1])
            {
            case 0x22://"
                str += 0x22;
                j += 1;
                break;
            case 0x5c://'\'
                str += 0x5c;
                j += 1;
                break;
            // case 0x2f: //\/
            case 0x62://\b
                str += 0x08;
                j += 1;
                break;
            case 0x66://\f
                str += 0x0c;
                j += 1;
                break;
            case 0x6e://\n
                str += 0x0a;
                j += 1;
                break;
            case 0x72://\r
                str += 0x0d;
                j += 1;
                break;
            case 0x74://\t
                str += 0x09;
                j += 1;
                break;
            case 'u':
                // str+=json.substr(j,6);
                {
                    unsigned char c[10];
                    unsigned char ch;
                    unsigned int temp;
                    auto chartoint = [](unsigned char &ch, unsigned char &c)
                    {
                        if (ch >= '0' && ch <= '9')
                            c = (unsigned char)(ch - '0');
                        if (ch >= 'a' && ch <= 'f')
                            c = (unsigned char)(ch - 'a' + 10);
                        if (ch >= 'A' && ch <= 'F')
                            c = (unsigned char)(ch - 'A' + 10);
                    };

                    // 检查是不是emoji两个转码符一共12个字符 this emoji char
                    if (jsonstr[j + 2] == 'd' && jsonstr[j + 6] == 0x5c && jsonstr[j + 7] == 'u' &&
                        jsonstr[j + 8] == 'd')
                    {

                        // 转换成utf16
                        for (int si = 2, cj = 0; si < 12; si++)
                        {

                            if (jsonstr[j + si] != 0x5c && jsonstr[j + si] != 'u')
                            {
                                ch = jsonstr[j + si];
                                chartoint(ch, c[cj]);
                                cj++;
                            }
                        }

                        unsigned short ca = c[0] << 12 | c[1] << 8 | c[2] << 4 | c[3];
                        unsigned short cb = c[4] << 12 | c[5] << 8 | c[6] << 4 | c[7];
                        temp              = 0x10000 | ((ca - 0xD800) << 10) | (cb - 0xDC00);
                        // Unicode码再转换成utf-8
                        c[3] = (temp & 0x3F) | 0x80;
                        c[2] = ((temp >> 6) & 0x3F) | 0x80;
                        c[1] = ((temp >> 12) & 0x3F) | 0x80;
                        c[0] = ((temp >> 18) & 0x07) | 0xF0;
                        c[4] = '\0';
                        // emoji:c
                        // 再把utf-8组装到字符串上
                        str += c[0];
                        str += c[1];
                        str += c[2];
                        str += c[3];
                        j += 11;
                    }
                    else
                    {
                        // 只是Unicode码情况 4 char
                        for (int si = 2, cj = 0; si < 6; si++)
                        {
                            ch = jsonstr[j + si];
                            chartoint(ch, c[cj]);
                            cj++;
                        }
                        temp = c[0] << 4 | c[1];
                        temp = temp << 8 | c[2] << 4 | c[3];
                        c[3] = '\0';
                        c[2] = (temp & 0x3F) | 0x80;
                        c[1] = ((temp >> 6) & 0x3F) | 0x80;
                        c[0] = ((temp >> 12) & 0x0F) | 0xE0;

                        str += c[0];
                        str += c[1];
                        str += c[2];
                        j += 5;
                    }
                }
                break;
            default: str += jsonstr[j + 1]; j += 1;
            }
        }
        else if (jsonstr[j] == 0x22)
        {
            //" 如果是单独的双引号表示值结束 if quotation marks this end
            break;
        }
        else
        {
            str += jsonstr[j];
        }
    }
    offset = j;
    return str;
}

int JSON_OBJ(const std::string &jsonstr, Object &obj, unsigned int offset)
{
    unsigned int i = 0, nobj = 0;
    std::string key, value;
    Value temp;
    if (jsonstr[offset] == 0x7b)
    {
        offset++;
    }
    for (i = offset; i < jsonstr.length(); i++)
    {
        if (jsonstr[i] == 0x20 || jsonstr[i] == 0x0D || jsonstr[i] == 0x0A || jsonstr[i] == 0x09)
        {
            // 如果是空格 space
            continue;
        }

        if (jsonstr[i] == 0x7b)
        {
            Object newobj;
            i = JSON_OBJ(jsonstr, newobj, i);
            if (key.empty())
            {
                obj[std::to_string(nobj)] = newobj;
                nobj++;
                key.clear();
            }
            else
            {
                obj[key] = newobj;
                key.clear();
            }
        }
        else if (jsonstr[i] == 0x7d)
        {
            // 结束本层对象 end obj
            if (!key.empty())
            {
                obj[std::to_string(nobj)] = key;
                nobj++;
                key.clear();
            }
            break;
        }
        else if (jsonstr[i] == 0x5b)
        {
            // 数组
            Array newobj;
            i = JSON_ARRAY(jsonstr, newobj, i);
            if (key.empty())
            {
                obj[std::to_string(nobj)] = newobj;
                nobj++;
                key.clear();
            }
            else
            {
                obj[key] = newobj;
                key.clear();
            }
        }
        else if (jsonstr[i] == 0x22)
        {
            // 遇到字符
            key = JSON_STR(jsonstr, i);
        }
        else if (jsonstr[i] == 0x2c)
        {
            // 逗号，表示结速一个key,value
            if (!key.empty())
            {
                obj[std::to_string(nobj)] = key;
                nobj++;
                key.clear();
            }
        }
        else if (jsonstr[i] == 0x3A)
        {
            // 冒号，表示结速一个key
            // 下一个字符如果不是[{"说明是数字或true或null
            // A JSON value can be an string,array,object,number,true,false,or null.
            // 如果是引号说明了没有下一层
            value.clear();
            // 消除空格
            // jsonstr[j]!=0x0D&&jsonstr[j]!=0x0A&&jsonstr[j]!=0x20&&jsonstr[j]!=0x09
            while (jsonstr[i + 1] == 0x20 || jsonstr[i + 1] == 0x0D || jsonstr[i + 1] == 0x0A || jsonstr[i + 1] == 0x09)
            {
                ++i;
            }

            if (jsonstr[i + 1] == 0x22)
            {
                // 字符
                i++;
                value    = JSON_STR(jsonstr, i);
                obj[key] = value;
            }
            else if (jsonstr[i + 1] == 0x5b)
            {
                Array newobj;
                i++;
                i = JSON_ARRAY(jsonstr, newobj, i);
                if (key.empty())
                {
                    obj[std::to_string(nobj)] = newobj;
                    nobj++;
                    key.clear();
                }
                else
                {
                    obj[key] = newobj;
                    key.clear();
                }
            }
            else if (jsonstr[i + 1] == 0x7b)
            {
                // 进入对象
                i++;
                Object newobj;
                i = JSON_OBJ(jsonstr, newobj, i);
                if (key.empty())
                {
                    obj[std::to_string(nobj)] = newobj;
                    nobj++;
                }
                else
                {
                    obj[key] = newobj;
                }
            }
            else
            {
                // number,true,false,or null
                value = JSON_VALUE(jsonstr, i);
                value.erase(0, value.find_first_not_of(" "));
                value.erase(value.find_last_not_of(" ") + 1);

                if (value == "true" || value == "TRUE")
                {
                    obj[key] = true;
                }
                else if (value == "false" || value == "FALSE")
                {
                    obj[key] = false;
                }
                else if (value == "null" || value == "NULL")
                {
                    Value tv;
                    obj[key] = tv;
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    long double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0.0;
                    }
                    obj[key] = tc;
                }
                else
                {
                    long long int tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    obj[key] = tc;
                }
            }
            key.clear();
            value.clear();
        }
    }
    return i;
}

int JSON_ARRAY(const std::string &jsonstr, Array &obj, unsigned int offset)
{
    unsigned int i = 0;
    std::string key, value;
    if (jsonstr[offset] == 0x5b)
    {
        offset++;
    }
    for (i = offset; i < jsonstr.length(); i++)
    {
        if (jsonstr[i] == 0x20 || jsonstr[i] == 0x0D || jsonstr[i] == 0x0A || jsonstr[i] == 0x09)
        {
            // 如果是空格 space
            continue;
        }

        if (jsonstr[i] == 0x5b)
        {
            // 数组
            Array newobj;
            i = JSON_ARRAY(jsonstr, newobj, i);
            obj.push_back(newobj);
            key.clear();
        }
        else if (jsonstr[i] == 0x5d)
        {
            // 结束数组
            if (!value.empty())
            {
                value.erase(0, value.find_first_not_of(" "));
                value.erase(value.find_last_not_of(" ") + 1);

                if (value == "true" || value == "TRUE")
                {
                    obj.push_back(true);
                }
                else if (value == "false" || value == "FALSE")
                {
                    obj.push_back(false);
                }
                else if (value == "null" || value == "NULL")
                {
                    Value tv;
                    obj.push_back(tv);
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    long double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    obj.push_back(tc);
                }
                else
                {
                    long long int tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    obj.push_back(tc);
                }
            }
            value.clear();
            key.clear();
            break;
        }
        else if (jsonstr[i] == 0x7b)
        {
            // 进入对象
            Object newobj;
            i = JSON_OBJ(jsonstr, newobj, i);
            obj.push_back(newobj);
            key.clear();
        }
        else if (jsonstr[i] == 0x22)
        {
            // 遇到字符,里面肯定没有下一层了
            key = JSON_STR(jsonstr, i);
            obj.push_back(key);
            key.clear();
        }
        else if (jsonstr[i] == 0x2c)
        {
            // 逗号，表示结速一个key,value
            // 冒号，表示结速一个key
            // 如果是接着一个引号，说明也是字符，没有下一层
            // A JSON value can be an string,array,object,number,true,false,or null.
            // value.clear();
            // 消除空格

            if (!value.empty())
            {

                value.erase(0, value.find_first_not_of(" "));
                value.erase(value.find_last_not_of(" ") + 1);

                if (value == "true" || value == "TRUE")
                {
                    obj.push_back(true);
                }
                else if (value == "false" || value == "FALSE")
                {
                    obj.push_back(false);
                }
                else if (value == "null" || value == "NULL")
                {
                    Value tv;
                    obj.push_back(tv);
                }
                else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos ||
                         value.find('E') != std::string::npos)
                {
                    long double tc;
                    try
                    {
                        tc = std::stold(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0.0;
                    }
                    obj.push_back(tc);
                }
                else
                {
                    long long int tc;
                    try
                    {
                        tc = std::atoll(value.c_str());
                    }
                    catch (const std::exception &e)
                    {
                        tc = 0;
                    }
                    obj.push_back(tc);
                }
            }
            value.clear();
            key.clear();
        }
        else
        {
            value += jsonstr[i];
        }
    }
    return i;
}

Value from_json(const std::string &jsonstr)
{
    Object obj;
    int offset          = 0;
    unsigned int zlsize = jsonstr.length();
    if (zlsize > 5000000)
    {
        // limit size
        return obj;
    }
    // 分出数组或对象 消除前面空格
    for (unsigned int i = 0; i < zlsize; i++)
    {
        if (jsonstr[i] == 0x7b || jsonstr[i] == 0x5b)
        {
            offset = i;
            break;
        }
    }
    if (jsonstr[offset] == 0x7b)
    {
        // 对象情况
        JSON_OBJ(jsonstr, obj, offset);
    }
    else if (jsonstr[offset] == 0x5b)
    {
        // 数组情况
        Array arr;
        JSON_ARRAY(jsonstr, arr, offset);
        return arr;
    }

    return obj;
}

Value from_json(const std::string &jsonstr, bool isarray)
{
    Array obj;
    int offset          = 0;
    unsigned int zlsize = jsonstr.length();
    if (zlsize > 5000000)
    {
        // limit size
        return obj;
    }
    // 分出数组或对象 消除前面空格
    for (unsigned int i = 0; i < zlsize; i++)
    {
        if (jsonstr[i] == 0x7b || jsonstr[i] == 0x5b)
        {
            offset = i;
            break;
        }
    }
    if (jsonstr[offset] == 0x7b)
    {
        // 对象情况
        Object newobj;
        JSON_OBJ(jsonstr, newobj, offset);
        // obj.push_back(newobj);
        return newobj;
    }
    else if (jsonstr[offset] == 0x5b)
    {
        // 数组情况

        JSON_ARRAY(jsonstr, obj, offset);
    }
    if (true || isarray)
    {
    }
    return obj;
}

std::string load_json(const std::string &path)
{
    std::string s;

    FILE *f = fopen(path.c_str(), "rb");
    if (f == NULL)
    {
        return s;
    }
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);

    s.resize(size);
    fseek(f, 0, SEEK_SET);
    auto const nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    return s;
}

bool save_json(const std::string &path, const std::string &content)
{

    FILE *f = fopen(path.c_str(), "wb+");
    if (f == NULL)
    {
        return false;
    }
    size_t n = fwrite(content.data(), 1, content.size(), f);
    fclose(f);
    return n > 0 ? true : false;
}

}// namespace json

std::ostream &operator<<(std::ostream &os, const json::Value &v)
{
    switch (v.type())
    {
    /** Base types */
    case json::INT: os << (long long int)v; break;

    case json::FLOAT:

    {

        std::stringstream stream;
        std::string temp;

        stream.precision(15);
        // stream << std::fixed;
        stream << v.as_float();

        temp           = stream.str();
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
            temp.push_back('.');
            temp.push_back('0');
        }

        os << temp;
    }

    break;

    case json::BOOL: os << ((bool)v ? "true" : "false"); break;

    case json::NIL: os << "null"; break;

    case json::STRING: os << '"' << json::JSON_UTF8_TO_ASCII((std::string)v) << '"'; break;

    /** Compound types */
    case json::ARRAY: os << (json::Array)v; break;

    case json::OBJECT: os << (json::Object)v; break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const json::Object &o)
{
    os << "{";
    for (auto e = o.begin(); e != o.end();)
    {
        if (e->second.isNum())
        {

            std::stringstream stream;
            std::string temp;
            if (e->second.isFloat())
            {
                stream.precision(15);
                // stream << std::fixed;
                stream << e->second.as_float();

                temp           = stream.str();
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
                    temp.push_back('.');
                    temp.push_back('0');
                }
            }
            else
            {
                stream << (long long int)e->second;
                temp = stream.str();
            }

            os << '"' << e->first << '"' << ":" << temp;
        }
        else
        {
            os << '"' << e->first << '"' << ":" << e->second;
        }

        if (++e != o.end())
            os << ",";
    }
    os << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const json::Array &a)
{
    os << "[";
    for (auto e = a.begin(); e != a.end();)
    {
        os << (*e);
        if (++e != a.end())
            os << ",";
    }
    os << "]";

    return os;
}
