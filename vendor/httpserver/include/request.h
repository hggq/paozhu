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
#include <stdexcept>
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
    // 标签只能由类自身或解析器写; 对外写标签可以造出「OBJECT + 空指针」这种非法状态
    void set_type(obj_type _type) { _val_type = _type; }
    void release_deep() noexcept;

    friend std::string http::to_json_internal(const obj_val &val, unsigned int level);
    friend std::string http::to_json_uncode_internal(const obj_val &val, unsigned int level);
    friend void from_json_internal(const std::string &json_str, obj_val &val, unsigned int &offset, unsigned int level);

  public:
    std::string to_string() const;
    std::string_view to_string_view() const;
    // 注意: 这不是通用净化器. 只按 MySQL addslashes 风格转义 \' \" \\ \0 \n \r \Z,
    // 不识别字符集(GBK/Big5 宽字节仍可注入), 对 PostgreSQL/SQLite 方言也不对,
    // 不处理 % _ 与 < > &. 入库请走 ORM 参数绑定, 不要拿它拼 SQL/HTML/shell.
    std::string to_sql_mysql_escape();
    // 旧名, 与 to_sql_mysql_escape() 是同一个实现(仍是 MySQL 方言那一套), 保留给既有调用点
    std::string to_escape() { return to_sql_mysql_escape(); }
    std::string to_trim();
    void trim();
    bool to_bool() const;
    long long to_int() const;
    double to_float() const;
    double to_double() const;
    char to_char() const;
    obj_val &from_char(char c);
    obj_type get_type() const { return _val_type; }

    float str_to_float();
    double str_to_double();
    int str_to_int();
    long long str_to_long();
    unsigned int str_to_uint();
    unsigned long long str_to_ulong();

    // 返回的引用指向内部 vector 的元素: 插入/删除可能搬动元素, 需要跨操作持有时先拷贝.
    // 数字下标对数组取位置, 对对象取十进制文本 key("1" 与 1 等价).
    obj_val &operator[](std::string_view key);
    obj_val &operator[](unsigned int index);
    const obj_val &operator[](std::string_view key) const;
    const obj_val &operator[](unsigned int index) const;

    // at() 是只读语义的严格版本: 不会隐式创建成员, 缺失/越界一律抛异常
    obj_val &at(std::string_view key);
    obj_val &at(unsigned int index);
    const obj_val &at(std::string_view key) const;
    const obj_val &at(unsigned int index) const;

    std::size_t size() const;

    void append(const std::string &v);
    void append(const char *_str, unsigned int str_length);
    std::string substr(int a, int b);
    std::string substr(int a);
    // 逐字节过滤: 回调返回 '\0' 丢弃该字节, 否则输出; 参数是无符号值, 可直接交给 toupper/tolower
    std::string filter(std::function<char(unsigned char)> func=nullptr);

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
    obj_val(obj_val &&v) noexcept;
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
    // 没有这个重载时 `x = nullptr` 会选中 operator=(const char*) 并在 strlen 上崩溃
    obj_val &operator=(std::nullptr_t);

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

    // 显式的追加/合并入口: operator= 是整体覆盖, 需要保留旧语义时调用.
    // - 目标是 ARRAY: 把 v 的内容追加进来 (v 为 ARRAY 则展开每个元素, 否则作为一项追加)
    // - 目标是 OBJECT 且 v 为 OBJECT: 逐键合并 (同名键用 v 的值覆盖)
    // - 类型不匹配: 退回整体覆盖
    obj_val &append(const obj_val &v);

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

    // 定义放在文件末尾(obj_t/obj_array 完整之后); 类型不是数组/对象时抛 std::runtime_error
    auto begin();
    auto end();
    auto cbegin() const;
    auto cend() const;

    auto obj_begin();
    auto obj_end();
    auto obj_cbegin() const;
    auto obj_cend() const;

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
    std::string to_json() const;
    std::string to_json_uncode() const;

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
    // 插入/删除可能搬动元素, operator[]/as_object() 返回的引用与迭代器随之失效
    std::vector<std::pair<std::string, obj_val>> _data;
};
struct obj_array
{
  public:
    // 插入/删除可能搬动元素, operator[]/as_array() 返回的引用与迭代器随之失效
    std::vector<obj_val> _data;
};

inline auto obj_val::begin()
{
    if (_val_type == obj_type::ARRAY)
        return array_val->_data.begin();
    throw std::runtime_error("Not an array");
}
inline auto obj_val::end()
{
    if (_val_type == obj_type::ARRAY)
        return array_val->_data.end();
    throw std::runtime_error("Not an array");
}
inline auto obj_val::cbegin() const
{
    if (_val_type == obj_type::ARRAY)
        return array_val->_data.cbegin();
    throw std::runtime_error("Not an array");
}
inline auto obj_val::cend() const
{
    if (_val_type == obj_type::ARRAY)
        return array_val->_data.cend();
    throw std::runtime_error("Not an array");
}
inline auto obj_val::obj_begin()
{
    if (_val_type == obj_type::OBJECT)
        return obj->_data.begin();
    throw std::runtime_error("Not an object");
}
inline auto obj_val::obj_end()
{
    if (_val_type == obj_type::OBJECT)
        return obj->_data.end();
    throw std::runtime_error("Not an object");
}
inline auto obj_val::obj_cbegin() const
{
    if (_val_type == obj_type::OBJECT)
        return obj->_data.cbegin();
    throw std::runtime_error("Not an object");
}
inline auto obj_val::obj_cend() const
{
    if (_val_type == obj_type::OBJECT)
        return obj->_data.cend();
    throw std::runtime_error("Not an object");
}
}// namespace http
std::ostream &operator<<(std::ostream &, const http::obj_val &);
#endif
