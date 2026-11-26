/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http2_frame.cpp
 *  @date 2022-10-15
 *
 *  http2 protocol make header file
 *
 *
 */
#include "http2_frame.h"
#include "http2_huffman.h"

namespace http
{
std::map<std::string, unsigned char> http2_header_codes_table = {
    {"empty", 0},
    {"authority", 1},
    {"GET", 2},
    {"POST", 3},
    {"path", 4},
    {"index", 5},
    {"http", 6},
    {"https", 7},
    {"200", 8},
    {"204", 9},
    {"206", 10},
    {"304", 11},
    {"400", 12},
    {"404", 13},
    {"500", 14},
    {"accept-charset", 15},
    {"accept-encoding", 16},
    {"accept-language", 17},
    {"accept-ranges", 18},
    {"accept", 19},
    {"access-control-allow-origin", 20},
    {"age", 21},
    {"allow", 22},
    {"authorization", 23},
    {"cache-control", 24},
    {"content-disposition", 25},
    {"content-encoding", 26},
    {"content-language", 27},
    {"content-length", 28},
    {"content-location", 29},
    {"content-range", 30},
    {"content-type", 31},
    {"cookie", 32},
    {"date", 33},
    {"etag", 34},
    {"expect", 35},
    {"expires", 36},
    {"from", 37},
    {"host", 38},
    {"if-match", 39},
    {"if-modified-since", 40},
    {"if-none-match", 41},
    {"if-range", 42},
    {"if-unmodified-since", 43},
    {"last-modified", 44},
    {"link", 45},
    {"location", 46},
    {"max-forwards", 47},
    {"proxy-authenticate", 48},
    {"proxy-authorization", 49},
    {"range", 50},
    {"referer", 51},
    {"refresh", 52},
    {"retry-after", 53},
    {"server", 54},
    {"set-cookie", 55},
    {"strict-transport-security", 56},
    {"transfer-encoding", 57},
    {"user-agent", 58},
    {"vary", 59},
    {"via", 60},
    {"www-authenticate", 61},

};

bool set_http2_frame_streamid(std::string &hh_data, unsigned int streamid)
{
    if (hh_data.size() < 9)
    {
        return false;
    }
    hh_data[8] = streamid & 0xFF;
    streamid   = streamid >> 8;
    hh_data[7] = streamid & 0xFF;
    streamid   = streamid >> 8;
    hh_data[6] = streamid & 0xFF;
    streamid   = streamid >> 8;
    hh_data[5] = streamid & 0x7F;

    return true;
}
bool set_http2_headers_size(std::string &hh_data, unsigned int sizenum)
{
    if (hh_data.size() < 3)
    {
        return false;
    }
    hh_data[2] = sizenum & 0xFF;
    sizenum    = sizenum >> 8;
    hh_data[1] = sizenum & 0xFF;
    sizenum    = sizenum >> 8;
    hh_data[0] = sizenum & 0xFF;

    return true;
}

bool set_http2_headers_flag(std::string &hh_data, unsigned char flag)
{
    if (hh_data.size() < 9)
    {
        return false;
    }
    hh_data[4] = flag;
    return true;
}
// 与 set_http2_headers_flag() 的赋值语义相对：只把指定位并进首帧 flags，其余位保持原样。
// 头块切成 HEADERS + CONTINUATION 后，首帧不再带 END_HEADERS，用赋值补 END_STREAM 会把
// 「头块未结束」这个状态抹掉。
bool or_http2_headers_flag(std::string &hh_data, unsigned char flag)
{
    if (hh_data.size() < 9)
    {
        return false;
    }
    hh_data[4] = (unsigned char)(((unsigned char)hh_data[4]) | flag);
    return true;
}
// 把一个已成型的首帧（9 字节帧头 + 完整 HPACK 块）原地切成 HEADERS + CONTINUATION 链。
// 头块长度不超过 cap 时原样返回 false（单帧，最常见的响应走这条）。
// 切出来的链里：每片写自己的 24 位长度；END_HEADERS 只落末片；END_STREAM 只落首片
// （CONTINUATION 的合法 flag 只有 END_HEADERS）；中间片 flags 为 0。
bool http2_split_headers(std::string &hh_data, unsigned int cap)
{
    if (hh_data.size() < 9 || cap == 0)
    {
        return false;
    }
    unsigned int block_size = (unsigned int)hh_data.size() - 9;
    if (block_size <= cap)
    {
        return false;
    }
    if (cap > 0xFFFFFF)
    {
        cap = 0xFFFFFF;
    }
    unsigned char first_flags = (unsigned char)(((unsigned char)hh_data[4]) & (unsigned char)(~HTTP2_HEADER_END_HEADERS));
    unsigned char id_bytes[4];
    for (int i = 0; i < 4; i++)
    {
        id_bytes[i] = (unsigned char)hh_data[5 + i];
    }
    std::string block = hh_data.substr(9);
    hh_data.clear();
    hh_data.reserve(block_size + ((block_size / cap) + 2) * 9);
    bool first = true;
    for (unsigned int off = 0; off < block_size; off += cap)
    {
        unsigned int n = block_size - off;
        if (n > cap)
        {
            n = cap;
        }
        bool last = (off + n >= block_size);
        unsigned char flags = last ? HTTP2_HEADER_END_HEADERS : 0;
        if (first)
        {
            flags = (unsigned char)(flags | first_flags);
        }
        hh_data.push_back((char)((n >> 16) & 0xFF));
        hh_data.push_back((char)((n >> 8) & 0xFF));
        hh_data.push_back((char)(n & 0xFF));
        // 0x01 HEADERS / 0x09 CONTINUATION
        hh_data.push_back(first ? 0x01 : 0x09);
        hh_data.push_back((char)flags);
        for (int i = 0; i < 4; i++)
        {
            hh_data.push_back((char)id_bytes[i]);
        }
        hh_data.append(block, off, n);
        first = false;
    }
    return true;
}
bool set_http2_headers_static(unsigned char *hh_data, unsigned char hh_code)
{
    *hh_data = hh_code | 0x80;
    return true;
}
bool make_http2_headers_static(std::string &hh_data, unsigned int hh_code)
{
    // 本函数拿不到 hpack_enc_table，所以每个状态码都只能用两端都不碰动态表的表示：
    // 静态表里有的（200/204/206/304/400/404/500）发 Indexed，其余发 0x08 Literal without
    // Indexing。发 0x48（with Incremental Indexing）会让对端插一条而本端 hpack_enc 不插，
    // 同一块里后续任何 Indexed 引用从此错位一格，Firefox 报 NS_ERROR_CORRUPTED_CONTENT。
    switch (hh_code)
    {
    case 200:
        hh_data.push_back((char)0x88);
        break;
    case 204:
        hh_data.push_back((char)0x89);
        break;
    case 206:
        hh_data.push_back((char)0x8A);
        break;
    case 302:
        hh_data.push_back(0x08);
        hh_data.push_back(0x03);
        hh_data.push_back(0x33);
        hh_data.push_back(0x30);
        hh_data.push_back(0x32);
        break;
    case 304:
        hh_data.push_back((char)0x8B);
        break;
    case 400:
        hh_data.push_back((char)0x8C);
        break;
    case 403:
        hh_data.push_back(0x08);
        hh_data.push_back(0x03);
        hh_data.push_back(0x34);
        hh_data.push_back(0x30);
        hh_data.push_back(0x33);
        break;
    case 404:
        hh_data.push_back((char)0x8D);
        break;
    case 500:
        hh_data.push_back((char)0x8E);
        break;
    default:
        // 未单列的状态码用 :status 的字面量形式（名字索引 8 + 3 位数字值）如实编码。
        // 这里原先兜底 push 0x88（:status 200）并返回 false，而调用方 httppeer::make_http2_header()
        // 忽略返回值，于是 h2 上凡是这张小表没列出的码（301/401/405/502/503…）都被说成 200。
        if (hh_code >= 100 && hh_code <= 999)
        {
            std::string code_str = std::to_string(hh_code);
            hh_data.push_back(0x08);
            hh_data.push_back(static_cast<char>(code_str.size()));
            hh_data.append(code_str);
            return true;
        }
        hh_data.push_back((char)0x88);
        return false;
    }

    return true;
}
// ---- 旧编码件（item / item2）：本仓库已无调用点，响应头一律走下面的 item3 ----
// 保留实现是为了留下"改前形状"的参考，不要当死代码删掉。
// 2026-11-23 修复：这几个函数原先把首字节写成 `hh_code | 0x40`（Literal with Incremental
// Indexing，RFC 7541 §6.2.1），但签名里没有 hpack_enc_table，本端编码表不插而对端插一条——
// 之后每一个 Indexed 引用都错位一格，整个 HEADERS 块被对端判废。成因与
// make_http2_headers_static() 的 0x48 完全相同（那个只改状态行，这里改字段行）。
// 现在统一发 `0000xxxx` Literal without Indexing（§6.2.2）：拿不到 enc 就不许动动态表，
// 于是 PAOZHU_HPACK_ENCODER_DYNAMIC_TABLE 开与关两种档位都安全。
// 单参的 make_http2_headers_item(hh_data, hh_code) 是例外：它发 Indexed（静态表槽引用），
// 本来两端都不碰动态表，原样保留。
// hh_code 是静态表槽号 1..61；≥16 的名字索引要两字节 varint，交给 hpack_put_uint 的 4-bit
// 前缀分支——原来手写的 `hh_code > 63` 判的是 6-bit 前缀的 cap，换成 without-indexing 后是错的。
static void hpack_put_uint(std::string &out, unsigned int prefix_bits, unsigned char first, unsigned int value);

bool make_http2_headers_item(std::string &hh_data, unsigned char hh_code)
{
    hh_data.push_back(hh_code | 0x80);
    return true;
}

bool make_http2_headers_item(std::string &hh_data, unsigned char hh_code, const std::string &value)
{
    std::string en_value;
    http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
    hpack_put_uint(hh_data, 4, 0x00, hh_code);
    if (en_value.size() > 0x7F)
    {
        unsigned int totalsize = static_cast<unsigned int>(en_value.size() - 0x7F);
        hh_data.push_back((char)0xFF);
        if (totalsize > 127)
        {
            unsigned char temp    = totalsize / 128;
            unsigned char tempsub = totalsize % 128;
            hh_data.push_back((unsigned char)tempsub | 0x80);
            hh_data.push_back((unsigned char)temp);
        }
        else
        {
            hh_data.push_back((unsigned char)totalsize);
        }
    }
    else
    {
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
    }
    hh_data.append((char *)&en_value[0], en_value.size());
    return true;
}

bool make_http2_headers_item2(std::string &hh_data, const std::string &key, const std::string &value)
{

    hpack_put_uint(hh_data, 4, 0x00, 0);// 索引 0 = 名字随字面量给出

    std::string en_value;
    http_huffman_encode((unsigned char *)&key[0], key.size(), en_value);

    if (en_value.size() > 0x7F)
    {
        unsigned int totalsize = static_cast<unsigned int>(en_value.size() - 0x7F);
        hh_data.push_back((char)0xFF);
        if (totalsize > 127)
        {
            unsigned char temp    = totalsize / 128;
            unsigned char tempsub = totalsize % 128;
            hh_data.push_back((unsigned char)tempsub | 0x80);
            hh_data.push_back((unsigned char)temp);
        }
        else
        {
            hh_data.push_back((unsigned char)totalsize);
        }
    }
    else
    {
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
    }

    hh_data.append((char *)&en_value[0], en_value.size());

    en_value.clear();
    http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

    if (en_value.size() > 0x7F)
    {
        unsigned int totalsize = static_cast<unsigned int>(en_value.size() - 0x7F);
        hh_data.push_back((char)0xFF);
        if (totalsize > 127)
        {
            unsigned char temp    = totalsize / 128;
            unsigned char tempsub = totalsize % 128;
            hh_data.push_back((unsigned char)tempsub | 0x80);
            hh_data.push_back((unsigned char)temp);
        }
        else
        {
            hh_data.push_back((unsigned char)totalsize);
        }
    }
    else
    {
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
    }
    hh_data.append((char *)&en_value[0], en_value.size());
    return true;
}

bool make_http2_headers_item2(std::string &hh_data, unsigned char hh_code, const std::string &value)
{
    std::string en_value;
    http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

    hpack_put_uint(hh_data, 4, 0x00, hh_code);

    if (en_value.size() > 0x7F)
    {
        unsigned int totalsize = static_cast<unsigned int>(en_value.size() - 0x7F);
        hh_data.push_back((char)0xFF);
        if (totalsize > 127)
        {
            unsigned char temp    = totalsize / 128;
            unsigned char tempsub = totalsize % 128;
            hh_data.push_back((unsigned char)tempsub | 0x80);
            hh_data.push_back((unsigned char)temp);
        }
        else
        {
            hh_data.push_back((unsigned char)totalsize);
        }
    }
    else
    {
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
    }
    hh_data.append((char *)&en_value[0], en_value.size());
    return true;
}

bool make_http2_headers_item2(std::string &hh_data, unsigned char hh_code, unsigned long long num)
{

    hpack_put_uint(hh_data, 4, 0x00, hh_code);

    std::string value, en_value;
    value = std::to_string(num);
    http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
    hh_data.push_back((unsigned char)en_value.size() | 0x80);
    hh_data.append((char *)&en_value[0], en_value.size());
    return true;
}

// ---- HPACK 整数 / 字面量编码 helper（RFC 7541 §5.1 / §6.2）----
// N-bit 前缀整数：first 是已含类型位的首字节基底，value 填满前缀后走 varint 尾巴。
// 7-bit + 0x80 = Indexed / 字符串长度；6-bit + 0x40 = Incremental Indexing 的 name 索引；
// 4-bit + 0x00 / 0x10 = without / never indexed；5-bit + 0x20 = Size Update。
static void hpack_put_uint(std::string &out, unsigned int prefix_bits, unsigned char first, unsigned int value)
{
    const unsigned int cap = (1u << prefix_bits) - 1;
    if (value < cap)
    {
        out.push_back(static_cast<char>(first | value));
        return;
    }
    out.push_back(static_cast<char>(first | cap));
    value -= cap;
    while (value >= 128)
    {
        out.push_back(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<char>(value));
}

// 字符串字面量：Huffman 标志（恒编码）+ 7-bit 前缀长度 + 编码字节。
static void hpack_put_string(std::string &out, const std::string &s)
{
    std::string en;
    if (!s.empty())
        http_huffman_encode((unsigned char *)&s[0], static_cast<unsigned int>(s.size()), en);
    hpack_put_uint(out, 7, 0x80, static_cast<unsigned int>(en.size()));
    out.append(en);
}

// 静态表查找：name+value 全匹配返回槽号；仅 name 匹配经 out_name_index 返回。
// 槽 0 是 ":empty" 哨兵，不参与；空 value 不算全匹配（静态表空值条目只做 name 引用）。
static unsigned int hpack_static_find(const std::string &name, const std::string &value, unsigned int &out_name_index)
{
    out_name_index = 0;
    for (unsigned int i = 1; i <= 61; ++i)
    {
        if (http2_header_static_table[i].key == name)
        {
            if (!value.empty() && http2_header_static_table[i].value == value)
                return i;
            if (out_name_index == 0)
                out_name_index = i;
        }
    }
    return 0;
}

// 查表 + 插表编码。enc==nullptr 保持原 Literal without Indexing（流式路径 / 宏回退）。
// known_name_index 非 0 时 name 必须与该静态槽同名（field index 重载的调用契约）。
static void hpack_encode_field(std::string &hh_data, const std::string &name, const std::string &value,
                               unsigned int known_name_index, hpack_enc_table *enc)
{
    if (enc == nullptr)
    {
        hpack_put_uint(hh_data, 4, 0x00, known_name_index);
        if (known_name_index == 0)
            hpack_put_string(hh_data, name);
        hpack_put_string(hh_data, value);
        return;
    }

    unsigned int static_name_ix = 0;
    unsigned int static_hit     = hpack_static_find(name, value, static_name_ix);
    if (static_hit > 0)
    {
        hpack_put_uint(hh_data, 7, 0x80, static_hit);
        return;
    }
    // 对端表上限为 0：退化为 without-indexing（仍可用静态 name 索引），不插表，
    // 免得对端每条都做一遍 insert+evict 的空转。
    if (enc->max_size == 0)
    {
        unsigned int name_ix = known_name_index != 0 ? known_name_index : static_name_ix;
        hpack_put_uint(hh_data, 4, 0x00, name_ix);
        if (name_ix == 0)
            hpack_put_string(hh_data, name);
        hpack_put_string(hh_data, value);
        return;
    }
    unsigned int dyn_name_ix = 0;
    unsigned int hit         = enc->find(name, value, dyn_name_ix);
    if (hit > 0)
    {
        hpack_put_uint(hh_data, 7, 0x80, hit);
        return;
    }

    unsigned int name_ix = known_name_index != 0 ? known_name_index
                           : (static_name_ix != 0 ? static_name_ix : dyn_name_ix);
    // Literal with Incremental Indexing（6-bit 前缀）+ 插入动态表。
    // max_size==0 时 insert 逐空全表，表示本身仍合法，两端同步为空。
    hpack_put_uint(hh_data, 6, 0x40, name_ix);
    if (name_ix == 0)
        hpack_put_string(hh_data, name);
    hpack_put_string(hh_data, value);
    enc->insert(name, value);
}

bool make_http2_headers_item3(std::string &hh_data, const std::string &key, const std::string &value, hpack_enc_table *enc)
{
    hpack_encode_field(hh_data, key, value, 0, enc);
    return true;
}

bool make_http2_headers_item3(std::string &hh_data, unsigned char hh_code, const std::string &value, hpack_enc_table *enc)
{
    if (hh_code >= 1 && hh_code <= 61)
        hpack_encode_field(hh_data, http2_header_static_table[hh_code].key, value, hh_code, enc);
    else
        hpack_encode_field(hh_data, std::string(), value, 0, enc);
    return true;
}

bool make_http2_headers_item3(std::string &hh_data, unsigned char hh_code, unsigned long long num, hpack_enc_table *enc)
{
    return make_http2_headers_item3(hh_data, hh_code, std::to_string(num), enc);
}

// Literal never indexed（0001xxxx，RFC 7541 §6.2.3）：敏感值（set-cookie）专用，
// 两端都不插表，避免 CRIME 类压缩侧信道，也不占动态表额度。
bool make_http2_headers_item4(std::string &hh_data, unsigned char hh_code, const std::string &value)
{
    hpack_put_uint(hh_data, 4, 0x10, hh_code);
    hpack_put_string(hh_data, value);
    return true;
}

bool make_http2_headers_item4(std::string &hh_data, unsigned char hh_code, unsigned long long num)
{
    return make_http2_headers_item4(hh_data, hh_code, std::to_string(num));
}

bool make_http2_headers_item4(std::string &hh_data, const std::string &key, const std::string &value)
{
    hh_data.push_back(0x10);
    hpack_put_string(hh_data, key);
    hpack_put_string(hh_data, value);
    return true;
}
}// namespace http
