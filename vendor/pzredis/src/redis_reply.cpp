/*
 * pzredis protocol layer implementation
 * author Huang ziquan (黄自权)
 * date 2026-10-01
 */
#include "redis_reply.h"

#include <algorithm>
#include <cstdlib>

namespace pz
{
namespace redis
{

std::atomic<unsigned long long> &reply_parser_t::resp3_reject()
{
    static std::atomic<unsigned long long> instance{0};
    return instance;
}

bool reply_parser_t::read_line(const unsigned char *data, std::size_t length, std::size_t &offset, std::string &line)
{
    line.clear();
    while (offset + 1 < length)
    {
        if (data[offset] == '\r' && data[offset + 1] == '\n')
        {
            offset += 2;
            return true;
        }
        line.push_back(static_cast<char>(data[offset]));
        offset++;
    }
    // 没找到完整行：offset 停在截断处，调用方按 need_more 回滚
    return false;
}

bool reply_parser_t::try_parse_longlong(const std::string &s, long long &val)
{
    if (s.empty())
        return false;
    try
    {
        std::size_t pos = 0;
        val             = std::stoll(s, &pos);
        return pos == s.size();
    }
    catch (...)
    {
        return false;
    }
}

bool reply_parser_t::try_parse_double(const std::string &s, double &val)
{
    if (s.empty())
        return false;
    try
    {
        val = std::stod(s);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

parse_status reply_parser_t::parse_bulk_like(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, reply_type t)
{
    std::size_t s = consumed;
    std::string len_str;
    if (!read_line(data, length, consumed, len_str))
    {
        consumed = s;
        return parse_status::need_more;
    }
    long long bulk_len = 0;
    if (!try_parse_longlong(len_str, bulk_len))
    {
        consumed = s;
        return parse_status::error;
    }
    if (bulk_len < 0)
    {
        out.type    = t;
        out.is_null = true;
        out.str_value.clear();
        return parse_status::ok;
    }
    if (static_cast<std::size_t>(bulk_len) > kMaxBulk)
    {
        consumed = s;
        return parse_status::error;
    }
    if (consumed + static_cast<std::size_t>(bulk_len) + 2 > length)
    {
        consumed = s;
        return parse_status::need_more;
    }
    out.type    = t;
    out.is_null = false;
    out.str_value.assign(reinterpret_cast<const char *>(data + consumed), static_cast<std::size_t>(bulk_len));
    consumed += static_cast<std::size_t>(bulk_len);
    if (data[consumed] != '\r' || data[consumed + 1] != '\n')
    {
        consumed = s;
        return parse_status::error;
    }
    consumed += 2;
    return parse_status::ok;
}

parse_status reply_parser_t::parse_collection(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, unsigned depth)
{
    std::size_t s = consumed;
    std::string len_str;
    if (!read_line(data, length, consumed, len_str))
    {
        consumed = s;
        return parse_status::need_more;
    }
    long long array_len = 0;
    if (!try_parse_longlong(len_str, array_len))
    {
        consumed = s;
        return parse_status::error;
    }
    if (array_len < 0)
    {
        // 类型已由 feed() 在调用前按首字符前缀设好，这里只标记 null。
        out.is_null = true;
        out.array_value.clear();
        return parse_status::ok;
    }
    out.is_null = false;
    out.array_value.clear();
    out.array_value.reserve(static_cast<std::size_t>(array_len < 0 ? 0 : std::min<long long>(array_len, 4096)));
    for (long long i = 0; i < array_len; ++i)
    {
        reply_t elem;
        parse_status st = feed(data, length, consumed, elem, depth + 1);
        if (st != parse_status::ok)
        {
            out.array_value.clear();
            consumed = s;
            return st;
        }
        out.array_value.push_back(std::move(elem));
    }
    return parse_status::ok;
}

parse_status reply_parser_t::feed(const unsigned char *data, std::size_t length, std::size_t &consumed, reply_t &out, unsigned depth)
{
    if (consumed >= length)
        return parse_status::need_more;
    if (depth > kMaxDepth)
    {
        return parse_status::error;// 嵌套过深
    }
    std::size_t start = consumed;
    char prefix       = static_cast<char>(data[consumed]);
    consumed++;

    switch (prefix)
    {
    case '+':// simple string
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type      = reply_type::simple_string;
        out.is_null   = false;
        out.str_value = std::move(line);
        return parse_status::ok;
    }
    case '-':// error
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type      = reply_type::error;
        out.is_null   = false;
        out.str_value = std::move(line);
        return parse_status::ok;
    }
    case '!':// blob error（RESP3）——按 bulk 收，类型记为 blob_error
    {
        out.type        = reply_type::blob_error;
        parse_status st = parse_bulk_like(data, length, consumed, out, reply_type::blob_error);
        if (st != parse_status::ok)
            out = reply_t{};
        return st;
    }
    case ':':// integer
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type    = reply_type::integer;
        out.is_null = false;
        long long v = 0;
        if (try_parse_longlong(line, v))
            out.int_value = v;
        else
            out.str_value = std::move(line);
        return parse_status::ok;
    }
    case '$':// bulk string
    {
        out.type        = reply_type::bulk_string;
        parse_status st = parse_bulk_like(data, length, consumed, out, reply_type::bulk_string);
        if (st != parse_status::ok)
            out = reply_t{};
        return st;
    }
    case '=':// verbatim（RESP3）——按 bulk 收，丢掉前 3 字节 "fmt:"
    {
        out.type        = reply_type::verbatim;
        parse_status st = parse_bulk_like(data, length, consumed, out, reply_type::verbatim);
        if (st == parse_status::ok && out.str_value.size() > 3)
            out.str_value = out.str_value.substr(4);// 跳过 "xxx:" 前缀
        else if (st != parse_status::ok)
            out = reply_t{};
        return st;
    }
    case '*':// array
    {
        out.type = reply_type::array;
        return parse_collection(data, length, consumed, out, depth);
    }
    case '%':// map（RESP3）——显式拒绝，见 reply_parser_t::resp3_reject() 的说明。
             // map 的帧是 2N 个元素平铺（key,value 成对），按 array 收下等于静默拿出一个
             // "长度对、内容成对"的错形状；这里宁可判协议错、让调用方把连接作废。
    {
        resp3_reject().fetch_add(1);
        consumed = start;
        return parse_status::error;
    }
    case '~':// set（RESP3）——同样显式拒绝，理由同 map
    {
        resp3_reject().fetch_add(1);
        consumed = start;
        return parse_status::error;
    }
    case '>':// push（RESP3）——按 array 收，二期 pubsub 使用
    {
        out.type = reply_type::push;
        return parse_collection(data, length, consumed, out, depth);
    }
    case ',':// double
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type    = reply_type::double_value;
        out.is_null = false;
        double v    = 0.0;
        if (try_parse_double(line, v))
            out.double_value = v;
        else
            out.str_value = std::move(line);
        return parse_status::ok;
    }
    case '#':// boolean
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type       = reply_type::boolean;
        out.is_null    = false;
        out.bool_value = (line == "t" || line == "1" || line == "true");
        return parse_status::ok;
    }
    case '(':// big number
    {
        std::string line;
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type      = reply_type::big_number;
        out.is_null   = false;
        out.str_value = std::move(line);
        return parse_status::ok;
    }
    case '_':// null（RESP3）
    {
        std::string line;
        // null 行通常为空，但严格应读掉这一行
        if (!read_line(data, length, consumed, line))
        {
            consumed = start;
            return parse_status::need_more;
        }
        out.type    = reply_type::null;
        out.is_null = true;
        out.str_value.clear();
        return parse_status::ok;
    }
    default:
        consumed = start;
        return parse_status::error;
    }
}

}// namespace redis
}// namespace pz
