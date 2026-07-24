/*
 * paozhu micro obj rebuild
 * author Huang ziquan (黄自权)
 * date 2026-07-24
 */
#ifndef HTTP_OBJ_HTTP_H
#define HTTP_OBJ_HTTP_H
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <functional>

namespace http
{
#define SORT_ASC 0
#define SORT_DESC 1

constexpr size_t JSON_MAX_DEPTH = 128;
constexpr size_t JSON_MAX_STRING_LENGTH = 1024 * 1024;
constexpr size_t JSON_MAX_ARRAY_ELEMENTS = 65536;
constexpr size_t JSON_MAX_OBJECT_MEMBERS = 65536;
constexpr size_t JSON_MAX_SIZE = 10 * 1024 * 1024;

class json_parse_error : public std::runtime_error
{
public:
    explicit json_parse_error(const std::string &msg) : std::runtime_error(msg) {}
};

enum class obj_type : unsigned char
{
    OBJECT    = 0,
    ARRAY     = 1,
    BOOL      = 2,
    STRING    = 3,
    DOUBLE    = 4,
    INT       = 5,
    UINT      = 6,
    LONG      = 7,
    ULONG     = 8,
    NIL       = 9
};

struct obj_val;
struct obj_array;
struct obj_t;

std::string to_json_internal(const obj_val &val, unsigned int level);
std::string to_json_uncode_internal(const obj_val &val, unsigned int level);

struct obj_val
{
    std::string str_;
    union
    {
        long long lval;
        unsigned long long uval;
        double dval;
        unsigned long long mask_val;
        obj_t *obj;
        obj_array *array_val;
    };
  private:
    obj_type _val_type = obj_type::NIL;
    
    friend std::string http::to_json_internal(const obj_val &val, unsigned int level);
    friend std::string http::to_json_uncode_internal(const obj_val &val, unsigned int level);

  public:
    std::string to_string() const;
    std::string_view to_string_view() const;
    std::string to_escape();
    std::string to_trim();
    void trim();
    bool to_bool() const;
    long long to_int() const;
    double to_float() const;
    char to_char() const;
    obj_val &from_char(char c);
    obj_type get_type() const;
    void set_type(obj_type _type);

    float str_to_float();
    double str_to_double();
    int str_to_int();
    long long str_to_long();
    unsigned int str_to_uint();
    unsigned long long str_to_ulong();

    obj_val &operator[](std::string_view key);
    obj_val &operator[](unsigned int index);
    const obj_val &operator[](std::string_view key) const;
    const obj_val &operator[](unsigned int index) const;

    unsigned int size();

    void append(const std::string &v);
    void append(const char *_str, unsigned int str_length);
    std::string substr(int a, int b);
    std::string substr(int a);
    std::string filter(std::function<char(char)> func=nullptr);

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
    obj_val &set_double();
    obj_val &set_string();
    obj_val &set_bool();
    obj_val &set_null();

    bool is_int() const { return _val_type == obj_type::INT; }
    bool is_uint() const { return _val_type == obj_type::UINT; }
    bool is_long() const { return _val_type == obj_type::LONG; }
    bool is_ulong() const { return _val_type == obj_type::ULONG; }
    bool is_integer() const { return _val_type == obj_type::INT || _val_type == obj_type::UINT || _val_type == obj_type::LONG || _val_type == obj_type::ULONG; }
    bool is_double() const { return _val_type == obj_type::DOUBLE; }
    bool is_number() const { return is_integer() || is_double(); }
    bool is_string() const { return _val_type == obj_type::STRING; }
    bool is_array() const { return _val_type == obj_type::ARRAY; }
    bool is_null() const { return _val_type == obj_type::NIL; }
    bool is_bool() const { return _val_type == obj_type::BOOL; }
    bool is_object() const { return _val_type == obj_type::OBJECT; }
    bool is_obj() const { return _val_type == obj_type::OBJECT; }

    explicit operator double() const;
    explicit operator float() const;
    explicit operator long long() const;
    explicit operator unsigned long long() const;
    explicit operator int() const;
    explicit operator unsigned int() const;
    explicit operator bool() const;
    explicit operator std::string() const;

    bool unset(const std::string &key);
    bool erase(const std::string &key);
    bool erase(const obj_val &key);

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

    std::vector<std::pair<std::string, obj_val>> get_obj();
    std::vector<obj_val> get_array();

    const char *c_str() const;
    char *str_data();
    bool reserve(unsigned int resize);
    unsigned int mb_strlen();
    std::string mb_substr(int begin_pos, int cut_size = 0);
    std::string_view str_view();
    std::string_view str_view(std::string_view default_val);
    std::string_view str_view(int a, int b = 0);

    bool isset(const std::string &key);
    int casecmp(const std::string &str) const;

    void from_json(const std::string &json_str);
    std::string to_json();
    std::string to_json_uncode();

    std::vector<std::pair<std::string, obj_val>> &as_object();
    std::vector<obj_val> &as_array();
    const std::vector<std::pair<std::string, obj_val>> &ref_obj() const;
    const std::vector<obj_val> &ref_array() const;
    std::string as_string();
    std::string as_string(std::string_view default_val);

    void zip(const std::vector<std::string> &key, const std::vector<int> &val);
    void zip(const std::vector<std::string> &key, const std::vector<float> &val);
    void zip(const std::vector<std::string> &key, const std::vector<std::string> &val);
    void zip(const std::vector<std::string> &key, const obj_val &val);
    void zip(const obj_val &key, const obj_val &val);

    obj_val multi_sort(std::string_view key, unsigned char order);
    obj_val multi_sort(std::string_view key, unsigned char order, std::string_view key2, unsigned char order2);

    ~obj_val();
};
struct obj_t
{
  public:
    std::vector<std::pair<std::string, obj_val>> _data;
};
struct obj_array
{
  public:
    std::vector<obj_val> _data;
};
}// namespace http
std::ostream &operator<<(std::ostream &, http::obj_val &);
#endif
