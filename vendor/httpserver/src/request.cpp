/*
 * paozhu micro obj rebuild
 * author Huang ziquan (黄自权)
 * date 2026-07-24
 */
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <list>
#include <cmath>
#include <climits>
#include <algorithm>
#include <functional>
#include <charconv>
#include "request.h"

namespace http
{
std::string obj_val::filter(std::function<char(unsigned char)> func)
{
    std::string temp;
    if (_val_type != obj_type::STRING)
        return temp;
    if (func == nullptr)
        return temp;
    for (char c : str_)
    {
        // 回调收无符号值, toupper/tolower 之类不再拿到负 char(未定义行为)
        char a = func(static_cast<unsigned char>(c));
        if (a != '\0')
            temp.push_back(a);
    }
    return temp;
}

unsigned int obj_val::mb_strlen()
{
    if (_val_type != obj_type::STRING)
        return 0;
    unsigned int temp_length = 0;
    std::size_t pos = 0;
    while (pos < str_.size())
    {
        unsigned char c = (unsigned char)str_[pos];
        if (c < 0x80)
            temp_length++;
        else if (c < 0xC0)
            temp_length++;
        else if (c >= 0xC0 && c < 0xE0)
        {
            pos += 1;
            temp_length++;
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            pos += 2;
            temp_length++;
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            pos += 3;
            temp_length++;
        }
        else
            temp_length++;
        pos++;
    }
    return temp_length;
}

std::string obj_val::mb_substr(int begin_pos, int cut_size)
{
    std::string temp;
    if (_val_type != obj_type::STRING)
        return temp;
    
    int str_length = mb_strlen();
    int n, j;
    
    if (begin_pos < 0)
    {
        n = str_length + begin_pos;
        if (n < 0) n = 0;
    }
    else
    {
        n = begin_pos;
        if (n > str_length) n = str_length;
    }
    
    if (cut_size < 0)
    {
        j = str_length + cut_size;
        if (j < 0) j = 0;
        if (j < n) return temp;
    }
    else if (cut_size == 0)
    {
        j = str_length;
    }
    else
    {
        j = n + cut_size;
        if (j > str_length) j = str_length;
        if (j < n) return temp;
    }
    
    int offsetnum = 0;
    unsigned char c;
    for (std::size_t pos = 0; pos < str_.size() && offsetnum < j; pos++)
    {
        c = (unsigned char)str_[pos];
        if (c < 0x80)
        {
            if (offsetnum >= n)
                temp.push_back(str_[pos]);
            offsetnum++;
        }
        else if (c < 0xC0)
        {
            if (offsetnum >= n)
                temp.push_back(str_[pos]);
            offsetnum++;
        }
        else if (c >= 0xC0 && c < 0xE0)
        {
            if (pos + 1 >= str_.size()) break;
            if (offsetnum >= n)
            {
                temp.push_back(str_[pos]);
                temp.push_back(str_[pos + 1]);
            }
            pos += 1;
            offsetnum++;
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            if (pos + 2 >= str_.size()) break;
            if (offsetnum >= n)
            {
                temp.push_back(str_[pos]);
                temp.push_back(str_[pos + 1]);
                temp.push_back(str_[pos + 2]);
            }
            pos += 2;
            offsetnum++;
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            if (pos + 3 >= str_.size()) break;
            if (offsetnum >= n)
            {
                temp.push_back(str_[pos]);
                temp.push_back(str_[pos + 1]);
                temp.push_back(str_[pos + 2]);
                temp.push_back(str_[pos + 3]);
            }
            pos += 3;
            offsetnum++;
        }
        else
        {
            if (offsetnum >= n)
                temp.push_back(str_[pos]);
            offsetnum++;
        }
    }
    return temp;
}

// get_type() / set_type() 已 inline 定义在 include/request.h
const char *obj_val::c_str() const
{
    if (_val_type == obj_type::STRING)
        return str_.c_str();
    return nullptr;
}

char *obj_val::str_data()
{
    if (_val_type == obj_type::STRING)
        return const_cast<char*>(str_.data());
    return nullptr;
}

std::string_view obj_val::str_view()
{
    if (_val_type == obj_type::STRING)
        return str_;
    return {};
}

std::string_view obj_val::str_view(std::string_view default_val)
{
    if (_val_type == obj_type::STRING)
        return str_;
    return default_val;
}

std::string_view obj_val::str_view(int a, int b)
{
    if (_val_type != obj_type::STRING)
        return {};
    
    int aa = (int)str_.size();
    if (a < 0)
    {
        aa = aa + a;
        if (aa < 0) aa = 0;
    }
    else if (a < (int)str_.size())
    {
        aa = a;
    }
    
    int bb = (int)str_.size();
    if (b < 0)
    {
        bb = bb + b;
        if (bb < 0) bb = 0;
    }
    else
    {
        bb = aa + b;
        if (bb > (int)str_.size()) bb = (int)str_.size();
    }
    
    bb = bb - aa;
    if (bb < 0) bb = 0;
    if (aa >= (int)str_.size()) return {};
    
    return std::string_view(str_.data() + aa, bb);
}

bool obj_val::reserve(unsigned int resize)
{
    if (_val_type == obj_type::STRING)
    {
        str_.reserve(resize);
        return true;
    }
    else if (_val_type == obj_type::OBJECT && obj != nullptr)
    {
        if (resize > obj->_data.size())
        {
            obj->_data.reserve(resize);
            return true;
        }
    }
    else if (_val_type == obj_type::ARRAY && array_val != nullptr)
    {
        if (resize > array_val->_data.size())
        {
            array_val->_data.reserve(resize);
            return true;
        }
    }
    return false;
}

std::string obj_val::substr(int a, int b)
{
    if (_val_type != obj_type::STRING)
        return {};
    
    if (a < 0)
        a = (int)str_.size() + a;
    if (a < 0) a = 0;
    if (b < 0)
        b = (int)str_.size() + b;
    if (b < 0) b = 0;
    if (b > (int)str_.size()) b = (int)str_.size();
    if (a >= b) return {};
    
    return str_.substr(a, b - a);
}

std::string obj_val::substr(int a)
{
    if (_val_type != obj_type::STRING)
        return {};
    
    if (a < 0)
        a = (int)str_.size() + a;
    if (a < 0) a = 0;
    if (a >= (int)str_.size()) return {};
    
    return str_.substr(a);
}

void obj_val::append(const std::string &v)
{
    if (_val_type == obj_type::NIL)
        _val_type = obj_type::STRING;
    if (_val_type != obj_type::STRING)
        return;
    str_.append(v);
}

void obj_val::append(const char *_str, unsigned int str_length)
{
    if (_str == nullptr)
        return;
    if (_val_type == obj_type::NIL)
        _val_type = obj_type::STRING;
    if (_val_type != obj_type::STRING)
        return;
    str_.append(_str, str_length);
}

std::size_t obj_val::size() const
{
    if (_val_type == obj_type::ARRAY)
        return array_val->_data.size();
    else if (_val_type == obj_type::OBJECT)
        return obj->_data.size();
    else if (_val_type == obj_type::STRING)
        return str_.size();
    return 0;
}

obj_val &obj_val::operator[](unsigned int index)
{
    // 数字下标只做两件事: 数组取位置, 对象取十进制文本 key; 不再有「对象按位置取」的第三语义
    if (_val_type == obj_type::ARRAY)
    {
        if (index < array_val->_data.size())
            return array_val->_data[index];
        throw std::out_of_range("obj_val::operator[]: array index out of range");
    }
    // 其余类型(含 NIL)与 ["<n>"] 走同一路径: NIL 建 OBJECT, 标量先清空再建 OBJECT
    return (*this)[std::to_string(index)];
}

obj_val &obj_val::operator[](std::string_view key)
{
    if (_val_type == obj_type::ARRAY)
    {
        unsigned int i_pos = 0;
        auto [ptr, ec] = std::from_chars(key.data(), key.data() + key.size(), i_pos);
        if (ec == std::errc{} && ptr == key.data() + key.size() && i_pos < array_val->_data.size())
            return array_val->_data[i_pos];
        throw std::out_of_range("obj_val::operator[]: array index out of range");
    }
    
    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj = new obj_t;
        _val_type = obj_type::OBJECT;
    }
    
    for (auto &pair : obj->_data)
    {
        if (pair.first == key)
            return pair.second;
    }
    obj->_data.emplace_back(key, nullptr);
    obj->_data.back().second.set_type(obj_type::NIL);
    return obj->_data.back().second;
}

const obj_val &obj_val::operator[](unsigned int index) const
{
    // 数组走位置、对象走十进制文本 key、NIL 返回 nil, 与 ["<n>"] 的 const 版一致
    return (*this)[std::to_string(index)];
}

const obj_val &obj_val::operator[](std::string_view key) const
{
    static const obj_val nil;
    if (_val_type == obj_type::ARRAY)
    {
        unsigned int i_pos = 0;
        auto [ptr, ec] = std::from_chars(key.data(), key.data() + key.size(), i_pos);
        if (ec == std::errc{} && ptr == key.data() + key.size() && i_pos < array_val->_data.size())
            return array_val->_data[i_pos];
        return nil;
    }
    if (_val_type == obj_type::OBJECT)
    {
        for (const auto &pair : obj->_data)
        {
            if (pair.first == key)
                return pair.second;
        }
    }
    return nil;
}

const obj_val &obj_val::at(std::string_view key) const
{
    if (_val_type == obj_type::OBJECT)
    {
        for (const auto &pair : obj->_data)
        {
            if (pair.first == key)
                return pair.second;
        }
    }
    throw std::out_of_range("obj_val::at: key not found");
}

obj_val &obj_val::at(std::string_view key)
{
    if (_val_type == obj_type::OBJECT)
    {
        for (auto &pair : obj->_data)
        {
            if (pair.first == key)
                return pair.second;
        }
    }
    throw std::out_of_range("obj_val::at: key not found");
}

const obj_val &obj_val::at(unsigned int index) const
{
    if (_val_type != obj_type::ARRAY)
        throw std::runtime_error("obj_val::at: not an array");
    if (index >= array_val->_data.size())
        throw std::out_of_range("obj_val::at: array index out of range");
    return array_val->_data[index];
}

obj_val &obj_val::at(unsigned int index)
{
    if (_val_type != obj_type::ARRAY)
        throw std::runtime_error("obj_val::at: not an array");
    if (index >= array_val->_data.size())
        throw std::out_of_range("obj_val::at: array index out of range");
    return array_val->_data[index];
}

void obj_val::clear()
{
    if (_val_type == obj_type::OBJECT || _val_type == obj_type::ARRAY)
        release_deep();
    str_.clear();
    _val_type = obj_type::NIL;
}

// 迭代式释放: 逐层把子容器摘下来再统一 delete, 避免析构按嵌套层数递归;
// 通过 push/operator[] 搭出来的深树没有深度上限, 递归会撑爆栈.
void obj_val::release_deep() noexcept
{
    std::vector<obj_val *> pending;
    std::vector<obj_t *> owned_objs;
    std::vector<obj_array *> owned_arrays;

    auto detach = [&pending, &owned_objs, &owned_arrays](obj_val *p) {
        if (p->_val_type == obj_type::OBJECT && p->obj != nullptr)
        {
            obj_t *o = p->obj;
            // 先登记再摘: push_back 抛异常时 o 仍归 p 所有, catch 分支只删登记的容器
            owned_objs.push_back(o);
            p->obj = nullptr;
            p->_val_type = obj_type::NIL;
            for (auto &kv : o->_data)
                pending.push_back(&kv.second);
        }
        else if (p->_val_type == obj_type::ARRAY && p->array_val != nullptr)
        {
            obj_array *a = p->array_val;
            owned_arrays.push_back(a);
            p->array_val = nullptr;
            p->_val_type = obj_type::NIL;
            for (auto &e : a->_data)
                pending.push_back(&e);
        }
    };

    try
    {
        detach(this);
        while (!pending.empty())
        {
            obj_val *p = pending.back();
            pending.pop_back();
            detach(p);
        }
    }
    catch (...)
    {
        // 内存不足时退回递归释放(极端场景, 宁可慢也不能漏)
        for (obj_t *o : owned_objs)
            delete o;
        for (obj_array *a : owned_arrays)
            delete a;
        if (_val_type == obj_type::OBJECT && obj != nullptr)
        {
            delete obj;
            obj = nullptr;
        }
        else if (_val_type == obj_type::ARRAY && array_val != nullptr)
        {
            delete array_val;
            array_val = nullptr;
        }
        _val_type = obj_type::NIL;
        return;
    }

    // 此时树上的每个容器节点都已是 NIL, 删除容器不会再次递归
    for (obj_t *o : owned_objs)
        delete o;
    for (obj_array *a : owned_arrays)
        delete a;
}

obj_val &obj_val::operator=(long long i)
{
    clear();
    lval = i;
    _val_type = obj_type::LONG;
    return *this;
}

obj_val &obj_val::operator=(unsigned long long i)
{
    clear();
    uval = i;
    _val_type = obj_type::ULONG;
    return *this;
}

obj_val &obj_val::operator=(int i)
{
    clear();
    lval = i;
    _val_type = obj_type::INT;
    return *this;
}

obj_val &obj_val::operator=(short i)
{
    clear();
    lval = i;
    _val_type = obj_type::INT;
    return *this;
}

obj_val &obj_val::operator=(unsigned short i)
{
    clear();
    uval = i;
    _val_type = obj_type::UINT;
    return *this;
}

obj_val &obj_val::operator=(unsigned int i)
{
    clear();
    uval = i;
    _val_type = obj_type::UINT;
    return *this;
}

obj_val &obj_val::operator=(long i)
{
    clear();
    lval = i;
    _val_type = obj_type::LONG;
    return *this;
}

obj_val &obj_val::operator=(unsigned long i)
{
    clear();
    uval = i;
    _val_type = obj_type::ULONG;
    return *this;
}

obj_val &obj_val::operator=(float i)
{
    clear();
    dval = i;
    _val_type = obj_type::DOUBLE;
    return *this;
}

obj_val &obj_val::operator=(double i)
{
    clear();
    dval = i;
    _val_type = obj_type::DOUBLE;
    return *this;
}

obj_val &obj_val::operator=(long double i)
{
    clear();
    // long double 只保留到 double: obj_val 的数值存储是 union 里的 double
    dval = static_cast<double>(i);
    _val_type = obj_type::DOUBLE;
    return *this;
}

obj_val &obj_val::operator=(bool i)
{
    clear();
    lval = i;
    _val_type = obj_type::BOOL;
    return *this;
}

obj_val &obj_val::operator=(std::string_view v)
{
    clear();
    str_ = v;
    _val_type = obj_type::STRING;
    return *this;
}

obj_val &obj_val::operator=(const char *v)
{
    clear();
    if (v == nullptr)
        return *this;
    str_ = v;
    _val_type = obj_type::STRING;
    return *this;
}

obj_val &obj_val::operator=(std::nullptr_t)
{
    clear();
    return *this;
}

obj_val &obj_val::operator=(std::string &&v)
{
    clear();
    str_ = std::move(v);
    _val_type = obj_type::STRING;
    return *this;
}

obj_val &obj_val::operator=(const std::string &v)
{
    clear();
    str_ = v;
    _val_type = obj_type::STRING;
    return *this;
}

obj_val &obj_val::operator=(const obj_val &v)
{
    if (this == &v)
        return *this;

    // 整体覆盖, 与拷贝构造语义一致; 需要按键合并/数组追加请显式调用 append().
    // v 可能就在 *this 的容器里(如 o = o["f"]), 必须先拍快照: clear() 会释放 v 所在的容器.
    obj_val tmp(v);
    clear();
    switch (tmp.get_type())
    {
    case obj_type::INT:
        lval = tmp.lval;
        _val_type = obj_type::INT;
        break;
    case obj_type::UINT:
        uval = tmp.uval;
        _val_type = obj_type::UINT;
        break;
    case obj_type::ULONG:
        uval = tmp.uval;
        _val_type = obj_type::ULONG;
        break;
    case obj_type::LONG:
        lval = tmp.lval;
        _val_type = obj_type::LONG;
        break;
    case obj_type::BOOL:
        lval = tmp.lval;
        _val_type = obj_type::BOOL;
        break;
    case obj_type::DOUBLE:
        dval = tmp.dval;
        _val_type = obj_type::DOUBLE;
        break;
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::STRING:
        str_ = std::move(tmp.str_);
        _val_type = obj_type::STRING;
        break;
    case obj_type::OBJECT:
        obj = tmp.obj;
        tmp.obj = nullptr;
        tmp.set_type(obj_type::NIL);
        _val_type = obj_type::OBJECT;
        break;
    case obj_type::ARRAY:
        array_val = tmp.array_val;
        tmp.array_val = nullptr;
        tmp.set_type(obj_type::NIL);
        _val_type = obj_type::ARRAY;
        break;
    default:
        _val_type = obj_type::NIL;
    }
    return *this;
}

obj_val &obj_val::append(const obj_val &v)
{
    if (this == &v)
        return *this;

    // 先拍快照: v 可能就是 *this 容器里的元素(如 o.append(o["f"])),
    // 直接引用会在扩容/clear 时悬垂; 这里统一走独立副本.
    obj_val tmp(v);

    if (tmp.get_type() == obj_type::ARRAY)
    {
        // 目标确保是 ARRAY: 没有就先建空的, 再展开追加 v 的每一个元素
        if (_val_type != obj_type::ARRAY)
        {
            clear();
            array_val = new obj_array;
            _val_type = obj_type::ARRAY;
        }
        for (auto &e : tmp.array_val->_data)
            array_val->_data.push_back(std::move(e));
        return *this;
    }

    if (tmp.get_type() == obj_type::OBJECT)
    {
        // 目标确保是 OBJECT: 没有就先建空的, 再逐键合并 (同名键用 v 的值覆盖)
        if (_val_type != obj_type::OBJECT)
        {
            clear();
            obj = new obj_t;
            _val_type = obj_type::OBJECT;
        }
        for (auto &iter : tmp.obj->_data)
        {
            auto iter2 = obj->_data.begin();
            for (; iter2 != obj->_data.end(); iter2++)
            {
                if (iter.first == iter2->first)
                {
                    iter2->second = std::move(iter.second);
                    break;
                }
            }
            if (iter2 == obj->_data.end())
                obj->_data.push_back(std::move(iter));
        }
        return *this;
    }

    // v 是标量/STRING/NIL: 目标已经是 ARRAY 就作为一项追加, 否则退回整体覆盖
    if (_val_type == obj_type::ARRAY)
    {
        array_val->_data.push_back(std::move(tmp));
        return *this;
    }
    return operator=(std::move(tmp));
}

obj_val &obj_val::operator=(obj_val &&v)
{
    if (this == &v)
        return *this;

    // 同拷贝赋值: v 可能就在 *this 的容器里(如 o = std::move(o["f"])),
    // 先把载荷偷进局部快照, 之后的 clear() 不会碰到它.
    obj_val tmp(std::move(v));
    clear();
    switch (tmp.get_type())
    {
    case obj_type::INT:
        lval = tmp.lval;
        _val_type = obj_type::INT;
        break;
    case obj_type::UINT:
        uval = tmp.uval;
        _val_type = obj_type::UINT;
        break;
    case obj_type::ULONG:
        uval = tmp.uval;
        _val_type = obj_type::ULONG;
        break;
    case obj_type::LONG:
        lval = tmp.lval;
        _val_type = obj_type::LONG;
        break;
    case obj_type::BOOL:
        lval = tmp.lval;
        _val_type = obj_type::BOOL;
        break;
    case obj_type::DOUBLE:
        dval = tmp.dval;
        _val_type = obj_type::DOUBLE;
        break;
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::STRING:
        str_ = std::move(tmp.str_);
        _val_type = obj_type::STRING;
        break;
    case obj_type::OBJECT:
        obj = tmp.obj;
        tmp.obj = nullptr;
        tmp.set_type(obj_type::NIL);
        _val_type = obj_type::OBJECT;
        break;
    case obj_type::ARRAY:
        array_val = tmp.array_val;
        tmp.array_val = nullptr;
        tmp.set_type(obj_type::NIL);
        _val_type = obj_type::ARRAY;
        break;
    default:
        _val_type = obj_type::NIL;
    }
    return *this;
}

namespace
{
// p 是否落在这段连续存储里; 用来识别 push/zip 的参数就是自己内部的元素.
// vector 扩容会把源元素搬走, 这种自引用必须先做成值再插入.
bool points_into_storage(const void *p, const void *begin, std::size_t bytes)
{
    if (bytes == 0)
        return false;
    const std::uintptr_t q = reinterpret_cast<std::uintptr_t>(p);
    const std::uintptr_t b = reinterpret_cast<std::uintptr_t>(begin);
    return q >= b && q < b + bytes;
}
}

obj_val &obj_val::push(const obj_val &v)
{
    if (this == &v)
        return *this;
    if (_val_type == obj_type::ARRAY)
    {
        if (points_into_storage(&v, array_val->_data.data(), array_val->_data.size() * sizeof(obj_val)))
        {
            obj_val tmp(v);
            array_val->_data.push_back(std::move(tmp));
        }
        else
            array_val->_data.emplace_back(v);
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        array_val->_data.emplace_back(v);
    }
    return *this;
}

obj_val &obj_val::push(obj_val &&v)
{
    if (this == &v)
        return *this;
    if (_val_type == obj_type::ARRAY)
    {
        if (points_into_storage(&v, array_val->_data.data(), array_val->_data.size() * sizeof(obj_val)))
        {
            obj_val tmp(v);
            array_val->_data.push_back(std::move(tmp));
        }
        else
            array_val->_data.emplace_back(std::move(v));
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        array_val->_data.emplace_back(std::move(v));
    }
    return *this;
}

obj_val &obj_val::push(const std::string &key, const obj_val &v)
{
    if (this == &v)
        return *this;
    if (_val_type == obj_type::OBJECT)
    {
        const std::size_t bytes = obj->_data.size() * sizeof(std::pair<std::string, obj_val>);
        if (points_into_storage(&v, obj->_data.data(), bytes) || points_into_storage(&key, obj->_data.data(), bytes))
        {
            std::string tmp_key(key);
            obj_val tmp_val(v);
            obj->_data.emplace_back(std::move(tmp_key), std::move(tmp_val));
        }
        else
            obj->_data.emplace_back(key, v);
    }
    else if (_val_type == obj_type::NIL)
    {
        obj = new obj_t;
        _val_type = obj_type::OBJECT;
        obj->_data.emplace_back(key, v);
    }
    return *this;
}

obj_val &obj_val::push(const std::string &key, obj_val &&v)
{
    if (this == &v)
        return *this;
    if (_val_type == obj_type::OBJECT)
    {
        const std::size_t bytes = obj->_data.size() * sizeof(std::pair<std::string, obj_val>);
        if (points_into_storage(&v, obj->_data.data(), bytes) || points_into_storage(&key, obj->_data.data(), bytes))
        {
            std::string tmp_key(key);
            obj_val tmp_val(v);
            obj->_data.emplace_back(std::move(tmp_key), std::move(tmp_val));
        }
        else
            obj->_data.emplace_back(key, std::move(v));
    }
    else if (_val_type == obj_type::NIL)
    {
        obj = new obj_t;
        _val_type = obj_type::OBJECT;
        obj->_data.emplace_back(key, std::move(v));
    }
    return *this;
}

obj_val::obj_val(const obj_val &v)
{
    switch (v.get_type())
    {
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::BOOL:
        _val_type = obj_type::BOOL;
        lval = v.lval;
        break;
    case obj_type::STRING:
        _val_type = obj_type::STRING;
        str_ = v.str_;
        break;
    case obj_type::INT:
        _val_type = obj_type::INT;
        lval = v.lval;
        break;
    case obj_type::UINT:
        _val_type = obj_type::UINT;
        uval = v.uval;
        break;
    case obj_type::LONG:
        _val_type = obj_type::LONG;
        lval = v.lval;
        break;
    case obj_type::ULONG:
        _val_type = obj_type::ULONG;
        uval = v.uval;
        break;
    case obj_type::DOUBLE:
        _val_type = obj_type::DOUBLE;
        dval = v.dval;
        break;
    case obj_type::OBJECT:
        _val_type = obj_type::OBJECT;
        obj = new obj_t;
        obj->_data = v.obj->_data;
        break;
    case obj_type::ARRAY:
        _val_type = obj_type::ARRAY;
        array_val = new obj_array;
        array_val->_data = v.array_val->_data;
        break;
    default:
        _val_type = obj_type::NIL;
    }
}

obj_val::obj_val(obj_val &&v) noexcept
{
    switch (v.get_type())
    {
    case obj_type::NIL:
        _val_type = obj_type::NIL;
        break;
    case obj_type::BOOL:
        _val_type = obj_type::BOOL;
        lval = v.lval;
        break;
    case obj_type::STRING:
        _val_type = obj_type::STRING;
        str_ = std::move(v.str_);
        break;
    case obj_type::INT:
        _val_type = obj_type::INT;
        lval = v.lval;
        break;
    case obj_type::UINT:
        _val_type = obj_type::UINT;
        uval = v.uval;
        break;
    case obj_type::LONG:
        _val_type = obj_type::LONG;
        lval = v.lval;
        break;
    case obj_type::ULONG:
        _val_type = obj_type::ULONG;
        uval = v.uval;
        break;
    case obj_type::DOUBLE:
        _val_type = obj_type::DOUBLE;
        dval = v.dval;
        break;
    case obj_type::OBJECT:
        _val_type = obj_type::OBJECT;
        obj = v.obj;
        v.obj = nullptr;
        v.set_type(obj_type::NIL);
        break;
    case obj_type::ARRAY:
        _val_type = obj_type::ARRAY;
        array_val = v.array_val;
        v.array_val = nullptr;
        v.set_type(obj_type::NIL);
        break;
    default:
        _val_type = obj_type::NIL;
    }
}

std::string_view obj_val::to_string_view() const
{
    if (_val_type == obj_type::STRING)
        return str_;
    return {};
}

std::string obj_val::to_string() const
{
    char buf[64];
    switch (_val_type)
    {
    case obj_type::STRING:
        return str_;
    case obj_type::OBJECT:
        return "{}";
    case obj_type::ARRAY:
        return "[]";
    case obj_type::BOOL:
        return lval ? "true" : "false";
    case obj_type::INT:
    case obj_type::LONG:
    {
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), lval);
        if (ec == std::errc{})
            return std::string(buf, ptr - buf);
        return std::to_string(lval);
    }
    case obj_type::UINT:
    case obj_type::ULONG:
    {
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), uval);
        if (ec == std::errc{})
            return std::string(buf, ptr - buf);
        return std::to_string(uval);
    }
    case obj_type::DOUBLE:
    {
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), dval);
        if (ec == std::errc{})
        {
            std::string result(buf, ptr - buf);
            // float/double must keep a decimal form, never an integer literal
            if (result.find('.') == std::string::npos && result.find('e') == std::string::npos &&
                result.find('E') == std::string::npos && result.find('n') == std::string::npos &&
                result.find('i') == std::string::npos)
                result += ".0";
            return result;
        }
        return std::to_string(dval);
    }
    case obj_type::NIL:
    default:
        return "";
    }
}

std::string obj_val::to_sql_mysql_escape()
{
    if (_val_type == obj_type::STRING)
    {
        std::string a_temp;
        auto first = str_.begin();
        auto last = str_.end();
        
        while (first != last && (*first == ' ' || *first == '\t' || *first == '\r' || *first == '\n'))
            ++first;
        
        if (first != last)
        {
            auto temp_last = last;
            --temp_last;
            while (temp_last != first && (*temp_last == ' ' || *temp_last == '\t' || *temp_last == '\r' || *temp_last == '\n'))
                --temp_last;
            ++temp_last;
            
            for (auto it = first; it != temp_last; ++it)
            {
                switch (*it)
                {
                case '\'': a_temp += "\\'"; break;
                case '"': a_temp += "\\\""; break;
                case '\\': a_temp += "\\\\"; break;
                case '\0': a_temp += "\\0"; break;
                case '\n': a_temp += "\\n"; break;
                case '\r': a_temp += "\\r"; break;
                case '\x1a': a_temp += "\\Z"; break;
                default: a_temp.push_back(*it); break;
                }
            }
        }
        return a_temp;
    }
    return static_cast<std::string>(*this);
}

std::string obj_val::to_trim()
{
    if (_val_type == obj_type::STRING)
    {
        auto first = str_.begin();
        auto last = str_.end();
        
        while (first != last && (*first == ' ' || *first == '\t' || *first == '\r' || *first == '\n'))
            ++first;
        
        if (first != last)
        {
            auto temp_last = last;
            --temp_last;
            while (temp_last != first && (*temp_last == ' ' || *temp_last == '\t' || *temp_last == '\r' || *temp_last == '\n'))
                --temp_last;
            ++temp_last;
            return std::string(first, temp_last);
        }
        return "";
    }
    return static_cast<std::string>(*this);
}

void obj_val::trim()
{
    if (_val_type == obj_type::STRING)
    {
        auto first = str_.begin();
        auto last = str_.end();
        
        while (first != last && (*first == ' ' || *first == '\t' || *first == '\r' || *first == '\n'))
            ++first;
        
        if (first != last)
        {
            auto temp_last = last;
            --temp_last;
            while (temp_last != first && (*temp_last == ' ' || *temp_last == '\t' || *temp_last == '\r' || *temp_last == '\n'))
                --temp_last;
            ++temp_last;
            str_ = std::string(first, temp_last);
        }
        else
        {
            str_.clear();
        }
    }
}

obj_val::~obj_val()
{
    if (_val_type == obj_type::OBJECT || _val_type == obj_type::ARRAY)
        release_deep();
}

obj_val::obj_val() : _val_type(obj_type::NIL) {}
obj_val::obj_val(std::nullptr_t) : _val_type(obj_type::NIL) {}
obj_val::obj_val(bool i) : lval(i), _val_type(obj_type::BOOL) {}
obj_val::obj_val(long long i) : lval(i), _val_type(obj_type::LONG) {}
obj_val::obj_val(unsigned long long i) : uval(i), _val_type(obj_type::ULONG) {}
obj_val::obj_val(int i) : lval(i), _val_type(obj_type::INT) {}
obj_val::obj_val(long i) : lval(i), _val_type(obj_type::LONG) {}
obj_val::obj_val(unsigned int i) : uval(i), _val_type(obj_type::UINT) {}
obj_val::obj_val(unsigned long i) : uval(i), _val_type(obj_type::ULONG) {}
obj_val::obj_val(unsigned short i) : uval(i), _val_type(obj_type::UINT) {}
obj_val::obj_val(short i) : lval(i), _val_type(obj_type::INT) {}
obj_val::obj_val(float i) : dval(i), _val_type(obj_type::DOUBLE) {}
obj_val::obj_val(double i) : dval(i), _val_type(obj_type::DOUBLE) {}
obj_val::obj_val(long double i) : dval(i), _val_type(obj_type::DOUBLE) {}
obj_val::obj_val(const char *_str) : str_(_str ? _str : ""), _val_type(_str ? obj_type::STRING : obj_type::NIL) {}
obj_val::obj_val(const char *_str, unsigned int str_length)
    : str_(_str ? std::string(_str, str_length) : std::string()), _val_type(_str ? obj_type::STRING : obj_type::NIL) {}
obj_val::obj_val(std::string_view _str) : str_(_str), _val_type(obj_type::STRING) {}
obj_val::obj_val(const std::string &_str) : str_(_str), _val_type(obj_type::STRING) {}
obj_val::obj_val(std::string &&_str) : str_(std::move(_str)), _val_type(obj_type::STRING) {}

obj_val::obj_val(const obj_t &v) : _val_type(obj_type::OBJECT)
{
    obj = new obj_t;
    obj->_data = v._data;
}

obj_val::obj_val(obj_t &&v) : _val_type(obj_type::OBJECT)
{
    obj = new obj_t;
    obj->_data = std::move(v._data);
}

obj_val::obj_val(const obj_array &v) : _val_type(obj_type::ARRAY)
{
    array_val = new obj_array;
    array_val->_data = v._data;
}

obj_val::obj_val(obj_array &&v) : _val_type(obj_type::ARRAY)
{
    array_val = new obj_array;
    array_val->_data = std::move(v._data);
}

float obj_val::str_to_float()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stof(str_); }
    catch (const std::exception &) { return 0; }
}

double obj_val::str_to_double()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stod(str_); }
    catch (const std::exception &) { return 0; }
}

int obj_val::str_to_int()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stoi(str_); }
    catch (const std::exception &) { return 0; }
}

long long obj_val::str_to_long()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stoll(str_); }
    catch (const std::exception &) { return 0; }
}

unsigned int obj_val::str_to_uint()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stoul(str_); }
    catch (const std::exception &) { return 0; }
}

unsigned long long obj_val::str_to_ulong()
{
    if (_val_type != obj_type::STRING)
        return 0;
    try { return std::stoull(str_); }
    catch (const std::exception &) { return 0; }
}

// 字符串真值: "false"/"False"/"FALSE"/"0"/空串为假; to_bool() 与 operator bool 共用同一口径
static bool str_truthy(const std::string &s)
{
    return !s.empty() && s != "false" && s != "False" && s != "FALSE" && s != "0";
}

bool obj_val::to_bool() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return false;
    case obj_type::BOOL: return lval != 0;
    case obj_type::STRING:
        return str_truthy(str_);
    case obj_type::INT:
    case obj_type::LONG: return lval != 0;
    case obj_type::UINT:
    case obj_type::ULONG: return uval != 0;
    case obj_type::DOUBLE: return dval != 0.0;
    case obj_type::OBJECT: return obj && !obj->_data.empty();
    case obj_type::ARRAY: return array_val && !array_val->_data.empty();
    default: return false;
    }
}

long long obj_val::to_int() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0;
    case obj_type::BOOL: return lval ? 1 : 0;
    case obj_type::STRING:
        if (str_.empty()) return 0;
        try { return std::stoll(str_); }
        catch (const std::exception &) { return 0; }
    case obj_type::INT:
    case obj_type::LONG: return lval;
    case obj_type::UINT:
    case obj_type::ULONG:
        // 超出 long long 的无符号值饱和到 LLONG_MAX, 避免回绕成负数
        return uval > static_cast<unsigned long long>(LLONG_MAX) ? LLONG_MAX : static_cast<long long>(uval);
    case obj_type::DOUBLE: return static_cast<long long>(dval);
    default: return 0;
    }
}

double obj_val::to_float() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0.0;
    case obj_type::BOOL: return lval ? 1.0 : 0.0;
    case obj_type::STRING:
        if (str_.empty()) return 0.0;
        try { return std::stod(str_); }
        catch (const std::exception &) { return 0.0; }
    case obj_type::INT:
    case obj_type::LONG: return static_cast<double>(lval);
    case obj_type::UINT:
    case obj_type::ULONG: return static_cast<double>(uval);
    case obj_type::DOUBLE: return dval;
    default: return 0.0;
    }
}

double obj_val::to_double() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0.0;
    case obj_type::BOOL: return lval ? 1.0 : 0.0;
    case obj_type::STRING:
        if (str_.empty()) return 0.0;
        try { return std::stod(str_); }
        catch (const std::exception &) { return 0.0; }
    case obj_type::INT:
    case obj_type::LONG: return static_cast<double>(lval);
    case obj_type::UINT:
    case obj_type::ULONG: return static_cast<double>(uval);
    case obj_type::DOUBLE: return dval;
    default: return 0.0;
    }
}

char obj_val::to_char() const
{
    switch (_val_type)
    {
    case obj_type::INT:
    case obj_type::LONG:
        return static_cast<char>(lval);
    case obj_type::UINT:
    case obj_type::ULONG:
        return static_cast<char>(uval);
    case obj_type::STRING:
        return str_.empty() ? '\0' : str_[0];
    default:
        return '\0';
    }
}

obj_val &obj_val::from_char(char c)
{
    clear();
    if (c == '\0')
    {
        _val_type = obj_type::NIL;
    }
    else if (c >= 0)
    {
        uval = static_cast<unsigned char>(c);
        _val_type = obj_type::UINT;
    }
    else
    {
        lval = static_cast<signed char>(c);
        _val_type = obj_type::INT;
    }
    return *this;
}

obj_val &obj_val::set_array()
{
    if (_val_type != obj_type::ARRAY)
    {
        clear();
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
    }
    return *this;
}

obj_val &obj_val::set_obj()
{
    if (_val_type != obj_type::OBJECT)
    {
        clear();
        obj = new obj_t;
        _val_type = obj_type::OBJECT;
    }
    return *this;
}

obj_val &obj_val::set_object()
{
    return set_obj();
}

obj_val &obj_val::set_int()
{
    if (_val_type != obj_type::INT)
    {
        long long val = static_cast<long long>(*this);
        clear();
        lval = val;
        _val_type = obj_type::INT;
    }
    return *this;
}

obj_val &obj_val::set_long()
{
    if (_val_type != obj_type::LONG)
    {
        long long val = static_cast<long long>(*this);
        clear();
        lval = val;
        _val_type = obj_type::LONG;
    }
    return *this;
}

obj_val &obj_val::set_uint()
{
    if (_val_type != obj_type::UINT)
    {
        unsigned long long val = static_cast<unsigned long long>(*this);
        clear();
        uval = val;
        _val_type = obj_type::UINT;
    }
    return *this;
}

obj_val &obj_val::set_ulong()
{
    if (_val_type != obj_type::ULONG)
    {
        unsigned long long val = static_cast<unsigned long long>(*this);
        clear();
        uval = val;
        _val_type = obj_type::ULONG;
    }
    return *this;
}

obj_val &obj_val::set_double()
{
    if (_val_type != obj_type::DOUBLE)
    {
        double val = static_cast<double>(*this);
        clear();
        dval = val;
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}

obj_val &obj_val::set_string()
{
    if (_val_type != obj_type::STRING)
    {
        std::string val = static_cast<std::string>(*this);
        clear();
        str_ = val;
        _val_type = obj_type::STRING;
    }
    return *this;
}

obj_val &obj_val::set_bool()
{
    if (_val_type != obj_type::BOOL)
    {
        bool val = static_cast<bool>(*this);
        clear();
        lval = val;
        _val_type = obj_type::BOOL;
    }
    return *this;
}

obj_val &obj_val::set_null()
{
    clear();
    _val_type = obj_type::NIL;
    return *this;
}

obj_val &obj_val::operator+(const std::string &v)
{
    if (_val_type == obj_type::STRING)
        str_ += v;
    return *this;
}

obj_val &obj_val::operator+(long long i)
{
    if (_val_type == obj_type::LONG)
        lval += i;
    else if (_val_type == obj_type::DOUBLE)
        dval += i;
    else if (_val_type == obj_type::INT)
    {
        lval += i;
        _val_type = obj_type::LONG;
    }
    else if (_val_type == obj_type::UINT)
    {
        uval += i;
        _val_type = obj_type::ULONG;
    }
    else if (_val_type == obj_type::ULONG)
        uval += i;
    else if (_val_type == obj_type::NIL)
    {
        lval = i;
        _val_type = obj_type::LONG;
    }
    return *this;
}

obj_val &obj_val::operator+(unsigned long long i)
{
    if (_val_type == obj_type::ULONG)
        uval += i;
    else if (_val_type == obj_type::DOUBLE)
        dval += i;
    else if (_val_type == obj_type::UINT)
    {
        uval += i;
        _val_type = obj_type::ULONG;
    }
    else if (_val_type == obj_type::LONG)
        lval += i;
    else if (_val_type == obj_type::INT)
        lval += i;
    else if (_val_type == obj_type::NIL)
    {
        uval = i;
        _val_type = obj_type::ULONG;
    }
    return *this;
}

obj_val &obj_val::operator+(int i)
{
    return operator+(static_cast<long long>(i));
}

obj_val &obj_val::operator+(unsigned int i)
{
    return operator+(static_cast<unsigned long long>(i));
}

obj_val &obj_val::operator+(long i)
{
    return operator+(static_cast<long long>(i));
}

obj_val &obj_val::operator+(unsigned long i)
{
    return operator+(static_cast<unsigned long long>(i));
}

obj_val &obj_val::operator+(float i)
{
    if (_val_type == obj_type::DOUBLE)
        dval += i;
    else if (_val_type == obj_type::LONG)
        lval += static_cast<long long>(i);
    else if (_val_type == obj_type::ULONG)
        uval += static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::INT)
        lval += static_cast<long long>(i);
    else if (_val_type == obj_type::UINT)
        uval += static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::NIL)
    {
        dval = i;
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}

obj_val &obj_val::operator+(double i)
{
    if (_val_type == obj_type::DOUBLE)
        dval += i;
    else if (_val_type == obj_type::LONG)
        lval += static_cast<long long>(i);
    else if (_val_type == obj_type::ULONG)
        uval += static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::INT)
        lval += static_cast<long long>(i);
    else if (_val_type == obj_type::UINT)
        uval += static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::NIL)
    {
        dval = i;
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}

obj_val &obj_val::operator-(long long i)
{
    if (_val_type == obj_type::LONG)
        lval -= i;
    else if (_val_type == obj_type::DOUBLE)
        dval -= i;
    else if (_val_type == obj_type::INT)
    {
        lval -= i;
        _val_type = obj_type::LONG;
    }
    else if (_val_type == obj_type::UINT)
    {
        uval -= i;
        _val_type = obj_type::ULONG;
    }
    else if (_val_type == obj_type::ULONG)
        uval -= i;
    else if (_val_type == obj_type::NIL)
    {
        lval = -i;
        _val_type = obj_type::LONG;
    }
    return *this;
}

obj_val &obj_val::operator-(unsigned long long i)
{
    if (_val_type == obj_type::ULONG)
        uval -= i;
    else if (_val_type == obj_type::DOUBLE)
        dval -= i;
    else if (_val_type == obj_type::UINT)
        uval -= i;
    else if (_val_type == obj_type::LONG)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::INT)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::NIL)
    {
        uval = i;
        _val_type = obj_type::ULONG;
    }
    return *this;
}

obj_val &obj_val::operator-(int i)
{
    return operator-(static_cast<long long>(i));
}

obj_val &obj_val::operator-(unsigned int i)
{
    return operator-(static_cast<unsigned long long>(i));
}

obj_val &obj_val::operator-(long i)
{
    return operator-(static_cast<long long>(i));
}

obj_val &obj_val::operator-(unsigned long i)
{
    return operator-(static_cast<unsigned long long>(i));
}

obj_val &obj_val::operator-(float i)
{
    if (_val_type == obj_type::DOUBLE)
        dval -= i;
    else if (_val_type == obj_type::LONG)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::ULONG)
        uval -= static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::INT)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::UINT)
        uval -= static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::NIL)
    {
        dval = i;
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}

obj_val &obj_val::operator-(double i)
{
    if (_val_type == obj_type::DOUBLE)
        dval -= i;
    else if (_val_type == obj_type::LONG)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::ULONG)
        uval -= static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::INT)
        lval -= static_cast<long long>(i);
    else if (_val_type == obj_type::UINT)
        uval -= static_cast<unsigned long long>(i);
    else if (_val_type == obj_type::NIL)
    {
        dval = i;
        _val_type = obj_type::DOUBLE;
    }
    return *this;
}

bool obj_val::operator==(const obj_val &v)
{
    if (this == &v)
        return true;
    if (_val_type != v._val_type)
        return false;
    
    switch (_val_type)
    {
    case obj_type::NIL:
        return true;
    case obj_type::BOOL:
        return lval == v.lval;
    case obj_type::STRING:
        return str_ == v.str_;
    case obj_type::INT:
    case obj_type::LONG:
        return lval == v.lval;
    case obj_type::UINT:
    case obj_type::ULONG:
        return uval == v.uval;
    case obj_type::DOUBLE:
        // 精确比较: 带误差的比较不满足传递性, 会让去重/查找结果不确定
        return dval == v.dval;
    case obj_type::OBJECT:
        if (obj->_data.size() != v.obj->_data.size())
            return false;
        for (auto it1 = obj->_data.begin(), it2 = v.obj->_data.begin(); it1 != obj->_data.end(); ++it1, ++it2)
        {
            if (it1->first != it2->first || !(it1->second == it2->second))
                return false;
        }
        return true;
    case obj_type::ARRAY:
        if (array_val->_data.size() != v.array_val->_data.size())
            return false;
        for (auto it1 = array_val->_data.begin(), it2 = v.array_val->_data.begin(); it1 != array_val->_data.end(); ++it1, ++it2)
        {
            if (!(*it1 == *it2))
                return false;
        }
        return true;
    default:
        return false;
    }
}

bool obj_val::operator!=(const obj_val &v)
{
    return !operator==(v);
}

bool obj_val::operator>(const obj_val &v)
{
    if (this == &v)
        return false;
    
    if (_val_type == obj_type::STRING && v._val_type == obj_type::STRING)
        return str_ > v.str_;
    
    if (is_number() && v.is_number())
        return static_cast<double>(*this) > static_cast<double>(v);
    
    if (_val_type == obj_type::BOOL && v._val_type == obj_type::BOOL)
        return lval > v.lval;
    
    return false;
}

bool obj_val::operator>=(const obj_val &v)
{
    if (this == &v)
        return true;
    
    if (_val_type == obj_type::STRING && v._val_type == obj_type::STRING)
        return str_ >= v.str_;
    
    if (is_number() && v.is_number())
        return static_cast<double>(*this) >= static_cast<double>(v);
    
    if (_val_type == obj_type::BOOL && v._val_type == obj_type::BOOL)
        return lval >= v.lval;
    
    return false;
}

bool obj_val::unset(const std::string &key)
{
    if (_val_type != obj_type::OBJECT)
        return false;
    for (auto it = obj->_data.begin(); it != obj->_data.end(); ++it)
    {
        if (it->first == key)
        {
            obj->_data.erase(it);
            return true;
        }
    }
    return false;
}

bool obj_val::erase(const std::string &key)
{
    return unset(key);
}

bool obj_val::erase(const obj_val &key)
{
    if (_val_type != obj_type::OBJECT)
        return false;
    std::string k = static_cast<std::string>(key);
    return unset(k);
}

obj_val::obj_val(const std::initializer_list<std::string> nsl) : _val_type(obj_type::ARRAY)
{
    array_val = new obj_array;
    for (const auto &s : nsl)
        array_val->_data.emplace_back(s);
}

obj_val::obj_val(const std::initializer_list<double> nsl) : _val_type(obj_type::ARRAY)
{
    array_val = new obj_array;
    for (double v : nsl)
        array_val->_data.emplace_back(v);
}

obj_val::obj_val(const std::initializer_list<std::pair<std::string, double>> nsl) : _val_type(obj_type::OBJECT)
{
    obj = new obj_t;
    for (const auto &p : nsl)
        obj->_data.emplace_back(p.first, p.second);
}

bool obj_val::isset(const std::string &key)
{
    if (_val_type != obj_type::OBJECT)
        return false;
    for (auto &pair : obj->_data)
    {
        if (pair.first == key)
            return true;
    }
    return false;
}

// 返回 -1 表示不是十六进制字符; 上面两处 \u 转义必须靠它区分「数字 0」和「非法字符」
static int chartoint(char a)
{
    if (a >= '0' && a <= '9')
        return a - '0';
    if (a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    if (a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    return -1;
}

// 从 s[pos..pos+3] 读四个十六进制位; 非法字符抛 json_parse_error, 不能当成 0 继续编码
static unsigned int hex4(const std::string &s, unsigned int pos)
{
    int h[4];
    for (int i = 0; i < 4; i++)
    {
        h[i] = chartoint(s[pos + i]);
        if (h[i] < 0)
            throw json_parse_error("Invalid \\u escape in JSON string");
    }
    return static_cast<unsigned int>((h[0] << 12) | (h[1] << 8) | (h[2] << 4) | h[3]);
}

static std::string JSON_STRING(const std::string &jsonstr, unsigned int &offset)
{
    std::string result;
    unsigned int j = offset;
    bool closed = false;
    
    if (jsonstr[j] == 0x22)
    {
        j++;
    }
    
    for (; j < jsonstr.length(); j++)
    {
        if (jsonstr[j] == 0x5c)
        {
            if ((j + 1) >= jsonstr.length())
            {
                offset = jsonstr.length();
                return "";
            }
            
            switch (jsonstr[j + 1])
            {
            case 0x22:
                result += 0x22;
                j += 1;
                break;
            case 0x5c:
                result += 0x5c;
                j += 1;
                break;
            case 0x2f:
                result += 0x2f;
                j += 1;
                break;
            case 0x62:
                result += 0x08;
                j += 1;
                break;
            case 0x66:
                result += 0x0c;
                j += 1;
                break;
            case 0x6e:
                result += 0x0a;
                j += 1;
                break;
            case 0x72:
                result += 0x0d;
                j += 1;
                break;
            case 0x74:
                result += 0x09;
                j += 1;
                break;
            case 'u':
            {
                if ((j + 6) >= jsonstr.length())
                    throw json_parse_error("Truncated \\u escape in JSON string");

                unsigned int temp = hex4(jsonstr, j + 2);

                // 代理对合成一个码点; 落单的代理项编不成合法 UTF-8, 写 U+FFFD 顶上
                if (temp >= 0xD800 && temp <= 0xDBFF && (j + 12) < jsonstr.length() &&
                    jsonstr[j + 6] == 0x5c && jsonstr[j + 7] == 'u')
                {
                    unsigned int low = hex4(jsonstr, j + 8);
                    if (low >= 0xDC00 && low <= 0xDFFF)
                    {
                        temp = 0x10000 + ((temp - 0xD800) << 10) + (low - 0xDC00);
                        result += static_cast<char>(0xF0 | (temp >> 18));
                        result += static_cast<char>(0x80 | ((temp >> 12) & 0x3F));
                        result += static_cast<char>(0x80 | ((temp >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (temp & 0x3F));
                        j += 11;
                        break;
                    }
                }

                if (temp >= 0xD800 && temp <= 0xDFFF)
                {
                    result += "\xEF\xBF\xBD";
                }
                else if (temp < 0x80)
                {
                    result += static_cast<char>(temp);
                }
                else if (temp < 0x800)
                {
                    result += static_cast<char>(0xC0 | (temp >> 6));
                    result += static_cast<char>(0x80 | (temp & 0x3F));
                }
                else
                {
                    result += static_cast<char>(0xE0 | (temp >> 12));
                    result += static_cast<char>(0x80 | ((temp >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (temp & 0x3F));
                }
                j += 5;
            }
            break;
            default:
                result += jsonstr[j];
                result += jsonstr[j + 1];
                j += 1;
            }
        }
        else if (jsonstr[j] == 0x22)
        {
            closed = true;
            break;
        }
        else
        {
            result += jsonstr[j];
        }
    }

    // 字符串必须以 " 收尾, 否则后面的字节会被当成 JSON 结构继续解析
    if (!closed)
        throw json_parse_error("Unterminated string in JSON");

    offset = j + 1;
    return result;
}

static int JSON_OBJ(const std::string &json_val, obj_t &obj, unsigned int &offset, unsigned int level);
static int JSON_ARRAY(const std::string &json_val, obj_array &obj, unsigned int &offset, unsigned int level);
void from_json_internal(const std::string &json_str, obj_val &val, unsigned int &offset, unsigned int level);

static int JSON_OBJ(const std::string &json_val, obj_t &obj, unsigned int &offset, unsigned int level)
{
    if (level > JSON_MAX_DEPTH)
        throw json_parse_error("JSON nested depth exceeded");
    // 重复键取最后一个值(与 JS/Python/Go 一致), 成员位置停在首次出现处
    // 键存成 std::string: 成员数组扩容会搬动元素, string_view 指向的旧缓冲会失效
    // 查重索引等成员多了才建: 建表要一次桶分配加每键一个节点, 常见对象只有几个键,
    // 逐个建表的分配比线性比较更贵; 到阈值后补建索引, 成员接近上限时仍是 O(1) 查重
    constexpr std::size_t dedup_index_min = 32;
    std::unordered_map<std::string, std::size_t> seen;
    bool has_index = false;
    offset++;
    while (offset < json_val.size())
    {
        while (offset < json_val.size() && (json_val[offset] == ' ' || json_val[offset] == '\t' || json_val[offset] == '\r' || json_val[offset] == '\n'))
            offset++;
        if (offset >= json_val.size())
            throw json_parse_error("Unexpected end of JSON");
        if (json_val[offset] == '}')
        {
            offset++;
            return 0;
        }
        if (json_val[offset] != '"')
            throw json_parse_error("Expected string key in object");
        std::string key = JSON_STRING(json_val, offset);
        if (key.size() > JSON_MAX_STRING_LENGTH)
            throw json_parse_error("JSON key too long");
        while (offset < json_val.size() && (json_val[offset] == ' ' || json_val[offset] == '\t' || json_val[offset] == '\r' || json_val[offset] == '\n'))
            offset++;
        if (offset >= json_val.size() || json_val[offset] != ':')
            throw json_parse_error("Expected ':' after key");
        offset++;
        obj_val val;
        from_json_internal(json_val, val, offset, level + 1);
        std::size_t dup_pos = obj._data.size();
        if (has_index)
        {
            auto seen_it = seen.find(key);
            if (seen_it != seen.end())
                dup_pos = seen_it->second;
        }
        else
        {
            for (std::size_t i = 0; i < obj._data.size(); i++)
            {
                if (obj._data[i].first == key)
                {
                    dup_pos = i;
                    break;
                }
            }
        }
        if (dup_pos == obj._data.size())
        {
            if (obj._data.size() >= JSON_MAX_OBJECT_MEMBERS)
                throw json_parse_error("JSON object has too many members");
            obj._data.emplace_back(key, std::move(val));
            if (has_index)
                seen.emplace(std::move(key), obj._data.size() - 1);
            else if (obj._data.size() == dedup_index_min)
            {
                // 补建索引时键只能拷贝(成员还在用); 这个代价每个大对象只付一次
                has_index = true;
                for (std::size_t i = 0; i < obj._data.size(); i++)
                    seen.emplace(obj._data[i].first, i);
            }
        }
        else
        {
            obj._data[dup_pos].second = std::move(val);
        }
        while (offset < json_val.size() && (json_val[offset] == ' ' || json_val[offset] == '\t' || json_val[offset] == '\r' || json_val[offset] == '\n'))
            offset++;
        if (offset >= json_val.size())
            throw json_parse_error("Unexpected end of JSON");
        if (json_val[offset] == ',')
        {
            offset++;
            continue;
        }
        if (json_val[offset] == '}')
        {
            offset++;
            return 0;
        }
        throw json_parse_error("Unexpected character in object");
    }
    throw json_parse_error("Unexpected end of JSON");
}

static int JSON_ARRAY(const std::string &json_val, obj_array &obj, unsigned int &offset, unsigned int level)
{
    if (level > JSON_MAX_DEPTH)
        throw json_parse_error("JSON nested depth exceeded");
    offset++;
    while (offset < json_val.size())
    {
        while (offset < json_val.size() && (json_val[offset] == ' ' || json_val[offset] == '\t' || json_val[offset] == '\r' || json_val[offset] == '\n'))
            offset++;
        if (offset >= json_val.size())
            throw json_parse_error("Unexpected end of JSON");
        if (json_val[offset] == ']')
        {
            offset++;
            return 0;
        }
        obj_val val;
        from_json_internal(json_val, val, offset, level + 1);
        if (obj._data.size() >= JSON_MAX_ARRAY_ELEMENTS)
            throw json_parse_error("JSON array has too many elements");
        obj._data.push_back(std::move(val));
        while (offset < json_val.size() && (json_val[offset] == ' ' || json_val[offset] == '\t' || json_val[offset] == '\r' || json_val[offset] == '\n'))
            offset++;
        if (offset >= json_val.size())
            throw json_parse_error("Unexpected end of JSON");
        if (json_val[offset] == ',')
        {
            offset++;
            continue;
        }
        if (json_val[offset] == ']')
        {
            offset++;
            return 0;
        }
        throw json_parse_error("Unexpected character in array");
    }
    throw json_parse_error("Unexpected end of JSON");
}

void from_json_internal(const std::string &json_str, obj_val &val, unsigned int &offset, unsigned int level)
{
    while (offset < json_str.size() && (json_str[offset] == ' ' || json_str[offset] == '\t' || json_str[offset] == '\r' || json_str[offset] == '\n'))
        offset++;
    // 空正文当成 NIL: 调用方(读数据库空列/空 session 文件)依赖这条既有语义
    if (offset >= json_str.size())
        return;
    
    if (json_str[offset] == '{')
    {
        val.obj = new obj_t;
        val.set_type(obj_type::OBJECT);
        JSON_OBJ(json_str, *val.obj, offset, level);
    }
    else if (json_str[offset] == '[')
    {
        val.array_val = new obj_array;
        val.set_type(obj_type::ARRAY);
        JSON_ARRAY(json_str, *val.array_val, offset, level);
    }
    else if (json_str[offset] == '"')
    {
        val.str_ = JSON_STRING(json_str, offset);
        if (val.str_.size() > JSON_MAX_STRING_LENGTH)
            throw json_parse_error("JSON string value too long");
        val.set_type(obj_type::STRING);
    }
    else if (json_str[offset] == 't' && offset + 3 < json_str.size() && json_str.substr(offset, 4) == "true")
    {
        val.lval = 1;
        val.set_type(obj_type::BOOL);
        offset += 4;
    }
    else if (json_str[offset] == 'f' && offset + 4 < json_str.size() && json_str.substr(offset, 5) == "false")
    {
        val.lval = 0;
        val.set_type(obj_type::BOOL);
        offset += 5;
    }
    else if (json_str[offset] == 'n' && offset + 3 < json_str.size() && json_str.substr(offset, 4) == "null")
    {
        val.set_type(obj_type::NIL);
        offset += 4;
    }
    else
    {
        std::string num_str;
        bool has_dot = false;
        bool has_e = false;
        while (offset < json_str.size() && (json_str[offset] == '-' || json_str[offset] == '+' ||
                                           (json_str[offset] >= '0' && json_str[offset] <= '9') ||
                                           json_str[offset] == '.' || json_str[offset] == 'e' || json_str[offset] == 'E'))
        {
            if (json_str[offset] == '.')
                has_dot = true;
            if (json_str[offset] == 'e' || json_str[offset] == 'E')
                has_e = true;
            num_str.push_back(json_str[offset]);
            offset++;
        }
        
        if (num_str.empty())
            throw json_parse_error("Unexpected character in JSON");
        
        if (num_str.size() > 30)
            throw json_parse_error("JSON number too long");
        
        if (has_dot || has_e)
        {
            try
            {
                val.dval = std::stod(num_str);
                if (!std::isfinite(val.dval))
                    throw json_parse_error("JSON number is not finite");
                val.set_type(obj_type::DOUBLE);
            }
            catch (const std::exception &)
            {
                throw json_parse_error("Invalid JSON number");
            }
        }
        else
        {
            try
            {
                long long l = std::stoll(num_str);
                if (l >= INT_MIN && l <= INT_MAX)
                {
                    val.lval = l;
                    val.set_type(obj_type::INT);
                }
                else
                {
                    val.lval = l;
                    val.set_type(obj_type::LONG);
                }
            }
            catch (const std::exception &)
            {
                try
                {
                    unsigned long long u = std::stoull(num_str);
                    if (u <= UINT_MAX)
                    {
                        val.uval = u;
                        val.set_type(obj_type::UINT);
                    }
                    else
                    {
                        val.uval = u;
                        val.set_type(obj_type::ULONG);
                    }
                }
                catch (const std::exception &)
                {
                    throw json_parse_error("Invalid JSON number");
                }
            }
        }
    }
}

void obj_val::from_json(const std::string &json_str)
{
    if (json_str.size() > JSON_MAX_SIZE)
        throw json_parse_error("JSON size exceeds maximum allowed");
    clear();
    unsigned int offset = 0;
    from_json_internal(json_str, *this, offset, 0);
    while (offset < json_str.size() && (json_str[offset] == ' ' || json_str[offset] == '\t' || json_str[offset] == '\r' || json_str[offset] == '\n'))
        offset++;
    // 一个值之后还有内容说明输入不是单个 JSON 文档, 直接拒绝而不是静默丢掉尾巴
    if (offset < json_str.size())
        throw json_parse_error("Unexpected trailing characters in JSON");
}

std::string obj_val::to_json() const
{
    return to_json_internal(*this, 0);
}

// 把 src 按 JSON 字符串语法写进 out(含两侧引号), 非 ASCII 的 UTF-8 序列转成 \uXXXX
static void append_json_escaped(std::string &result, std::string_view src)
{
    const char *text = src.data();
    unsigned int text_length = static_cast<unsigned int>(src.size());
    unsigned char c, t = 0;
    static constexpr unsigned char str_[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    result.push_back(0x22);
    for (unsigned int i = 0; i < text_length; i++)
    {
        c = *(text + i);
        if (c < 0x80)
        {
            switch (c)
            {
            case 0x22:
                result.push_back(0x5c);
                result.push_back(0x22);
                break;
            case 0x60:
                result.push_back(0x60);
                break;
            case 0x27:
                result.push_back(0x27);
                break;
            case 0x2f:
                result.push_back('/');
                break;
            case 0x08:
                result.push_back(0x5c);
                result.push_back('b');
                break;
            case 0x0c:
                result.push_back(0x5c);
                result.push_back('f');
                break;
            case 0x0a:
                result.push_back(0x5c);
                result.push_back('n');
                break;
            case 0x0d:
                result.push_back(0x5c);
                result.push_back('r');
                break;
            case 0x09:
                result.push_back(0x5c);
                result.push_back('t');
                break;
            case 0x5c:
                result.push_back(0x5c);
                result.push_back(0x5c);
                break;
            default:
                if (c < 0x20)
                {
                    // JSON 字符串中 0x00-0x1F 控制字符必须转义为 \u00XX, 否则生成非法JSON
                    result.push_back(0x5c);
                    result.push_back(0x75);
                    result.push_back('0');
                    result.push_back('0');
                    result.push_back(str_[(c >> 4) & 0x0F]);
                    result.push_back(str_[c & 0x0F]);
                }
                else
                {
                    result.push_back(c);
                }
            }
        }
        else if (c >= 0x80 && c < 0xC0)
        {
            result.push_back(c);
        }
        else if (c >= 0xC0 && c < 0xE0)
        {
            unsigned char b1, b2, cp[2], temp;
            if ((i + 1) >= text_length)
            {
                result.push_back(c);
            }
            else
            {
                b1 = c;
                b2 = *(text + i + 1);
                if ((b2 & 0xE0) != 0x80)
                {
                    result.push_back(c);
                }
                else
                {
                    result.push_back(0x5c);
                    result.push_back(0x75);

                    temp = (b1 >> 2) & 0x07;

                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    temp = (b1 << 6) + (b2 & 0x3F);
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    i++;
                }
            }
        }
        else if (c >= 0xE0 && c < 0xF0)
        {
            unsigned char b1, b2, b3, cp[2], temp;
            if ((i + 2) >= text_length)
            {
                result.push_back(c);
            }
            else
            {
                b1 = c;
                b2 = *(text + i + 1);
                b3 = *(text + i + 2);
                if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80))
                {
                    result.push_back(c);
                }
                else
                {
                    result.push_back(0x5c);
                    result.push_back(0x75);

                    temp = (b1 << 4) + ((b2 >> 2) & 0x0F);
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    temp = (b2 << 6) + (b3 & 0x3F);
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);
                    i += 2;
                }
            }
        }
        else if (c >= 0xF0 && c < 0xF8)
        {
            unsigned char b1, b2, b3, b4, cp[2], temp;
            unsigned int out = 0, untext = 0;
            if ((i + 3) >= text_length)
            {
                result.push_back(c);
            }
            else
            {
                b1 = c;
                b2 = *(text + i + 1);
                b3 = *(text + i + 2);
                b4 = *(text + i + 3);

                if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80))
                {
                    result.push_back(c);
                }
                else
                {
                    result.push_back(0x5c);
                    result.push_back(0x75);

                    unsigned char *pOutput = (unsigned char *)&out;
                    *pOutput       = (b3 << 6) + (b4 & 0x3F);
                    *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
                    *(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
                    untext         = (((((out - 0x10000) >> 10) | 0xD800) << 16) | (((out - 0x10000) & 0x3FF) | 0xDC00));

                    pOutput = (unsigned char *)&untext;

                    temp = pOutput[3];
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    temp = pOutput[2];
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    result.push_back(0x5c);
                    result.push_back(0x75);

                    temp = pOutput[1];
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    temp = pOutput[0];
                    t     = temp & 0xF;
                    cp[1] = str_[t];
                    temp  = temp >> 4;
                    t     = temp & 0xF;
                    cp[0] = str_[t];

                    result.push_back(cp[0]);
                    result.push_back(cp[1]);

                    i += 3;
                }
            }
        }
        else if (c >= 0xF8 && c < 0xFC)
        {
            result.push_back(c);
        }
        else
        {
            result.push_back(c);
        }
    }
    result.push_back(0x22);
}

// 把 src 按 JSON 字符串语法写进 out(含两侧引号), 保留 UTF-8 原文
static void append_json_raw(std::string &result, std::string_view src)
{
    result.push_back(0x22);
    for (char ch : src)
    {
        unsigned char c = static_cast<unsigned char>(ch);
        switch (c)
        {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '/': result += "\\/"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 0x20)
            {
                // JSON 字符串中 0x00-0x1F 控制字符必须转义为 \u00XX
                static constexpr char hex_[] = "0123456789abcdef";
                result += "\\u00";
                result.push_back(hex_[(c >> 4) & 0x0F]);
                result.push_back(hex_[c & 0x0F]);
            }
            else
            {
                result.push_back(ch);
            }
            break;
        }
    }
    result.push_back(0x22);
}

std::string to_json_internal(const obj_val &val, unsigned int level)
{
    if (level > JSON_MAX_DEPTH)
        throw json_parse_error("JSON nested depth exceeded during serialization");
    
    switch (val._val_type)
    {
    case obj_type::NIL:
        return "null";
    case obj_type::BOOL:
        return val.lval ? "true" : "false";
    case obj_type::STRING:
    {
        std::string result;
        append_json_escaped(result, val.str_);
        return result;
    }
    case obj_type::INT:
        return std::to_string(val.lval);
    case obj_type::UINT:
        return std::to_string(val.uval);
    case obj_type::LONG:
        return std::to_string(val.lval);
    case obj_type::ULONG:
        return std::to_string(val.uval);
    case obj_type::DOUBLE:
    {
        // JSON 没有 NaN/Inf 字面量, 与 NIL 一样输出 null, 避免整条响应抛异常
        if (!std::isfinite(val.dval))
            return "null";
        char buffer[64];
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), val.dval);
        std::string result(buffer, ptr);
        if (result.find('.') == std::string::npos && result.find('e') == std::string::npos)
            result += ".0";
        return result;
    }
    case obj_type::OBJECT:
    {
        std::string result = "{";
        bool first = true;
        for (const auto &pair : val.obj->_data)
        {
            if (!first)
                result += ",";
            first = false;
            append_json_escaped(result, pair.first);
            result.push_back(':');
            result += to_json_internal(pair.second, level + 1);
        }
        result += "}";
        return result;
    }
    case obj_type::ARRAY:
    {
        std::string result = "[";
        bool first = true;
        for (const auto &item : val.array_val->_data)
        {
            if (!first)
                result += ",";
            first = false;
            result += to_json_internal(item, level + 1);
        }
        result += "]";
        return result;
    }
    default:
        return "null";
    }
}

std::string obj_val::to_json_uncode() const
{
    return to_json_uncode_internal(*this, 0);
}

std::string to_json_uncode_internal(const obj_val &val, unsigned int level)
{
    if (level > JSON_MAX_DEPTH)
        throw json_parse_error("JSON nested depth exceeded during serialization");

    switch (val._val_type)
    {
    case obj_type::NIL:
        return "null";
    case obj_type::BOOL:
        return val.lval ? "true" : "false";
    case obj_type::STRING:
    {
        std::string result;
        append_json_raw(result, val.str_);
        return result;
    }
    case obj_type::INT:
        return std::to_string(val.lval);
    case obj_type::UINT:
        return std::to_string(val.uval);
    case obj_type::LONG:
        return std::to_string(val.lval);
    case obj_type::ULONG:
        return std::to_string(val.uval);
    case obj_type::DOUBLE:
    {
        // JSON 没有 NaN/Inf 字面量, 与 NIL 一样输出 null, 避免整条响应抛异常
        if (!std::isfinite(val.dval))
            return "null";
        char buffer[64];
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), val.dval);
        std::string result(buffer, ptr);
        if (result.find('.') == std::string::npos && result.find('e') == std::string::npos)
            result += ".0";
        return result;
    }
    case obj_type::OBJECT:
    {
        std::string result = "{";
        bool first = true;
        for (const auto &pair : val.obj->_data)
        {
            if (!first)
                result += ",";
            first = false;
            append_json_raw(result, pair.first);
            result.push_back(':');
            result += to_json_uncode_internal(pair.second, level + 1);
        }
        result += "}";
        return result;
    }
    case obj_type::ARRAY:
    {
        std::string result = "[";
        bool first = true;
        for (const auto &item : val.array_val->_data)
        {
            if (!first)
                result += ",";
            first = false;
            result += to_json_uncode_internal(item, level + 1);
        }
        result += "]";
        return result;
    }
    default:
        return "null";
    }
}

obj_val &obj_val::operator=(const std::vector<float> &v)
{
    clear();
    array_val = new obj_array;
    _val_type = obj_type::ARRAY;
    for (float f : v)
        array_val->_data.emplace_back(f);
    return *this;
}

obj_val &obj_val::operator=(const std::vector<long long> &v)
{
    clear();
    array_val = new obj_array;
    _val_type = obj_type::ARRAY;
    for (long long l : v)
        array_val->_data.emplace_back(l);
    return *this;
}

obj_val &obj_val::operator=(const std::vector<std::string> &v)
{
    clear();
    array_val = new obj_array;
    _val_type = obj_type::ARRAY;
    for (const std::string &s : v)
        array_val->_data.emplace_back(s);
    return *this;
}

obj_val &obj_val::operator=(const std::map<std::string, int> &v)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    for (const auto &pair : v)
        obj->_data.emplace_back(pair.first, pair.second);
    return *this;
}

obj_val &obj_val::operator=(const std::map<std::string, unsigned int> &v)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    for (const auto &pair : v)
        obj->_data.emplace_back(pair.first, pair.second);
    return *this;
}

obj_val &obj_val::operator=(const std::map<std::string, std::string> &v)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    for (const auto &pair : v)
        obj->_data.emplace_back(pair.first, pair.second);
    return *this;
}

obj_val &obj_val::find(const obj_val &v)
{
    std::string key = static_cast<std::string>(v);
    return find(key);
}

obj_val &obj_val::find(const std::string &v)
{
    if (_val_type == obj_type::ARRAY)
    {
        for (auto &val : array_val->_data)
        {
            if (val == v)
                return val;
        }
    }
    else if (_val_type == obj_type::OBJECT)
    {
        for (auto &pair : obj->_data)
        {
            if (pair.second == v)
                return pair.second;
        }
    }
    throw std::runtime_error("Value not found");
}

int obj_val::casecmp(const std::string &str) const
{
    if (_val_type != obj_type::STRING)
        return -1;
    size_t len = str_.size();
    if (len != str.size())
        return len > str.size() ? 1 : -1;
    for (size_t i = 0; i < len; i++)
    {
        char c1 = std::toupper(static_cast<unsigned char>(str_[i]));
        char c2 = std::toupper(static_cast<unsigned char>(str[i]));
        if (c1 != c2)
            return static_cast<int>(c1) - static_cast<int>(c2);
    }
    return 0;
}

std::vector<std::pair<std::string, obj_val>> &obj_val::as_object()
{
    if (_val_type == obj_type::OBJECT)
    {
        return obj->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        obj       = new obj_t;
        _val_type = obj_type::OBJECT;
        return obj->_data;
    }
    else if (_val_type == obj_type::ARRAY)
    {
        if (array_val->_data.size() == 0)
        {
            clear();
            obj       = new obj_t;
            _val_type = obj_type::OBJECT;
            return obj->_data;
        }
    }
    throw std::runtime_error("Not an object");
}

std::vector<obj_val> &obj_val::as_array()
{
    if (_val_type == obj_type::ARRAY)
    {
        return array_val->_data;
    }
    else if (_val_type == obj_type::NIL)
    {
        array_val = new obj_array;
        _val_type = obj_type::ARRAY;
        return array_val->_data;
    }
    else if (_val_type == obj_type::OBJECT)
    {
        if (obj->_data.size() == 0)
        {
            clear();
            array_val = new obj_array;
            _val_type = obj_type::ARRAY;
            return array_val->_data;
        }
    }
    throw std::runtime_error("as_array no array");
}

std::string obj_val::as_string()
{
    if (_val_type == obj_type::STRING)
    {
        return static_cast<std::string>(*this);
    }
    return to_string();
}

std::string obj_val::as_string(std::string_view default_val)
{
    if (_val_type == obj_type::STRING)
        return str_;
    return std::string(default_val);
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<int> &val)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    size_t min_size = std::min(key.size(), val.size());
    for (size_t i = 0; i < min_size; i++)
        obj->_data.emplace_back(key[i], val[i]);
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<float> &val)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    size_t min_size = std::min(key.size(), val.size());
    for (size_t i = 0; i < min_size; i++)
        obj->_data.emplace_back(key[i], val[i]);
}

void obj_val::zip(const std::vector<std::string> &key, const std::vector<std::string> &val)
{
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    size_t min_size = std::min(key.size(), val.size());
    for (size_t i = 0; i < min_size; i++)
        obj->_data.emplace_back(key[i], val[i]);
}

void obj_val::zip(const std::vector<std::string> &key, const obj_val &val)
{
    if (val._val_type != obj_type::ARRAY)
        return;
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    size_t min_size = std::min(key.size(), val.array_val->_data.size());
    for (size_t i = 0; i < min_size; i++)
        obj->_data.emplace_back(key[i], val.array_val->_data[i]);
}

void obj_val::zip(const obj_val &key, const obj_val &val)
{
    // this 就是 key/val 时 clear() 会先把要读的数组删掉, 先做快照
    if (this == &key || this == &val)
    {
        obj_val key_copy(key);
        obj_val val_copy(val);
        zip(key_copy, val_copy);
        return;
    }
    if (key._val_type != obj_type::ARRAY || val._val_type != obj_type::ARRAY)
        return;
    clear();
    obj = new obj_t;
    _val_type = obj_type::OBJECT;
    size_t min_size = std::min(key.array_val->_data.size(), val.array_val->_data.size());
    for (size_t i = 0; i < min_size; i++)
        obj->_data.emplace_back(static_cast<std::string>(key.array_val->_data[i]), val.array_val->_data[i]);
}

const std::vector<std::pair<std::string, obj_val>> &obj_val::ref_obj() const
{
    if (_val_type != obj_type::OBJECT)
        throw std::runtime_error("Not an object");
    return obj->_data;
}

const std::vector<obj_val> &obj_val::ref_array() const
{
    if (_val_type != obj_type::ARRAY)
        throw std::runtime_error("Not an array");
    return array_val->_data;
}

static const obj_val &get_obj_val(const obj_val &obj, std::string_view key)
{
    if (obj.get_type() != obj_type::OBJECT)
    {
        static obj_val nil;
        return nil;
    }
    for (const auto &pair : obj.ref_obj())
    {
        if (pair.first == key)
            return pair.second;
    }
    static obj_val nil;
    return nil;
}

obj_val obj_val::multi_sort(std::string_view key, unsigned char order)
{
    if (_val_type != obj_type::ARRAY)
        return {};
    obj_val result = *this;
    std::sort(result.array_val->_data.begin(), result.array_val->_data.end(),
        [key, order](const obj_val &a, const obj_val &b) {
            const obj_val &va = get_obj_val(a, key);
            const obj_val &vb = get_obj_val(b, key);
            if (order == SORT_ASC)
                return static_cast<double>(va) < static_cast<double>(vb);
            return static_cast<double>(va) > static_cast<double>(vb);
        });
    return result;
}

obj_val obj_val::multi_sort(std::string_view key, unsigned char order, std::string_view key2, unsigned char order2)
{
    if (_val_type != obj_type::ARRAY)
        return {};
    obj_val result = *this;
    std::sort(result.array_val->_data.begin(), result.array_val->_data.end(),
        [key, order, key2, order2](const obj_val &a, const obj_val &b) {
            const obj_val &va = get_obj_val(a, key);
            const obj_val &vb = get_obj_val(b, key);
            double da = static_cast<double>(va);
            double db = static_cast<double>(vb);
            if (order == SORT_ASC)
            {
                if (da != db)
                    return da < db;
            }
            else
            {
                if (da != db)
                    return da > db;
            }
            const obj_val &va2 = get_obj_val(a, key2);
            const obj_val &vb2 = get_obj_val(b, key2);
            double da2 = static_cast<double>(va2);
            double db2 = static_cast<double>(vb2);
            if (order2 == SORT_ASC)
                return da2 < db2;
            return da2 > db2;
        });
    return result;
}

obj_val::operator double() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0.0;
    case obj_type::BOOL: return lval ? 1 : 0;
    case obj_type::STRING:
        if (str_.empty()) return 0.0;
        try { return std::stod(str_); }
        catch (const std::exception &) { return 0.0; }
    case obj_type::INT: return lval;
    case obj_type::UINT: return uval;
    case obj_type::LONG: return lval;
    case obj_type::ULONG: return uval;
    case obj_type::DOUBLE: return dval;
    default: return 0.0;
    }
}

obj_val::operator float() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0.0f;
    case obj_type::BOOL: return lval ? 1.0f : 0.0f;
    case obj_type::STRING:
        if (str_.empty()) return 0.0f;
        try { return std::stof(str_); }
        catch (const std::exception &) { return 0.0f; }
    case obj_type::INT: return static_cast<float>(lval);
    case obj_type::UINT: return static_cast<float>(uval);
    case obj_type::LONG: return static_cast<float>(lval);
    case obj_type::ULONG: return static_cast<float>(uval);
    case obj_type::DOUBLE: return static_cast<float>(dval);
    default: return 0.0f;
    }
}

obj_val::operator long long() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0;
    case obj_type::BOOL: return lval ? 1 : 0;
    case obj_type::STRING:
        if (str_.empty()) return 0;
        try { return std::stoll(str_); }
        catch (const std::exception &) { return 0; }
    case obj_type::INT: return lval;
    case obj_type::UINT:
        return uval > static_cast<unsigned long long>(LLONG_MAX) ? LLONG_MAX : static_cast<long long>(uval);
    case obj_type::LONG: return lval;
    case obj_type::ULONG:
        return uval > static_cast<unsigned long long>(LLONG_MAX) ? LLONG_MAX : static_cast<long long>(uval);
    case obj_type::DOUBLE: return static_cast<long long>(dval);
    default: return 0;
    }
}

obj_val::operator unsigned long long() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return 0;
    case obj_type::BOOL: return lval ? 1 : 0;
    case obj_type::STRING:
        if (str_.empty()) return 0;
        try { return std::stoull(str_); }
        catch (const std::exception &) { return 0; }
    case obj_type::INT: return static_cast<unsigned long long>(lval);
    case obj_type::UINT: return uval;
    case obj_type::LONG: return static_cast<unsigned long long>(lval);
    case obj_type::ULONG: return uval;
    case obj_type::DOUBLE: return static_cast<unsigned long long>(dval);
    default: return 0;
    }
}

obj_val::operator int() const
{
    return static_cast<int>(static_cast<long long>(*this));
}

obj_val::operator unsigned int() const
{
    return static_cast<unsigned int>(static_cast<unsigned long long>(*this));
}

obj_val::operator bool() const
{
    switch (_val_type)
    {
    case obj_type::NIL: return false;
    case obj_type::BOOL: return lval != 0;
    case obj_type::STRING: return str_truthy(str_);
    case obj_type::INT: return lval != 0;
    case obj_type::UINT: return uval != 0;
    case obj_type::LONG: return lval != 0;
    case obj_type::ULONG: return uval != 0;
    case obj_type::DOUBLE: return dval != 0.0;
    case obj_type::OBJECT: return obj && !obj->_data.empty();
    case obj_type::ARRAY: return array_val && !array_val->_data.empty();
    default: return false;
    }
}

obj_val::operator std::string() const
{
    if (_val_type == obj_type::OBJECT) return "{}";
    if (_val_type == obj_type::ARRAY) return "[]";
    if (_val_type == obj_type::STRING) return str_;
    if (_val_type == obj_type::BOOL) return lval ? "true" : "false";
    if (_val_type == obj_type::INT) return std::to_string(lval);
    if (_val_type == obj_type::UINT) return std::to_string(uval);
    if (_val_type == obj_type::DOUBLE) return std::to_string(dval);
    if (_val_type == obj_type::LONG) return std::to_string(lval);
    if (_val_type == obj_type::ULONG) return std::to_string(uval);
    return "";
}
}

// 向流输出 obj_val, 采用框架标准 JSON 序列化
std::ostream &operator<<(std::ostream &os, const http::obj_val &val)
{
    os << val.to_json();
    return os;
}
