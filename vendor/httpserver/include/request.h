#ifndef HTTP_OBJ_HTTP_H
#define HTTP_OBJ_HTTP_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <vector>
#include <list>
#include <functional>

namespace http
{
    enum OBJ_TYPE
    {
        STRING,
        INT,
        FLOAT,
        ARRAY,
        NIL
    };
    union OBJ_KEY
    {
        unsigned long long ikey = 0;
        unsigned char ckey[8];
    };
    class OBJ_VALUE;
    class OBJ_ARRAY
    {
    public:
        OBJ_ARRAY();
        ~OBJ_ARRAY();
        OBJ_ARRAY(const OBJ_ARRAY &a);
        OBJ_ARRAY(OBJ_ARRAY &&a);

        // OBJ_ARRAY(std::initializer_list<OBJ_VALUE> nsl);
        OBJ_ARRAY(std::initializer_list<std::string> nsl);
        OBJ_ARRAY(std::initializer_list<std::map<unsigned long long, OBJ_VALUE>::value_type> nsl);

        OBJ_ARRAY &operator=(const OBJ_ARRAY &a);
        OBJ_ARRAY &operator=(OBJ_ARRAY &&a);

        OBJ_VALUE &operator[](const std::string &key);
        OBJ_VALUE &operator[](const std::string &&key);
        const OBJ_VALUE &operator[](const std::string &key) const;

        OBJ_VALUE &operator[](unsigned long long i);
        // const OBJ_VALUE& operator[] (unsigned long long i) const;

        OBJ_VALUE operator+(OBJ_VALUE &v);
        OBJ_VALUE operator+(OBJ_ARRAY &v);
        OBJ_VALUE operator+(std::string v);
        OBJ_VALUE operator+(char v);
        OBJ_VALUE operator+(int v);
        OBJ_VALUE operator+(float v);

        OBJ_VALUE operator-(OBJ_VALUE &v);
        OBJ_VALUE operator-(OBJ_ARRAY &v);

        std::map<unsigned long long, OBJ_VALUE>::iterator begin();

        std::map<unsigned long long, OBJ_VALUE>::iterator end();

        std::map<unsigned long long, OBJ_VALUE>::const_iterator begin() const;
        std::map<unsigned long long, OBJ_VALUE>::const_iterator end() const;

        void push(unsigned long long, std::string &v);
        void push(unsigned long long, std::string &&v);

        void push(std::string &, OBJ_VALUE &v);
        void push(std::string &, OBJ_VALUE &&v);
        void push(std::string &, OBJ_ARRAY &&v);
        void push(std::string &&, OBJ_VALUE &v);
        void push(std::string &&, OBJ_VALUE &&v);

        void push(const OBJ_VALUE &n);
        void push(const OBJ_VALUE &&n);

        void push(unsigned long long i, const OBJ_VALUE &n);

        size_t size() const;
        bool isset(unsigned long long n);
        bool isset(const std::string &key);
        bool isset(const std::string &&key);

        bool unset(unsigned long long n);
        bool unset(const std::string &key);
        bool unset(const std::string &&key);
        void clear();

        bool is_int() const { return false; }
        bool is_float() const { return false; }
        bool is_num() const { return false; }
        bool is_string() const { return false; }
        bool is_array() const { return true; }
        bool is_null() const { return false; }

        bool isval() const { return false; }

        std::string getkeyname(unsigned long long n) const;
        bool istag(unsigned long long n) const;
        bool is_numarray() const;

        std::string to_json();
        // OBJ_VALUE   fromjson(const std::string& v);
        // OBJ_VALUE   fromjson(const std::string&& v);
    protected:
        std::map<unsigned long long, OBJ_VALUE> _array;
        std::map<unsigned long long, std::string> _tag;
    };

    class OBJ_VALUE
    {
    public:
        OBJ_VALUE();
        ~OBJ_VALUE();
        OBJ_VALUE(OBJ_VALUE &&v);
        OBJ_VALUE(const OBJ_VALUE &v);

        OBJ_VALUE(const long long i);
        OBJ_VALUE(const unsigned long long i);
        OBJ_VALUE(const long i);
        OBJ_VALUE(const unsigned int i);
        OBJ_VALUE(const int i);
        OBJ_VALUE(const bool i);
        OBJ_VALUE(const long double f);
        OBJ_VALUE(const double f);

        OBJ_VALUE(const char *s);
        OBJ_VALUE(std::string &&s);
        OBJ_VALUE(const std::string &s);

        OBJ_VALUE(const OBJ_ARRAY &a);
        OBJ_VALUE(OBJ_ARRAY &&a);

        OBJ_VALUE(std::initializer_list<std::string> nsl);
        // OBJ_VALUE(std::initializer_list<std::map<unsigned long long, OBJ_VALUE>::value_type> nsl);

        OBJ_VALUE(std::initializer_list<std::map<unsigned long long, OBJ_VALUE>::value_type> nsl);
        OBJ_VALUE &operator[](const std::string &key);
        OBJ_VALUE &operator[](const std::string &&key);
        const OBJ_VALUE &operator[](const std::string &key) const;

        OBJ_VALUE &operator[](unsigned long long i);
        const OBJ_VALUE &operator[](unsigned long long i) const;

        OBJ_VALUE &operator=(const OBJ_VALUE &v);
        OBJ_VALUE &operator=(OBJ_VALUE &&v);

        OBJ_VALUE &get_self(){ return *this;  };

        OBJ_VALUE operator+(OBJ_VALUE &v);
        OBJ_VALUE operator+(OBJ_ARRAY &v);
        OBJ_VALUE operator+(std::string v);
        OBJ_VALUE operator+(char v);
        OBJ_VALUE operator+(int v);
        OBJ_VALUE operator+(float v);

        OBJ_VALUE operator-(OBJ_VALUE &v);
        OBJ_VALUE operator-(OBJ_ARRAY &v);

        OBJ_TYPE type() const
        {
            return type_t;
        }

        /** Cast operator for float */
        explicit operator long double() const { return float_v; }
        explicit operator float() const { return float_v; }
        /** Cast operator for int */
        explicit operator long long int() const { return int_v; }
        explicit operator int() const { return int_v; }

        /** Cast operator for string */
        explicit operator std::string() const
        {
            switch (type())
            {
                // case INT:
                //     return std::to_string(int_v);
                //     break;

                // case FLOAT:
                //     return std::to_string(float_v);
                //     break;

                // case NIL:
                //     return "";
                //     break;

                // case STRING:
                //     return string_v;
                //     break;

            case ARRAY:
                return "obj";
                break;
            default:
                return string_v;
            }
        }

        /** Cast operator for Object */
        explicit operator OBJ_ARRAY() const { return array_v; }

        /** Cast operator for Object */
        OBJ_ARRAY as_array() const { return array_v; }

        /** Cast operator for float */
        long double as_float() const { return float_v; }

        /** Cast operator for int */
        long long int as_int() const { return int_v; }

        /** Cast operator for string */
        std::string as_string() const { return string_v; }
        std::string *ptr_string() { return &string_v; }
        std::string &ref() { return std::ref(string_v); }
        std::string to_string() const
        {

            switch (type())
            {
            case INT:
                return std::to_string(int_v);
                break;

            case FLOAT:
                return std::to_string(float_v);
                break;

            case NIL:
                return "";
                break;

            case STRING:
                return string_v;
                break;

            case ARRAY:
                return "obj";
                break;
            default:
                return string_v;
            }
        }
        long long to_int() const
        {
            long long temp = 0;
            switch (type())
            {
            case INT:
                return int_v;
                break;

            case FLOAT:
                return float_v;
                break;

            case NIL:
                return 0;
                break;

            case STRING:

                for (std::size_t i = 0; i < string_v.size(); i++)
                {
                    if (string_v[i] == 0x20)
                    {
                        break;
                    }
                    if (string_v[i] >= '0' && string_v[i] <= '9')
                    {
                        temp = temp * 10 + (string_v[i] - '0');
                    }
                    else
                    {
                        break;
                    }
                    if (i > 20)
                    {
                        break;
                    }
                }
                return temp;
                break;

            case ARRAY:
                return 0;
                break;
            default:
                return 0;
            }
        }
        bool is_int() const { return type_t == INT; }
        bool is_float() const { return type_t == FLOAT; }
        bool is_num() const { return type_t == INT || type_t == FLOAT; }
        bool is_string() const { return type_t == STRING; }
        bool is_array() const { return type_t == ARRAY; }
        bool is_null() const { return type_t == NIL; }

        bool isval() const { return true; }

        OBJ_VALUE &set_float()
        {
            type_t = FLOAT;
            return *this;
        }
        OBJ_VALUE &set_int()
        {
            type_t = INT;
            return *this;
        }
        OBJ_VALUE &set_string()
        {
            type_t = STRING;
            return *this;
        }
        OBJ_VALUE &set_array()
        {
            type_t = ARRAY;
            return *this;
        }
        OBJ_VALUE &set_null()
        {
            type_t = NIL;
            return *this;
        }

        void push(unsigned long long, std::string &v);
        void push(unsigned long long, std::string &&v);

        void push(std::string &, OBJ_VALUE &v);
        void push(std::string &, OBJ_VALUE &&v);

        void push(std::string &&, OBJ_VALUE &v);
        void push(std::string &&, OBJ_VALUE &&v);

        void push(const OBJ_VALUE &n);
        void push(const OBJ_VALUE &&n);

        void push(unsigned long long i, const OBJ_VALUE &n);

        bool isset(unsigned long long n);
        bool isset(const std::string &key);
        bool isset(const std::string &&key);

        bool unset(unsigned long long n);
        bool unset(const std::string &key);
        bool unset(const std::string &&key);

        void clear();

        size_t size() const;

        std::string tag();
        std::string tag(const std::string &v);
        std::string tag(const std::string &&v);
        std::string getkeyname(unsigned long long n);
        std::string to_json();

        void from_json(std::string &v);
        void from_json(std::string &&v);

    protected:
        long double float_v;
        long long int_v = 0;
        std::string string_v;

        OBJ_ARRAY array_v;

        OBJ_TYPE type_t;
    };
    unsigned long long hash_objkey(const std::string &key);

    bool save_json(const std::string &filename, const OBJ_VALUE &v);
    bool save_json(const std::string &&filename, const OBJ_VALUE &v);
    bool value_write(FILE *, const OBJ_VALUE &v);
    bool array_write(FILE *, const OBJ_ARRAY &v, unsigned int &length);

    // std::string OBJ_ARRAY::tojson();

    OBJ_ARRAY from_json(std::string &);
    OBJ_ARRAY from_json(std::string &&);
    std::string JSON_UTF8_TO_ASCII(const std::string &source);
    std::string JSON_STR(std::string &,unsigned int &);
    std::string JSON_VALUE(std::string &,unsigned int &);
    int JSON_OBJ(std::string &, OBJ_ARRAY &,unsigned int);
    int JSON_ARRAY(std::string &, OBJ_ARRAY &,unsigned int);

}
std::ostream &operator<<(std::ostream &, const http::OBJ_VALUE &);
std::ostream &operator<<(std::ostream &, const http::OBJ_ARRAY &);
#endif
