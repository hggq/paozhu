
#include <stdexcept>
#include <sstream>   // ostringstream 类型
//#include <strstream> // ostrstream 类型
#include <string>
#include <cstdlib>
#include <charconv>
#include <cstring>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <utility>
#include <charconv>
#include <unordered_map>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include <functional>
#include "request.h"

namespace http
{

    unsigned long long hash_objkey(const std::string &key)
    {
        OBJ_KEY mapkey;
        unsigned char i = 0;
        bool isnum = true;
        for (; i < key.length(); i++)
        {
            if (i > 7)
            {
                break;
            }
            mapkey.ckey[i] = key[i];
            if (isnum)
            {
                if (key[i] > 0x39 || key[i] < 0x30)
                {
                    isnum = false;
                }
            }
        }
        unsigned char n = 7;
        for (; i < key.length(); i++)
        {
            unsigned char temp = (unsigned char)(mapkey.ckey[n] + key[i]);
            mapkey.ckey[n] = temp;
            if (isnum)
            {
                if (key[i] > 0x39 || key[i] < 0x30)
                {
                    isnum = false;
                }
            }
            if (n == 0)
            {
                n = 8;
            }
            n--;
        }
        if (isnum)
        {
            unsigned long long temp = 0;
            unsigned long long j = 1;
            for (int i = key.length() - 1; i >= 0; i--)
            {
                temp = temp + (key[i] - '0') * j;
                j = j * 10;
            }
            mapkey.ikey = temp;
        }
        return mapkey.ikey;
    }
    OBJ_VALUE::OBJ_VALUE() : type_t(NIL) {}
    OBJ_VALUE::OBJ_VALUE(const long long i) : int_v(i), type_t(INT) {}
    OBJ_VALUE::OBJ_VALUE(const unsigned long long i) : int_v(static_cast<long long>(i)), type_t(INT) {}
    OBJ_VALUE::OBJ_VALUE(const long i) : int_v(static_cast<long long>(i)), type_t(INT) {}
    OBJ_VALUE::OBJ_VALUE(const unsigned int i) : int_v(static_cast<long long>(i)), type_t(INT) {}

    OBJ_VALUE::OBJ_VALUE(const int i) : int_v(static_cast<int>(i)), type_t(INT) {}
    OBJ_VALUE::OBJ_VALUE(const bool i) : int_v(static_cast<bool>(i)), type_t(INT) {}

    OBJ_VALUE::OBJ_VALUE(const long double f) : float_v(f), type_t(FLOAT) {}

    OBJ_VALUE::OBJ_VALUE(const double f) : float_v(static_cast<long double>(f)), type_t(FLOAT) {}

    OBJ_VALUE::OBJ_VALUE(const char *s) : string_v(s), type_t(STRING) {}

    OBJ_VALUE::OBJ_VALUE(const std::string &s) : string_v(s), type_t(STRING) {}
    OBJ_VALUE::OBJ_VALUE(std::string &&s) : string_v(std::move(s)), type_t(STRING) {}

    OBJ_VALUE::OBJ_VALUE(const OBJ_ARRAY &o) : array_v(o), type_t(ARRAY) {}
    OBJ_VALUE::OBJ_VALUE(OBJ_ARRAY &&o) : array_v(std::move(o)), type_t(ARRAY) {}

    OBJ_VALUE::~OBJ_VALUE()
    {
        array_v.clear();
        string_v.clear();

        std::string empty;
        std::swap(string_v, empty);
    }

    OBJ_VALUE::OBJ_VALUE(std::initializer_list<std::string> nsl)
    {
        unsigned long long temp = 0;
        type_t = ARRAY;
        for (auto v : nsl)
        {
            OBJ_VALUE vv(v);
            array_v.push(temp, vv);
            temp++;
        }
    }
    OBJ_VALUE::OBJ_VALUE(std::initializer_list<std::map<unsigned long long, OBJ_VALUE>::value_type> nsl)
    {
        type_t = ARRAY;
        for (auto v : nsl)
        {
            array_v.push(v.first, v.second);
        }
    }
    OBJ_VALUE::OBJ_VALUE(const OBJ_VALUE &v)
    {
        switch (v.type())
        {
        case INT:
            int_v = v.int_v;
            type_t = INT;
            break;

        case FLOAT:
            float_v = v.float_v;
            type_t = FLOAT;
            break;

        case NIL:
            type_t = NIL;
            break;

        case STRING:
            string_v = v.string_v;
            type_t = STRING;
            break;

        case ARRAY:
            array_v = v.array_v;
            type_t = ARRAY;
            break;
        }
    }
    OBJ_VALUE::OBJ_VALUE(OBJ_VALUE &&v)
    {

        switch (v.type())
        {
        case INT:
            int_v = std::move(v.int_v);
            type_t = INT;
            break;

        case FLOAT:
            float_v = std::move(v.float_v);
            type_t = FLOAT;
            break;

        case NIL:
            type_t = NIL;
            break;

        case STRING:
            string_v = std::move(v.string_v);
            type_t = STRING;
            break;

        case ARRAY:
            array_v = std::move(v.array_v);
            type_t = ARRAY;
            break;
        }
    }

    OBJ_VALUE &OBJ_VALUE::operator=(const OBJ_VALUE &v)
    {
        if (type() == ARRAY)
        {
            unsigned int len;
            len = array_v.size();
            if(v.type()!=ARRAY)
            {
                for (long long i = len; i >= 0; i--)
                {
                    if (!array_v.isset(i))
                    {
                        array_v.push(i, v);
                        break;
                    }
                }
            }    
            else
            {
                if(!v.array_v.empty())
                {
                    array_v = v.array_v;
                }
            }
        }
        else
        {
            switch (v.type())
            {
            case INT:
                int_v = v.int_v;
                type_t = INT;
                break;

            case FLOAT:
                float_v = v.float_v;
                type_t = FLOAT;
                break;

            case NIL:
                type_t = NIL;
                break;

            case STRING:
                string_v = v.string_v;
                type_t = STRING;
                break;
            case ARRAY:
                //if(!array_v._array.empty())
                {
                    array_v._array = v.array_v._array;
                    array_v._tag = v.array_v._tag;
                }
                type_t = ARRAY;
                break;
            }
        }
        return *this;
    }

    OBJ_VALUE &OBJ_VALUE::operator=(OBJ_VALUE &&v)
    {
        if (type() == ARRAY)
        {
            unsigned int len;
            len = array_v.size();
            if(v.type()!=ARRAY)
            {
                for (long long i = len; i >= 0; i--)
                {
                    if (!array_v.isset(i))
                    {
                        array_v.push(i, std::move(v));
                        break;
                    }
                }
            }
            else
            {
                 if(!v.array_v.empty())
                {
                    array_v = v.array_v;
                }
            }
        }
        else
        {
            switch (v.type())
            {
            case INT:
                int_v = std::move(v.int_v);
                type_t = INT;
                break;

            case FLOAT:
                float_v = std::move(v.float_v);
                type_t = FLOAT;
                break;

            case NIL:
                type_t = NIL;
                break;

            case STRING:
                string_v = std::move(v.string_v);
                type_t = STRING;
                break;

            case ARRAY:
                array_v = std::move(v.array_v);
                type_t = ARRAY;
                break;
            }
        }

        return *this;
    }

    OBJ_VALUE &OBJ_VALUE::operator[](const std::string &key)
    {

        unsigned long long temp;

        temp = hash_objkey(key);

        if (array_v.isset(temp))
        {

            return array_v[temp];
        }
        else
        {
            OBJ_VALUE v;
            array_v[key] = std::move(v);
            type_t = ARRAY;
            return array_v[temp];
        }
    }
    OBJ_VALUE &OBJ_VALUE::operator[](const std::string &&key)
    {

        unsigned long long temp;

        temp = hash_objkey(key);

        if (array_v.isset(temp))
        {

            return array_v[temp];
        }
        else
        {
            OBJ_VALUE v;
            array_v[key] = std::move(v);
            type_t = ARRAY;
            return array_v[temp];
        }
    }

    // const OBJ_VALUE& OBJ_VALUE::operator[] (const std::string& key) const
    // {
    //    unsigned long long temp;

    //    temp=hash_objkey(key);

    //    if(array_v.isset(temp)){

    //        return array_v[temp];
    //    }else{
    //        OBJ_VALUE v;
    //      //  array_v[temp]=v;
    //        return v;
    //    }
    // }

    OBJ_VALUE &OBJ_VALUE::operator[](unsigned long long temp)
    {

        if (array_v.isset(temp))
        {

            return array_v[temp];
        }
        else
        {
            OBJ_VALUE v;
            array_v[temp] = std::move(v);
            type_t = ARRAY;
            return array_v[temp];
        }
    }

    // const OBJ_VALUE& OBJ_VALUE::operator[] (unsigned long long temp) const
    // {

    // //    if(array_v.find(temp)){

    // //        return array_v[temp];
    // //    }else{
    // //        OBJ_VALUE v;
    // //        array_v[temp]=v;
    // //        return array_v[temp];
    // //    }
    // }

    void OBJ_VALUE::push(const OBJ_VALUE &v)
    {
        unsigned int len;
        len = array_v.size();

        for (long long i = len; i >= 0; i--)
        {
            if (!array_v.isset(i))
            {
                array_v.push(i, v);
                break;
            }
        }
        type_t = ARRAY;
    }

    void OBJ_VALUE::push(const OBJ_VALUE &&v)
    {
        unsigned int len;
        len = array_v.size();

        for (long long i = len; i >= 0; i--)
        {
            if (!array_v.isset(i))
            {
                array_v.push(i, std::move(v));
                break;
            }
        }
        type_t = ARRAY;
    }

    void OBJ_VALUE::push(unsigned long long temp, std::string &v)
    {
        OBJ_VALUE vv(v);
        array_v.push(temp, std::move(vv));
    }
    void OBJ_VALUE::push(unsigned long long temp, std::string &&v)
    {
        OBJ_VALUE vv(std::move(v));
        array_v.push(temp, std::move(vv));
    }
    void OBJ_VALUE::push(unsigned long long temp, const OBJ_VALUE &v)
    {
        array_v.push(temp, v);
    }

    void OBJ_VALUE::push(std::string &key, OBJ_VALUE &v)
    {

        array_v.push(key, v);
    }
    void OBJ_VALUE::push(std::string &key, OBJ_VALUE &&v)
    {
        array_v.push(key, std::move(v));
    }

    void OBJ_VALUE::push(std::string &&key, OBJ_VALUE &v)
    {

        array_v.push(std::move(key), v);
    }
    void OBJ_VALUE::push(std::string &&key, OBJ_VALUE &&v)
    {
        array_v.push(std::move(key), std::move(v));
    }

    void OBJ_VALUE::clear()
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
    }
    std::string OBJ_VALUE::tag()
    {

        return string_v;
    }
    std::string OBJ_VALUE::tag(const std::string &v)
    {

        if (v.empty())
        {
            return string_v;
        }

        if (type_t == ARRAY)
        {
            string_v = v;
        }
        else
        {
            type_t = ARRAY;
            string_v = v;
            OBJ_VALUE vv;
            array_v[0] = std::move(vv);
        }
        return string_v;
    }
    std::string OBJ_VALUE::tag(const std::string &&v)
    {
        if (v.empty())
        {
            return string_v;
        }

        if (type_t == ARRAY)
        {
            string_v = std::move(v);
        }
        else
        {
            type_t = ARRAY;
            string_v = std::move(v);
            OBJ_VALUE vv;
            array_v[0] = std::move(vv);
        }
        return string_v;
    }
    std::string OBJ_VALUE::getkeyname(unsigned long long temp)
    {
        return array_v.getkeyname(temp);
    }

    bool OBJ_VALUE::isset(unsigned long long temp)
    {

        return array_v.isset(temp);
    }
    bool OBJ_VALUE::isset(const std::string &key)
    {
        return array_v.isset(key);
    }
    bool OBJ_VALUE::isset(const std::string &&key)
    {
        return array_v.isset(std::move(key));
    }

    bool OBJ_VALUE::unset(unsigned long long temp)
    {
        return array_v.unset(temp);
    }

    bool OBJ_VALUE::unset(const std::string &key)
    {
        return array_v.unset(key);
    }

    bool OBJ_VALUE::unset(const std::string &&key)
    {
        return array_v.unset(std::move(key));
    }

    std::size_t OBJ_VALUE::size() const
    {
        switch (type())
        {
        case INT:
            return sizeof(long long);
            break;

        case FLOAT:
            return sizeof(long double);
            break;

        case NIL:
            return 0;
            break;

        case STRING:
            return string_v.size();
            break;

        case ARRAY:
            return array_v.size();
            break;
        default:
            return 0;
        }
    }

    OBJ_VALUE OBJ_VALUE::operator+(int v)
    {
        OBJ_VALUE temp;
        temp.set_int();

        temp = int_v + v;

        return temp;
    }
    OBJ_VALUE OBJ_VALUE::operator+(float v)
    {
        OBJ_VALUE temp;
        temp.set_float();

        temp = float_v + v;

        return temp;
    }
    OBJ_VALUE OBJ_VALUE::operator+(char v)
    {
        OBJ_VALUE temp;
        std::string &str = temp.ref();

        str.append(string_v);
        str.push_back(v);

        return temp;
    }
    OBJ_VALUE OBJ_VALUE::operator+(std::string v)
    {
        OBJ_VALUE temp;
        std::string &str = temp.ref();

        str.append(string_v);
        str.append(v);

        return temp;
    }
    OBJ_VALUE OBJ_VALUE::operator-(OBJ_VALUE &v)
    {
        OBJ_VALUE temp;

        temp.set_array();
        for (auto [first, second] : array_v)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v.as_array())
        {
            if (temp.isset(first))
            {
                temp.unset(first);
            }
        }

        return temp;
    }

    OBJ_VALUE OBJ_VALUE::operator-(OBJ_ARRAY &v)
    {
        OBJ_VALUE temp;

        temp.set_array();
        for (auto [first, second] : array_v)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v)
        {
            if (temp.isset(first))
            {
                temp.unset(first);
            }
        }

        return temp;
    }

    OBJ_VALUE OBJ_VALUE::operator+(OBJ_ARRAY &v)
    {
        OBJ_VALUE temp;

        temp.set_array();
        for (auto [first, second] : array_v)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v)
        {
            temp[v.getkeyname(first)] = second;
        }

        return temp;
    }

    OBJ_VALUE OBJ_VALUE::operator+(OBJ_VALUE &v)
    {
        OBJ_VALUE temp;
        std::string &str = temp.ref();
        switch (type())
        {
        case INT:

            switch (v.type())
            {
            case INT:

                temp.set_int();

                temp = int_v + v.as_int();

                break;

            case FLOAT:

                temp.set_float();
                temp = (float)int_v + v.as_float();

                break;
            default:
                temp.set_int();

                temp = int_v;
                ;
            }

            break;

        case FLOAT:
            switch (v.type())
            {
            case INT:

                temp.set_float();

                temp = float_v + (float)v.as_int();

                break;

            case FLOAT:

                temp.set_float();
                temp = float_v + v.as_float();

                break;
            default:
                temp.set_float();
                temp = float_v;
                ;
            }
            break;

        case NIL:
            temp = v;
            break;

        case STRING:
            temp.set_string();

            str.append(string_v);
            str.append(v.as_string());
            break;

        case ARRAY:

            temp.set_array();

            for (auto [first, second] : array_v)
            {
                temp[getkeyname(first)] = second;
            }

            for (auto [first, second] : v.as_array())
            {
                temp[v.getkeyname(first)] = second;
            }

            break;
        }
        return temp;
    }
    OBJ_ARRAY::OBJ_ARRAY() {}

    OBJ_ARRAY::~OBJ_ARRAY()
    {
        std::map<unsigned long long, OBJ_VALUE> empty;
        _array.clear();
        std::swap(_array, empty);
    }
    OBJ_ARRAY::OBJ_ARRAY(const OBJ_ARRAY &a) : _array(a._array), _tag(a._tag) {}

    OBJ_ARRAY::OBJ_ARRAY(OBJ_ARRAY &&a) : _array(std::move(a._array)), _tag(std::move(a._tag)) {}

    OBJ_ARRAY::OBJ_ARRAY(std::initializer_list<std::string> nsl)
    {
        unsigned long long temp = 0;
        for (auto v : nsl)
        {
            OBJ_VALUE vv(v);
            _array[temp] = std::move(vv);
            temp++;
        }
    }
    OBJ_ARRAY::OBJ_ARRAY(std::initializer_list<std::map<unsigned long long, OBJ_VALUE>::value_type> nsl)
    {
        for (auto v : nsl)
        {
            _array[v.first] = v.second;
        }
    }

    OBJ_ARRAY &OBJ_ARRAY::operator=(const OBJ_ARRAY &a)
    {
        if(a._array.empty())
        {
            return *this;
        }
        _array = a._array;
        _tag = a._tag;
        return *this;
    }

    OBJ_ARRAY &OBJ_ARRAY::operator=(OBJ_ARRAY &&a)
    {
        if(a._array.empty())
        {
            return *this;
        }
        _array = std::move(a._array);
        _tag = std::move(a._tag);
        return *this;
    }

    std::map<unsigned long long, OBJ_VALUE>::iterator OBJ_ARRAY::begin()
    {
        return _array.begin();
    }

    std::map<unsigned long long, OBJ_VALUE>::iterator OBJ_ARRAY::end()
    {
        return _array.end();
    }

    std::map<unsigned long long, OBJ_VALUE>::const_iterator OBJ_ARRAY::begin() const
    {
        return _array.begin();
    }

    std::map<unsigned long long, OBJ_VALUE>::const_iterator OBJ_ARRAY::end() const
    {
        return _array.end();
    }

    OBJ_VALUE &OBJ_ARRAY::operator[](const std::string &key)
    {
        unsigned long long temp;

        temp = hash_objkey(key);

        if (_array.find(temp) == _array.end())
        {
            OBJ_VALUE v;
            _array[temp] = std::move(v);
            _tag[temp] = key;
            return _array[temp];
        }
        else
        {
            return _array[temp];
        }
    }
    OBJ_VALUE &OBJ_ARRAY::operator[](const std::string &&key)
    {
        unsigned long long temp;

        temp = hash_objkey(key);
        if (_array.find(temp) == _array.end())
        {
            OBJ_VALUE v;
            _array[temp] = std::move(v);
            _tag[temp] = key;
            return _array[temp];
        }
        else
        {
            return _array[temp];
        }
    }

    OBJ_VALUE &OBJ_ARRAY::operator[](unsigned long long temp)
    {
        if (_array.find(temp) == _array.end())
        {
            OBJ_VALUE v;
            _array[temp] = std::move(v);
            return _array[temp];
        }
        else
        {
            return _array[temp];
        }
    }
    // const OBJ_VALUE& OBJ_ARRAY::operator[] (unsigned long long temp) const
    // {
    //     if(_array.find(temp)==_array.end()){
    //        OBJ_VALUE v;
    //        _array[temp]=v;
    //        return _array[temp];
    //    }else{
    //        return _array[temp];
    //    }
    //}

    std::size_t OBJ_ARRAY::size() const
    {
        return _array.size();
    }

    void OBJ_ARRAY::push(const OBJ_VALUE &v)
    {
        unsigned int len;
        len = _array.size();

        for (long long i = len; i >= 0; i--)
        {
            if (_array.find(i) == _array.end())
            {
                _array[i] = v;
                break;
            }
        }
    }

    void OBJ_ARRAY::push(const OBJ_VALUE &&v)
    {
        unsigned int len;
        len = _array.size();

        for (long long i = len; i >= 0; i--)
        {
            if (_array.find(i) == _array.end())
            {
                _array[i] = std::move(v);
                break;
            }
        }
    }

    void OBJ_ARRAY::push(unsigned long long temp, std::string &v)
    {

        OBJ_VALUE vv(v);
        _array[temp] = std::move(vv);
    }
    void OBJ_ARRAY::push(unsigned long long temp, std::string &&v)
    {

        OBJ_VALUE vv(std::move(v));
        _array[temp] = std::move(vv);
    }

    void OBJ_ARRAY::push(std::string &key, OBJ_VALUE &v)
    {
        unsigned long long temp = hash_objkey(key);

        _tag[temp] = key;
        _array[temp] = v;
    }

    void OBJ_ARRAY::push(std::string &key, OBJ_ARRAY &&v)
    {

        unsigned long long temp = hash_objkey(key);

        _array[temp] = std::move(v);
        _tag[temp] = key;
    }

    void OBJ_ARRAY::push(std::string &key, OBJ_VALUE &&v)
    {

        unsigned long long temp = hash_objkey(key);

        _array[temp] = std::move(v);
        _tag[temp] = key;
    }

    void OBJ_ARRAY::push(std::string &&key, OBJ_VALUE &v)
    {

        unsigned long long temp = hash_objkey(key);
        _tag[temp] = key;
        _array[temp] = v;
    }
    void OBJ_ARRAY::push(std::string &&key, OBJ_VALUE &&v)
    {
        unsigned long long temp = hash_objkey(key);
        _array[temp] = std::move(v);
        _tag[temp] = key;
    }

    void OBJ_ARRAY::push(unsigned long long temp, const OBJ_VALUE &v)
    {

        _array[temp] = v;
    }
    bool OBJ_ARRAY::isset(unsigned long long temp)
    {
        if (_array.find(temp) != _array.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool OBJ_ARRAY::isset(const std::string &key)
    {
        unsigned long long temp;

        temp = hash_objkey(key);

        if (_array.find(temp) != _array.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool OBJ_ARRAY::isset(const std::string &&key)
    {
        unsigned long long temp;

        temp = hash_objkey(key);

        if (_array.find(temp) != _array.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool OBJ_ARRAY::is_numarray() const
    {

        unsigned int len;
        len = _array.size();

        for (long long i = 0; i < len; i++)
        {
            if (_array.find(i) == _array.end())
            {
                return false;
            }
        }
        return true;
    }

    bool OBJ_ARRAY::unset(unsigned long long temp)
    {
        auto iter = _array.find(temp);
        if (iter != _array.end())
        {
            _array.erase(iter);
            auto itertemp = _tag.find(temp);
            if (itertemp != _tag.end())
            {
                _tag.erase(itertemp);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool OBJ_ARRAY::unset(const std::string &key)
    {
        unsigned long long temp;
        temp = hash_objkey(key);
        auto iter = _array.find(temp);
        if (iter != _array.end())
        {
            _array.erase(iter);
            auto itertemp = _tag.find(temp);
            if (itertemp != _tag.end())
            {
                _tag.erase(itertemp);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    bool OBJ_ARRAY::unset(const std::string &&key)
    {
        unsigned long long temp;
        temp = hash_objkey(key);
        auto iter = _array.find(temp);
        if (iter != _array.end())
        {
            _array.erase(iter);
            auto itertemp = _tag.find(temp);

            if (itertemp != _tag.end())
            {
                _tag.erase(itertemp);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    void OBJ_ARRAY::clear()
    {

        std::map<unsigned long long, OBJ_VALUE> empty;
        _array.clear();
        std::swap(_array, empty);
    }
    std::string OBJ_ARRAY::getkeyname(unsigned long long temp) const
    {
        auto itertemp = _tag.find(temp);
        if (itertemp != _tag.end())
        {
            return itertemp->second;
        }
        else
        {

            auto iter = _array.find(temp);
            if (iter != _array.end())
            {
                return std::to_string(temp);
            }
            else
            {
                return "";
            }
        }
    }
    bool OBJ_ARRAY::istag(unsigned long long temp) const
    {
        auto itertemp = _tag.find(temp);
        if (itertemp != _tag.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    OBJ_VALUE OBJ_ARRAY::operator+(int v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }
        OBJ_VALUE vv(std::move(v));
        temp.push(std::move(vv));

        return temp;
    }
    OBJ_VALUE OBJ_ARRAY::operator+(float v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }
        OBJ_VALUE vv(std::move(v));
        temp.push(std::move(vv));

        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator+(char v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }
        OBJ_VALUE vv(std::move(v));
        temp.push(std::move(vv));

        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator+(std::string v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }
        OBJ_VALUE vv(std::move(v));
        temp.push(std::move(vv));

        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator+(OBJ_VALUE &v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v.as_array())
        {
            temp[v.getkeyname(first)] = second;
        }
        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator+(OBJ_ARRAY &v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v)
        {
            temp[v.getkeyname(first)] = second;
        }
        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator-(OBJ_ARRAY &v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v)
        {
            if (temp.isset(first))
            {
                temp.unset(first);
            }
        }
        return temp;
    }

    OBJ_VALUE OBJ_ARRAY::operator-(OBJ_VALUE &v)
    {
        OBJ_VALUE temp;
        temp.set_array();

        for (auto [first, second] : _array)
        {
            temp[getkeyname(first)] = second;
        }

        for (auto [first, second] : v.as_array())
        {
            if (temp.isset(first))
            {
                temp.unset(first);
            }
        }
        return temp;
    }

    bool save_json(const std::string &filename, const OBJ_VALUE &v)
    {

        FILE *f = fopen(filename.c_str(), "wb");
        if (f == NULL)
        {
            return false;
        }
        value_write(f, v);
        fclose(f);
        return true;
    }
    bool save_json(const std::string &&filename, const OBJ_VALUE &v)
    {

        FILE *f = fopen(filename.c_str(), "wb");
        if (f == NULL)
        {
            return false;
        }
        value_write(f, v);
        fclose(f);
        return true;
    }
    bool value_write(FILE *f, const OBJ_VALUE &v)
    {
        unsigned char type;
        unsigned int objsize = 0;
        //unsigned value_pos = ftell(f);

        type = v.type();
        fwrite(&type, sizeof(type), 1, f);
        // n=fwrite(objsize,sizeof(objsize),1,f);
        long long int tint;
        long double ffloat;
        unsigned slen;
        std::string str;
        switch (type)
        {

        case INT:
            tint = (long long int)v;
            fwrite(&tint, 1, sizeof(tint), f);

            break;
        case FLOAT:
            ffloat = (long double)v;
            fwrite(&ffloat, 1, sizeof(ffloat), f);

            break;
        case NIL:
            objsize += 0;
            break;

        case STRING:
            str = (std::string)v;

            slen = str.length();
            fwrite(&slen, sizeof(slen), 1, f);
            fwrite(str.data(), str.length(), 1, f);

            break;
        case ARRAY:

            array_write(f, v.as_array(), objsize);
            break;
        }
        return true;
    }
    bool array_write(FILE *f, const OBJ_ARRAY &a, unsigned int &length)
    {
        unsigned int objsize = a.size();

        fwrite(&objsize, sizeof(objsize), 1, f);

        for (auto [first, second] : a)
        {
            unsigned long long key = first;
            fwrite(&key, sizeof(key), 1, f);
            value_write(f, second);
        }
        return true;
    }

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
                case 0x22: //"
                    ptext[j] = 0x5c;
                    j++;
                    ptext[j] = 0x22;
                    break;
                case 0x60: //''
                    // ptext[j]=0x5c;
                    // j++;
                    ptext[j] = 0x60;
                    break;
                case 0x27: //'\'
                    // ptext[j]=0x5c;
                    // j++;
                    ptext[j] = 0x27;
                    break;
                case 0x2f: //\/
                    // ptext[j]=0x5c;
                    // j++;
                    ptext[j] = '/';
                    break;
                case 0x08: //\b
                           // ptext[j]=0x5c;
                    // j++;
                    // ptext[j]='b';
                    break;
                case 0x0c: //\f
                           // ptext[j]=0x5c;
                    // j++;
                    // ptext[j]='f';
                    break;
                case 0x0a: //\n
                    ptext[j] = 0x5c;
                    j++;
                    ptext[j] = 'n';
                    break;
                case 0x0d: //\r
                    ptext[j] = 0x5c;
                    j++;
                    ptext[j] = 'r';
                    break;
                case 0x09: //\t
                    ptext[j] = 0x5c;
                    j++;
                    ptext[j] = 't';
                    break;
                case 0x5c: //
                    ptext[j] = 0x5c;
                    j++;
                    ptext[j] = 0x5c;
                    break;
                default:
                    ptext[j] = c;
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

                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
                    cp[0] = str[t];

                    ptext[j] = cp[0];
                    j++;
                    ptext[j] = cp[1];
                    j++;
                    temp = (b1 << 6) + (b2 & 0x3F);
                    // numtohex(temp,cp);
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
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
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
                    cp[0] = str[t];

                    ptext[j] = cp[0];
                    j++;
                    ptext[j] = cp[1];
                    j++;
                    temp = (b2 << 6) + (b3 & 0x3F);
                    // numtohex(temp,cp);
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
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
                    pOutput = (unsigned char *)&out;
                    *pOutput = (b3 << 6) + (b4 & 0x3F);
                    *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
                    *(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
                    untext = (((((out - 0x10000) >> 10) | 0xD800) << 16) | (((out - 0x10000) & 0x3FF) | 0xDC00));

                    pOutput = (unsigned char *)&untext;
                    // 高端字节在前
                    temp = pOutput[3];
                    // numtohex(temp,cp);
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
                    cp[0] = str[t];

                    ptext[j] = cp[0];
                    j++;
                    ptext[j] = cp[1];
                    j++;
                    temp = pOutput[2];
                    // numtohex(temp,cp);
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
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
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
                    cp[0] = str[t];

                    ptext[j] = cp[0];
                    j++;
                    ptext[j] = cp[1];
                    j++;
                    temp = pOutput[0];

                    // numtohex(temp,cp);
                    t = temp & 0xF;
                    cp[1] = str[t];
                    temp = temp >> 4;
                    t = temp & 0xF;
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
    std::string OBJ_ARRAY::to_json()
    {

        std::stringstream os;

        unsigned int n = 0;

        if (is_numarray())
        {

            os << "[";

            n = 0;
            for (auto [first, second] : _array)
            {

                if (n > 0)
                {
                    os << ",";
                }
                n++;

                if (second.is_num())
                {
                    if (second.is_float())
                    {
                        os << std::to_string(second.as_float());
                    }
                    else
                    {
                        os << std::to_string(second.as_int());
                    }
                }
                else if (second.is_string())
                {
                    os << '"' << JSON_UTF8_TO_ASCII(second.as_string()) << '"';
                }
                else if (second.is_array())
                {
                    os << second.to_json();
                }
                else if (second.is_null())
                {
                    os << "NULL";
                }
            }

            os << "]";
        }
        else
        {

            os << "{";

            n = 0;
            for (auto [first, second] : _array)
            {

                if (n > 0)
                {
                    os << ",";
                }
                n++;

                if (_tag.find(first) != _tag.end())
                {
                    os << '"' << JSON_UTF8_TO_ASCII(getkeyname(first)) << '"' << ':';
                }
                else
                {
                    os << '"' << std::to_string(first) << '"' << ':';
                }

                if (second.is_num())
                {
                    if (second.is_float())
                    {
                        os << std::to_string(second.as_float());
                    }
                    else
                    {
                        os << std::to_string(second.as_int());
                    }
                }
                else if (second.is_string())
                {
                    os << '"' << JSON_UTF8_TO_ASCII(second.as_string()) << '"';
                }
                else if (second.is_array())
                {
                    os << second.to_json();
                }
                else if (second.is_null())
                {
                    os << "NULL";
                }
            }

            os << "}";
        }
        return os.str();
    }
    std::string OBJ_VALUE::to_json()
    {

        std::stringstream os;

        switch (type())
        {

        case INT:
            os << int_v;
            break;

        case FLOAT:

        {

            std::stringstream stream;
            std::string temp;

            stream.precision(15);
            // stream << std::fixed;
            stream << float_v;

            temp = stream.str();
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
        case NIL:
            os << "null";
            break;

        case STRING:
            os << '"' << JSON_UTF8_TO_ASCII(string_v) << '"';
            break;

        case ARRAY:

            os << array_v.to_json();
            break;
        }
        return os.str();
    }

    std::string JSON_VALUE(std::string &jsonstr,unsigned int &offset)
    {
        unsigned int j = 0;
        std::string temp;
        if (jsonstr[offset] == 0x3a)
        { //:
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

    std::string JSON_STR(std::string &jsonstr,unsigned int &offset)
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

            if (jsonstr[j] == 0x5c) //'\'
            {
                // 处理有斜杠情况 not slash
                switch (jsonstr[j + 1])
                {
                case 0x22: //"
                    str += 0x22;
                    j += 1;
                    break;
                case 0x5c: //'\'
                    str += 0x5c;
                    j += 1;
                    break;
                // case 0x2f: //\/
                case 0x62: //\b
                    str += 0x08;
                    j += 1;
                    break;
                case 0x66: //\f
                    str += 0x0c;
                    j += 1;
                    break;
                case 0x6e: //\n
                    str += 0x0a;
                    j += 1;
                    break;
                case 0x72: //\r
                    str += 0x0d;
                    j += 1;
                    break;
                case 0x74: //\t
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
                        if (jsonstr[j + 2] == 'd' && jsonstr[j + 6] == 0x5c && jsonstr[j + 7] == 'u' && jsonstr[j + 8] == 'd')
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
                            temp = 0x10000 | ((ca - 0xD800) << 10) | (cb - 0xDC00);
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
                default:
                    str += jsonstr[j + 1];
                    j += 1;
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

    int JSON_OBJ(std::string &jsonstr, OBJ_ARRAY &obj,unsigned int offset)
    {
        unsigned int i = 0, nobj = 0;
        std::string key, value;
        // OBJ_VALUE temp;
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
                OBJ_ARRAY newobj;
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
                OBJ_ARRAY newobj;
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

                    // obj[std::to_string(nobj)]=key;
                    obj[nobj] = key;
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
                    value = JSON_STR(jsonstr, i);

                    obj.push(key, value);
                    // obj[key]=value;
                }
                else if (jsonstr[i + 1] == 0x5b)
                {
                    OBJ_ARRAY newobj;
                    i++;
                    i = JSON_ARRAY(jsonstr, newobj, i);
                    if (key.empty())
                    {
                        // obj[std::to_string(nobj)]=newobj;
                        obj[nobj] = newobj;
                        nobj++;
                        key.clear();
                    }
                    else
                    {
                        // obj[key]=newobj;
                        obj.push(key, newobj);
                        key.clear();
                    }
                }
                else if (jsonstr[i + 1] == 0x7b)
                {
                    // 进入对象
                    i++;
                    OBJ_ARRAY newobj;
                    i = JSON_OBJ(jsonstr, newobj, i);

                    if (key.empty())
                    {
                        // obj[std::to_string(nobj)]=newobj;
                        obj[nobj] = newobj;
                        nobj++;
                    }
                    else
                    {
                        // obj[key]=newobj;

                        obj.push(key, newobj);
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
                        // obj[key]=1;
                        OBJ_VALUE tv(1);
                        obj.push(key, tv);
                    }
                    else if (value == "false" || value == "FALSE")
                    {
                        // obj[key]=0;
                        OBJ_VALUE tv(0);
                        obj.push(key, tv);
                    }
                    else if (value == "null" || value == "NULL")
                    {
                        OBJ_VALUE tv;
                        // obj[key]=tv;
                        obj.push(key, tv);
                    }
                    else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos || value.find('E') != std::string::npos)
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
                        // obj[key]=tc;
                        OBJ_VALUE tv(tc);
                        obj.push(key, tv);
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
                        // obj[key]=tc;
                        OBJ_VALUE tv(tc);
                        obj.push(key, tv);
                    }
                }
                key.clear();
                value.clear();
            }
        }
        return i;
    }

    int JSON_ARRAY(std::string &jsonstr, OBJ_ARRAY &obj,unsigned int offset)
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
                OBJ_ARRAY newobj;
                i = JSON_ARRAY(jsonstr, newobj, i);

                obj.push(newobj);
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
                        obj.push(1);
                    }
                    else if (value == "false" || value == "FALSE")
                    {
                        obj.push(0);
                    }
                    else if (value == "null" || value == "NULL")
                    {
                        OBJ_VALUE tv;
                        obj.push(tv);
                    }
                    else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos || value.find('E') != std::string::npos)
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
                        obj.push(tc);
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
                        obj.push(tc);
                    }
                }
                value.clear();
                key.clear();
                break;
            }
            else if (jsonstr[i] == 0x7b)
            {
                // 进入对象
                OBJ_ARRAY newobj;

                i = JSON_OBJ(jsonstr, newobj, i);

                obj.push(newobj);
                key.clear();
            }
            else if (jsonstr[i] == 0x22)
            {
                // 遇到字符,里面肯定没有下一层了
                key = JSON_STR(jsonstr, i);
                obj.push(key);
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
                        obj.push(1);
                    }
                    else if (value == "false" || value == "FALSE")
                    {
                        obj.push(0);
                    }
                    else if (value == "null" || value == "NULL")
                    {
                        OBJ_VALUE tv;
                        obj.push(tv);
                    }
                    else if (value.find('.') != std::string::npos || value.find('e') != std::string::npos || value.find('E') != std::string::npos)
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
                        obj.push(tc);
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
                        obj.push(tc);
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

    void OBJ_VALUE::from_json(std::string &&jsonstr)
    {
        int offset = 0;
        type_t = ARRAY;
        // 分出数组或对象 消除前面空格
        for (unsigned int i = 0; i < jsonstr.length(); i++)
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

            JSON_OBJ(jsonstr, array_v, offset);
        }
        else if (jsonstr[offset] == 0x5b)
        {
            // 数组情况

            JSON_ARRAY(jsonstr, array_v, offset);
        }
    }

    void OBJ_VALUE::from_json(std::string &jsonstr)
    {

        int offset = 0;
        type_t = ARRAY;
        // 分出数组或对象 消除前面空格
        for (unsigned int i = 0; i < jsonstr.length(); i++)
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

            JSON_OBJ(jsonstr, array_v, offset);
        }
        else if (jsonstr[offset] == 0x5b)
        {
            // 数组情况

            JSON_ARRAY(jsonstr, array_v, offset);
        }
    }

    OBJ_ARRAY from_json(std::string &&jsonstr)
    {
        OBJ_ARRAY obj;
        int offset = 0;
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
            OBJ_ARRAY arr;
            JSON_ARRAY(jsonstr, arr, offset);
            // obj["obj"]=arr;

            return arr;
        }

        return obj;
    }

    OBJ_ARRAY from_json(std::string &jsonstr)
    {
        OBJ_ARRAY obj;
        int offset = 0;
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
            OBJ_ARRAY arr;
            JSON_ARRAY(jsonstr, arr, offset);
            return arr;
        }

        return obj;
    }

}

std::ostream &operator<<(std::ostream &os, const http::OBJ_VALUE &v)
{
    switch (v.type())
    {

    case http::INT:
        os << (long long)v;
        break;

    case http::FLOAT:

    {

        std::stringstream stream;
        std::string temp;

        stream.precision(15);
        // stream << std::fixed;
        stream << v.as_float();

        temp = stream.str();
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
    case http::NIL:
        os << "null";
        break;

    case http::STRING:
        os << '"' << http::JSON_UTF8_TO_ASCII(v.as_string()) << '"';
        break;

    case http::ARRAY:
        os << v.as_array();
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const http::OBJ_ARRAY &a)
{
    unsigned int n = 0;

    if (a.is_numarray())
    {
        os << "[";

        n = 0;
        for (auto [first, second] : a)
        {

            if (n > 0)
            {
                os << ",";
            }
            n++;

            if (second.is_num())
            {
                if (second.is_float())
                {
                    os << std::to_string(second.as_float());
                }
                else
                {
                    os << std::to_string(second.as_int());
                }
            }
            else if (second.is_string())
            {
                os << '"' << http::JSON_UTF8_TO_ASCII(second.as_string()) << '"';
            }
            else if (second.is_array())
            {
                os << second.as_array();
            }
            else if (second.is_null())
            {
                os << "NULL";
            }
        }

        os << "]";
    }
    else
    {

        os << "{";

        n = 0;
        for (auto [first, second] : a)
        {

            if (n > 0)
            {
                os << ",";
            }
            n++;

            if (a.istag(first))
            {
                os << '"' << http::JSON_UTF8_TO_ASCII(a.getkeyname(first)) << '"' << ':';
            }
            else
            {

                os << '"' << std::to_string(first) << '"' << ':';
            }

            if (second.is_num())
            {
                if (second.is_float())
                {
                    os << std::to_string(second.as_float());
                }
                else
                {
                    os << std::to_string(second.as_int());
                }
            }
            else if (second.is_string())
            {
                os << '"' << http::JSON_UTF8_TO_ASCII(second.as_string()) << '"';
            }
            else if (second.is_array())
            {
                os << second.as_array();
            }
            else if (second.is_null())
            {
                os << "NULL";
            }
        }

        os << "}";
    }
    return os;
}
