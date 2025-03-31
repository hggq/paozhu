/*
 * paozhu micro obj rebuild
 * author Huang ziquan (黄自权)
 * date 2025-03-21
 */
#ifndef HTTP_OBJ_HTTP_H
#define HTTP_OBJ_HTTP_H
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <map>
#include <list>
#include <type_traits>
#include <utility>
#include <vector>
#include <concepts>
#include <tuple>
#include <string_view>

namespace http
{
enum class obj_type : unsigned char
{
    OBJECT    = 0,
    ARRAY     = 1,
    BOOL      = 2,
    STRING    = 3,
    FLOAT     = 4,
    DOUBLE    = 5,
    INT       = 6,
    UINT      = 7,
    LONG      = 8,
    ULONG     = 9,
    link_info = 10,//uid num
    card_info = 11,//tatal_val item_val
    list_info = 12,//mid isview ishome isjin istop
    node_info = 13,//node_id node_num
    item_info = 14,//project_id tag
    NIL       = 15
};

struct obj_array;
struct obj_t;

struct obj_val
{
    union
    {
        char *str;
        char name[8];
        long long lval;
        unsigned long long uval;
        double dval;
        unsigned long long mask_val;
        struct
        {
            bool isbool;
            unsigned short sval;
            unsigned int uival;
        };
        //list_info
        struct
        {
            bool isview;
            bool ishome;
            bool isjin;
            bool istop;
            unsigned int mid;
        };
        struct
        {
            float fval;
            int ival;
        };
        //link_info
        struct
        {
            unsigned int uid;
            unsigned int num;
        };
        //node_info
        struct
        {
            unsigned int node_num : 24;
            unsigned long long node_id : 40;
        };
        //item_info
        struct
        {
            unsigned int item_id;
            unsigned char tag[4];
        };
        //card_info
        struct
        {
            float total_val;
            float item_val;
        };
        obj_t *obj;
        obj_array *array_val;
    };
    unsigned int number : 24 = 0;
    unsigned char cataid     = 0;
    unsigned int length : 24 = 0;

  private:
    obj_type _val_type : 8 = obj_type::NIL;

  public:
    std::string to_string();
    bool to_bool();
    long long to_int();
    double to_float();
    obj_type get_type() const;
    void set_type(obj_type _type);

    float str_to_float();
    double str_to_double();
    int str_to_int();
    long long str_to_long();

    unsigned int str_to_uint();
    unsigned long long str_to_ulong();

    obj_val &operator[](const std::string &key);
    obj_val &operator[](std::string &&key);

    obj_val &operator[](unsigned int index);

    unsigned int size();

    void append(const std::string &v);
    void append(const char *_str, unsigned int str_length);
    std::string substr(int a, int b);
    std::string substr(int a);

    void clear();
    obj_val();
    obj_val(std::nullptr_t i);
    obj_val(bool i);
    obj_val(long long i);
    obj_val(unsigned long long i);
    obj_val(short i);
    obj_val(unsigned short i);
    obj_val(int i);
    obj_val(unsigned int i);
    obj_val(long i);
    obj_val(unsigned long i);
    obj_val(float i);
    obj_val(double i);
    obj_val(long double i);
    obj_val(const std::string &_str);
    obj_val(const char *_str);
    obj_val(const char *_str, unsigned int str_length);
    obj_val(std::string_view _str);
    obj_val(std::string &&_str);
    obj_val(const obj_t &v);
    obj_val(obj_t &&v);
    obj_val(const obj_val &v);
    obj_val(obj_val &&v);

    obj_val(const obj_array &v);
    obj_val(obj_array &&v);

    bool operator==(const obj_val &v);
    bool operator!=(const obj_val &v);

    bool operator>=(const obj_val &v);
    bool operator>(const obj_val &v);

    // bool operator<=(const obj_val &v);
    // bool operator<(const obj_val &v);

    obj_val &operator=(const std::vector<float> &v);
    obj_val &operator=(const std::vector<long long> &v);
    obj_val &operator=(const std::vector<std::string> &v);

    obj_val &operator=(const std::map<std::string, int> &v);
    obj_val &operator=(const std::map<std::string, unsigned int> &v);
    obj_val &operator=(const std::map<std::string, std::string> &v);

    obj_val &operator=(obj_val &&v);
    obj_val &operator=(const obj_val &v);
    obj_val &operator=(std::string &&_str);
    obj_val &operator=(const std::string &_str);
    obj_val &operator=(std::string_view str);
    obj_val &operator=(const char *str);

    obj_val &operator=(long long i);
    obj_val &operator=(unsigned long long i);
    obj_val &operator=(short i);
    obj_val &operator=(unsigned short i);
    obj_val &operator=(int i);
    obj_val &operator=(unsigned int i);
    obj_val &operator=(long i);
    obj_val &operator=(unsigned long i);
    obj_val &operator=(float i);
    obj_val &operator=(double i);
    obj_val &operator=(long double i);
    obj_val &operator=(bool i);

    obj_val &operator+(const std::string &v);

    obj_val &operator+(long long i);
    obj_val &operator+(unsigned long long i);
    obj_val &operator+(int i);
    obj_val &operator+(unsigned int i);
    obj_val &operator+(long i);
    obj_val &operator+(unsigned long i);
    obj_val &operator+(float i);
    obj_val &operator+(double i);

    obj_val &operator-(long long i);
    obj_val &operator-(unsigned long long i);
    obj_val &operator-(int i);
    obj_val &operator-(unsigned int i);
    obj_val &operator-(long i);
    obj_val &operator-(unsigned long i);
    obj_val &operator-(float i);
    obj_val &operator-(double i);

    obj_val &push(const obj_val &v);
    obj_val &push(obj_val &&v);

    obj_val &push(const std::string &key, obj_val &&v);
    obj_val &push(const std::string &key, const obj_val &v);

    obj_val &find(const obj_val &v);
    obj_val &find(const std::string &v);

    obj_val &set_array();
    obj_val &set_obj();
    obj_val &set_object();
    obj_val &set_int();
    obj_val &set_long();
    obj_val &set_uint();
    obj_val &set_ulong();
    obj_val &set_number();
    obj_val &set_float();
    obj_val &set_double();
    obj_val &set_string();
    obj_val &set_bool();
    obj_val &set_null();

    obj_val &set_link_info();
    obj_val &set_card_info();
    obj_val &set_list_info();
    obj_val &set_node_info();
    obj_val &set_item_info();

    bool is_int() const { return _val_type == obj_type::INT; }
    bool is_uint() const { return _val_type == obj_type::UINT; }
    bool is_long() const { return _val_type == obj_type::LONG; }
    bool is_ulong() const { return _val_type == obj_type::ULONG; }

    bool is_integer() const { return _val_type == obj_type::INT || _val_type == obj_type::INT || _val_type == obj_type::UINT || _val_type == obj_type::LONG || _val_type == obj_type::ULONG; }

    bool is_float() const { return _val_type == obj_type::FLOAT; }
    bool is_double() const { return _val_type == obj_type::DOUBLE; }

    bool is_point() const { return _val_type == obj_type::FLOAT || _val_type == obj_type::DOUBLE; }

    bool is_num() const { return _val_type == obj_type::INT || _val_type == obj_type::INT || _val_type == obj_type::UINT || _val_type == obj_type::LONG || _val_type == obj_type::ULONG || _val_type == obj_type::FLOAT || _val_type == obj_type::DOUBLE; }
    bool is_number() const { return _val_type == obj_type::INT || _val_type == obj_type::INT || _val_type == obj_type::UINT || _val_type == obj_type::LONG || _val_type == obj_type::ULONG || _val_type == obj_type::FLOAT || _val_type == obj_type::DOUBLE; }
    bool is_real() const { return _val_type == obj_type::INT || _val_type == obj_type::INT || _val_type == obj_type::UINT || _val_type == obj_type::LONG || _val_type == obj_type::ULONG || _val_type == obj_type::FLOAT || _val_type == obj_type::DOUBLE; }

    bool is_string() const { return _val_type == obj_type::STRING; }
    bool is_array() const { return _val_type == obj_type::ARRAY; }
    bool is_null() const { return _val_type == obj_type::NIL; }
    bool is_bool() const { return _val_type == obj_type::BOOL; }
    bool is_obj() const { return _val_type == obj_type::OBJECT; }
    bool is_object() const { return _val_type == obj_type::OBJECT; }

    bool is_link_info() const { return _val_type == obj_type::link_info; }
    bool is_card_info() const { return _val_type == obj_type::card_info; }
    bool is_list_info() const { return _val_type == obj_type::list_info; }
    bool is_node_info() const { return _val_type == obj_type::node_info; }
    bool is_item_info() const { return _val_type == obj_type::item_info; }

    explicit operator double() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0.0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0.0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0.0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0.0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0.0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0.0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stod(name);
                    }
                    else
                    {
                        return std::stod(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:
            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0.0;
            break;
        case obj_type::ARRAY:
            return 0;
        default:
            return 0.0;
        }
        return 0.0;
    }
    explicit operator float() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0.0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0.0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0.0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0.0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0.0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0.0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stod(name);
                    }
                    else
                    {
                        return std::stod(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:
            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0.0;
            break;
        case obj_type::ARRAY:
            return 0;
            break;
        default:
            return 0.0;
        }
        return 0.0;
    }
    explicit operator long long() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stoll(name);
                    }
                    else
                    {
                        return std::stoll(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:

            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0;
            break;
        case obj_type::ARRAY:
            return 0;
            break;
        default:
            return 0;
        }
        return 0;
    }
    explicit operator unsigned long long() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stoull(name);
                    }
                    else
                    {
                        return std::stoull(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:
            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0;
        case obj_type::ARRAY:
            return 0;
            break;
        default:
            return 0;
        }
        return 0;
    }
    explicit operator int() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stol(name);
                    }
                    else
                    {
                        return std::stol(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:

            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0;
            break;
        default:
            return 0;
        }
        return 0;
    }
    explicit operator unsigned int() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return 0;
            break;
        case obj_type::BOOL:
            return isbool ? 1 : 0;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return 0;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return 0;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return 0;
                }
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return 0;
                }
            }

            {
                try
                {
                    if (length < 8)
                    {
                        return std::stoul(name);
                    }
                    else
                    {
                        return std::stoul(str);
                    }
                }
                catch (const std::invalid_argument &e)
                {
                    return 0;
                }
                catch (const std::out_of_range &e)
                {
                    return 0;
                }
            }
            break;
        case obj_type::INT:
            return ival;
            break;
        case obj_type::UINT:
            return uival;
            break;
        case obj_type::LONG:

            return lval;
            break;
        case obj_type::ULONG:
            return uval;
            break;
        case obj_type::FLOAT:
            return fval;
            break;
        case obj_type::DOUBLE:
            return dval;
            break;
        case obj_type::OBJECT:
            return 0;
            break;
        default:
            return 0;
        }
        return 0;
    }
    explicit operator bool() const
    {
        switch (_val_type)
        {
        case obj_type::NIL:
            return false;
            break;
        case obj_type::BOOL:
            return isbool;
            break;
        case obj_type::STRING:
            if (length == 0)
            {
                return false;
            }
            if (length < 8 && length > 4)
            {
                if (name[0] == 'f' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return false;
                }
                else if (name[0] == 'F' && name[1] == 'a' && name[2] == 'l' && name[3] == 's' && name[4] == 'e')
                {
                    return false;
                }
                else if (name[0] == 'F' && name[1] == 'A' && name[2] == 'L' && name[3] == 'S' && name[4] == 'E')
                {
                    return false;
                }
                return true;
            }
            else if (length == 1)
            {
                if (name[0] == '0' || name[0] == 0x20)
                {
                    return false;
                }
            }

            if (length > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::INT:
            if (ival > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::UINT:
            if (uival > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::LONG:
            if (lval > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::ULONG:
            if (uval > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::FLOAT:
            if (fval > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::DOUBLE:
            if (dval > 0)
            {
                return true;
            }
            return false;
            break;
        case obj_type::OBJECT:
            return false;
            break;
        case obj_type::ARRAY:
            return false;
            break;
        default:
            return false;
        }
        return false;
    }
    explicit operator std::string() const
    {
        if (_val_type == obj_type::OBJECT)
        {
            return "{}";
        }
        else if (_val_type == obj_type::ARRAY)
        {
            return "[]";
        }
        else if (_val_type == obj_type::STRING)
        {
            std::string a_temp;
            if (length < 8)
            {
                for (unsigned int j = 0; j < length; j++)
                {
                    a_temp.push_back(name[j]);
                }
                return a_temp;
            }
            a_temp.append(str, length);
            return a_temp;
        }
        else if (_val_type == obj_type::BOOL)
        {
            if (isbool)
            {
                return "true";
            }
            else
            {
                return "false";
            }
        }
        else if (_val_type == obj_type::INT)
        {
            return std::to_string(ival);
        }
        else if (_val_type == obj_type::UINT)
        {
            return std::to_string(uival);
        }
        else if (_val_type == obj_type::FLOAT)
        {
            return std::to_string(fval);
        }
        else if (_val_type == obj_type::DOUBLE)
        {
            return std::to_string(dval);
        }
        else if (_val_type == obj_type::LONG)
        {
            return std::to_string(lval);
        }
        else if (_val_type == obj_type::ULONG)
        {
            return std::to_string(uval);
        }
        else if (_val_type == obj_type::NIL)
        {
            return "";
        }
        return "";
    }

    bool unset(const std::string &key);
    bool erase(const std::string &key);
    bool erase(const obj_val &key);

    // void set_keyname(const std::string &key);
    // std::string get_keyname();

    obj_val(const std::initializer_list<std::string> nsl);
    obj_val(const std::initializer_list<double> nsl);

    obj_val(const std::initializer_list<std::pair<std::string, double>> nsl);

    std::vector<obj_val>::iterator begin();
    std::vector<obj_val>::iterator end();
    std::vector<obj_val>::const_iterator cbegin() const;
    std::vector<obj_val>::const_iterator cend() const;

    std::vector<std::pair<std::string, obj_val>>::iterator obj_begin();
    std::vector<std::pair<std::string, obj_val>>::iterator obj_end();
    std::vector<std::pair<std::string, obj_val>>::const_iterator obj_cbegin() const;
    std::vector<std::pair<std::string, obj_val>>::const_iterator obj_cend() const;

    std::pair<std::string, obj_val> get_obj_val(unsigned int index);
    obj_val get_array_val(unsigned int index);

    std::pair<std::string, obj_val> &ref_obj_val(unsigned int index);
    obj_val &ref_array_val(unsigned int index);

    std::pair<std::string, obj_val> &cref_obj_val(unsigned int index) const;
    obj_val &cref_array_val(unsigned int index) const;

    std::vector<std::pair<std::string, obj_val>> get_obj();
    std::vector<obj_val> get_array();

    std::vector<std::pair<std::string, obj_val>> &ref_obj();
    std::vector<obj_val> &ref_array();
    std::vector<std::pair<std::string, obj_val>> &cref_obj() const;
    std::vector<obj_val> &cref_array() const;

    const char *c_str() const;
    char *str_data();
    bool reserve(unsigned int resize);
    unsigned int mb_strlen();
    std::string mb_substr(int begin_pos, int cut_size = 0);
    std::string_view str_view();
    std::string_view str_view(std::string_view default_val);
    std::string_view str_view(int a, int b = 0);

    bool isset(const std::string &key);

    void from_json(const std::string &json_str);
    std::string to_json();

    bool string_casecmp(std::string_view str1, std::string_view str2);
    std::string JSON_STRING(const std::string &jsonstr, unsigned int &offset);
    std::string JSON_VALUE(const std::string &jsonstr, unsigned int &offset);
    int JSON_OBJ(const std::string &json_val, obj_t &obj, unsigned int &offset);
    int JSON_ARRAY(const std::string &json_val, obj_array &obj, unsigned int &offset);
    std::string JSON_UTF8_TO_ASCII(const char *str, unsigned int length);

    std::vector<std::pair<std::string, obj_val>> &as_object();
    std::vector<obj_val> &as_array();
    std::string as_string();
    std::string as_string(std::string_view default_val);
    std::string get_string(std::string_view key, std::string_view default_val);
    std::string_view get_str_view(std::string_view key, std::string_view default_val);

    std::map<unsigned int, std::vector<unsigned int>> get_obj_key_index();
    obj_val &get_obj_val_index(std::string_view key, const std::map<unsigned int, std::vector<unsigned int>> &index_array);

    ~obj_val();

  private:
    std::string mb_substr_name(int begin_pos, int cut_size = 0);
};
struct obj_t
{
  public:
    std::vector<std::pair<std::string, obj_val>> _data;
};
struct obj_array
{
  public:
    // std::string key;
    std::vector<obj_val> _data;
};
}// namespace http
std::ostream &operator<<(std::ostream &, http::obj_val &);
#endif
