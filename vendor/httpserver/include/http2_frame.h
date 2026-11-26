#ifndef __HTTP2_FRAME_H
#define __HTTP2_FRAME_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <deque>
#include "http_header.h"

namespace http
{
typedef struct
{
    std::string key;
    std::string value;
} http2_header_static_table_t;

#define PER_DATA_BLOCK_SIZE 10240

#define HTTP2_SETTINGS_HEADER_TABLE_SIZE 0x01
#define HTTP2_SETTINGS_ENABLE_PUSH 0x02
#define HTTP2_SETTINGS_MAX_CONCURRENT_STREAM 0x03
#define HTTP2_SETTINGS_INITIAL_WINDOW_SIZE 0x04
#define HTTP2_SETTINGS_MAX_FRAME_SIZE 0x05
#define HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE 0x06

#define HTTP2_HEADER_END_STREAM 0x01
#define HTTP2_HEADER_END_HEADERS 0x04
#define HTTP2_HEADER_PADDED 0x08
#define HTTP2_HEADER_PRIORITY 0x20

#define HTTP2_DATA_END_STREAM 0x01
#define HTTP2_DATA_PADDED 0x08

//

// 对端 SETTINGS 帧的镜像。真正驱动本端行为的是 0x04 initial_window_size 和
// 0x05 max_frame_size（校验后另存到连接上）；0x01/0x02/0x03/0x06 目前只记录不读取——
// 本端解码表上限、入站并发流上限、入站头块上限都用本端自己的常量，
// 刻意不拿对端广告值当本端限额。要接 push 或按广告值收窄本端上限时，
// 得显式读这几个字段，不能假设默认值在生效。
struct http2_setting_t
{

    unsigned int header_table_size       = 4096; // 0x01
    unsigned int enable_push             = 0;    // 0x02
    unsigned int max_concurrent_streams  = 128;  // 0x03
    unsigned int initial_window_size     = 65535;// 0x04
    unsigned int max_frame_size          = 16384;// 0x05
    unsigned int max_heaer_list_size     = 0;    // 0x06
    unsigned int accept_cache_digest     = 0;    // 0x07
    unsigned int enable_connect_protocol = 0;    // 0x08
};

struct http2_priority_t
{

    unsigned int depend_stream_id;
    unsigned char weight;
};

struct http2_header_t
{
    unsigned int block_length;
    unsigned int stream_id;
    unsigned int depend_stream_id;
    unsigned char flag;
    unsigned char stream_e;
    unsigned char stream_d;
    unsigned char pad_length;
    unsigned char weight;

    std::vector<unsigned char> data;
};

// struct http2_window_update_t{
//      std::map<unsigned int,unsigned char> data;
// };

struct http2_goaway_t
{

    unsigned int last_stream_id;
    unsigned int error_code;
    std::vector<unsigned char> data;
};

struct http2_data_t
{
    http2_data_t() : uprawfile(nullptr, std::fclose){};
    bool isbegin                = false;
    bool isend                  = false;
    bool endstream              = false;
    bool endheader              = false;
    bool padded                 = false;
    bool priority               = false;
    unsigned char posttype      = 0;
    unsigned char postfieldtype = 0;
    unsigned char changetype    = 0;
    unsigned char pad_length    = 0;
    unsigned char weight        = 0;
    unsigned int stream_id      = 0;
    unsigned int match_offset   = 0;
    unsigned int length         = 0;// bloack data length;

    unsigned long long curnum         = 0;// now block length
    unsigned long long content_length = 0;// post length

    unsigned int get_param_count  = 0;
    unsigned int post_param_count = 0;

    //std::FILE *uprawfile = NULL;
    std::unique_ptr<std::FILE, int (*)(FILE *)> uprawfile;

    std::string boundary;
    std::string fieldname;
    std::string buffer_key;
    std::string buffer_value;
    std::string field_value;
    std::list<unsigned int> priority_lists;
    //struct uploadfile_t upfile;
    std::unique_ptr<uploadfile_t> upfile;

    void reset()
    {
        buffer_key.clear();
        buffer_value.clear();
        buffer_value.shrink_to_fit();
        field_value.clear();
        boundary.clear();
        fieldname.clear();
        priority_lists.clear();
        upfile.reset(nullptr);
        uprawfile.reset(nullptr);
        match_offset   = 0;
        curnum         = 0;
        content_length = 0;
        get_param_count  = 0;
        post_param_count = 0;
    }
};

// header code
extern std::map<std::string, unsigned char> http2_header_codes_table;

#define HTTP2_CODE_authority 1
#define HTTP2_CODE_GET 2
#define HTTP2_CODE_POST 3
#define HTTP2_CODE_path 4
#define HTTP2_CODE_index 5
#define HTTP2_CODE_http 6
#define HTTP2_CODE_https 7
#define HTTP2_CODE_200 8
#define HTTP2_CODE_204 9
#define HTTP2_CODE_206 10
#define HTTP2_CODE_304 11
#define HTTP2_CODE_400 12
#define HTTP2_CODE_404 13
#define HTTP2_CODE_500 14
#define HTTP2_CODE_accept_charset 15
#define HTTP2_CODE_accept_encoding 16
#define HTTP2_CODE_accept_language 17
#define HTTP2_CODE_accept_ranges 18
#define HTTP2_CODE_accept 19
#define HTTP2_CODE_access_control_allow_origin 20
#define HTTP2_CODE_age 21
#define HTTP2_CODE_allow 22
#define HTTP2_CODE_authorization 23
#define HTTP2_CODE_cache_control 24
#define HTTP2_CODE_content_disposition 25
#define HTTP2_CODE_content_encoding 26
#define HTTP2_CODE_content_language 27
#define HTTP2_CODE_content_length 28
#define HTTP2_CODE_content_location 29
#define HTTP2_CODE_content_range 30
#define HTTP2_CODE_content_type 31
#define HTTP2_CODE_cookie 32
#define HTTP2_CODE_date 33
#define HTTP2_CODE_etag 34
#define HTTP2_CODE_expect 35
#define HTTP2_CODE_expires 36
#define HTTP2_CODE_from 37
#define HTTP2_CODE_host 38
#define HTTP2_CODE_if_match 39
#define HTTP2_CODE_if_modified_since 40
#define HTTP2_CODE_if_none_match 41
#define HTTP2_CODE_if_range 42
#define HTTP2_CODE_if_unmodified_since 43
#define HTTP2_CODE_last_modified 44
#define HTTP2_CODE_link 45
#define HTTP2_CODE_location 46
#define HTTP2_CODE_max_forwards 47
#define HTTP2_CODE_proxy_authenticate 48
#define HTTP2_CODE_proxy_authorization 49
#define HTTP2_CODE_range 50
#define HTTP2_CODE_referer 51
#define HTTP2_CODE_refresh 52
#define HTTP2_CODE_retry_after 53
#define HTTP2_CODE_server 54
#define HTTP2_CODE_set_cookie 55
#define HTTP2_CODE_strict_transport_security 56
#define HTTP2_CODE_transfer_encoding 57
#define HTTP2_CODE_user_agent 58
#define HTTP2_CODE_vary 59
#define HTTP2_CODE_via 60
#define HTTP2_CODE_www_authenticate 61

static http2_header_static_table_t http2_header_static_table[] = {
    {":empty", ""},
    {":authority", ""},
    {":method", "GET"},
    {":method", "POST"},
    {":path", "/"},
    {":path", "/index.html"},
    {":scheme", "http"},
    {":scheme", "https"},
    {":status", "200"},
    {":status", "204"},
    {":status", "206"},
    {":status", "304"},
    {":status", "400"},
    {":status", "404"},
    {":status", "500"},
    {"accept-charset", ""},
    {"accept-encoding", "gzip, deflate"},
    {"accept-language", ""},
    {"accept-ranges", ""},
    {"accept", ""},
    {"access-control-allow-origin", ""},
    {"age", ""},
    {"allow", ""},
    {"authorization", ""},
    {"cache-control", ""},
    {"content-disposition", ""},
    {"content-encoding", ""},
    {"content-language", ""},
    {"content-length", ""},
    {"content-location", ""},
    {"content-range", ""},
    {"content-type", ""},
    {"cookie", ""},
    {"date", ""},
    {"etag", ""},
    {"expect", ""},
    {"expires", ""},
    {"from", ""},
    {"host", ""},
    {"if-match", ""},
    {"if-modified-since", ""},
    {"if-none-match", ""},
    {"if-range", ""},
    {"if-unmodified-since", ""},
    {"last-modified", ""},
    {"link", ""},
    {"location", ""},
    {"max-forwards", ""},
    {"proxy-authenticate", ""},
    {"proxy-authorization", ""},
    {"range", ""},
    {"referer", ""},
    {"refresh", ""},
    {"retry-after", ""},
    {"server", ""},
    {"set-cookie", ""},
    {"strict-transport-security", ""},
    {"transfer-encoding", ""},
    {"user-agent", ""},
    {"vary", ""},
    {"via", ""},
    {"www-authenticate", ""},
};
// HPACK 编码器侧动态表（RFC 7541 §2.3.2 / §4.1），每连接一份，挂在 client_session 上。
// 布局与解码侧 http2parse::dynamic_lists 完全一致：队头最新、尾部逐出。
// 索引不存条目里：index = 62 + 距队头位置，插入/逐出后全体自动移动（RFC 7541 §2.3.3）。
// 全部读写点都在连接 strand 上（喂入锁内编码 / SETTINGS 解析），不需要锁；
// 但编码必须发生在喂入锁临界区内，保证表变更顺序 = HEADERS 上线顺序。
struct hpack_enc_table
{
    std::deque<std::pair<std::string, std::string>> entries;// front = 最新
    unsigned int size     = 0;    // Σ(name.size + value.size + 32)
    unsigned int max_size = 4096; // 对端 SETTINGS_HEADER_TABLE_SIZE（默认 4096）
    // 上次发信号以来经历过的最紧上限。发信号时若它 < 当前 max_size，要先发它再发
    // 当前值（一个块里允许两个 Size Update）：缩表时编码器已按它逐出过条目，解码器
    // 必须镜像那次逐出——只发合并后的最终值会让解码器留着编码器已丢的条目，永久失步。
    unsigned int pending_size_update = 0;
    bool size_update_pending         = false;

    // 全匹配返回索引（62+位置）；仅 name 匹配经 out_name_index 返回索引；都没有返回 0。
    unsigned int find(const std::string &name, const std::string &value, unsigned int &out_name_index) const
    {
        out_name_index  = 0;
        unsigned int ix = 62;
        for (const auto &e : entries)
        {
            if (e.first == name)
            {
                if (e.second == value)
                    return ix;
                if (out_name_index == 0)
                    out_name_index = ix;
            }
            ++ix;
        }
        return 0;
    }

    void evict()
    {
        while (!entries.empty() && size > max_size)
        {
            const auto &back = entries.back();
            size -= static_cast<unsigned int>(back.first.size() + back.second.size()) + 32;
            entries.pop_back();
        }
    }

    // 超大条目（> max_size）无条件入表，evict 会把整表逐空（RFC 7541 §4.1）——
    // 不许加准入判断：对端认为这条已入表，跳过会让两端表长永久错开。
    void insert(const std::string &name, const std::string &value)
    {
        entries.emplace_front(name, value);
        size += static_cast<unsigned int>(name.size() + value.size()) + 32;
        evict();
    }

    // 对端 SETTINGS_HEADER_TABLE_SIZE 变更时调用（RFC 7541 §4.2）：
    // 上限任何变化（缩小或抬高）都要在下一个头块发 Size Update 信号——解码器的上限
    // 只跟信号走：缩表不信号，对端还按旧上限留着已逐出的条目；抬表不信号，对端按
    // 旧（小）上限逐掉本端新插的条目——两个方向都永久失步。
    void set_max_size(unsigned int new_max)
    {
        if (new_max != max_size)
        {
            if (!size_update_pending)
                pending_size_update = new_max;
            else if (new_max < pending_size_update)
                pending_size_update = new_max;
            max_size            = new_max;
            size_update_pending = true;
            evict();
        }
    }
};

// http2 header make item
bool set_http2_headers_static(unsigned char *hh_data, unsigned char hh_code);
bool make_http2_headers_static(std::string &hh_data, unsigned int hh_code);

// ---- 旧编码件：本仓库无调用点，保留作演变参考，勿删 ----
// 2026-11-23 起它们发 Literal without Indexing（4-bit 前缀 0000），两端动态表都不碰。改前发的是
// Incremental Indexing（6-bit 前缀 0x40）而签名里没有 hpack_enc_table，本端编码表不插、对端却插
// 一条，之后每个 Indexed 引用错位一格，整个 HEADERS 块被对端判废。
// 新代码请用下面带 hpack_enc_table *enc 的 item3（只有它能安全地拿动态表做压缩）；item4 是
// never-indexed。另注：下面 (unsigned int) 与 (const std::string &) 两条声明在 http2_frame.cpp
// 里没有对应定义，调用只会链接失败。
bool make_http2_headers_item(std::string &hh_data, unsigned int num);
bool make_http2_headers_item(std::string &hh_data, const std::string &value);
bool make_http2_headers_item(std::string &hh_data, unsigned char, const std::string &value);

bool make_http2_headers_item2(std::string &hh_data, unsigned char, const std::string &value);
bool make_http2_headers_item2(std::string &hh_data, const std::string &key, const std::string &value);
bool make_http2_headers_item2(std::string &hh_data, unsigned char, unsigned long long num);

bool make_http2_headers_item3(std::string &hh_data, const std::string &key, const std::string &value,
                              hpack_enc_table *enc = nullptr);
bool make_http2_headers_item3(std::string &hh_data, unsigned char, const std::string &value,
                              hpack_enc_table *enc = nullptr);
bool make_http2_headers_item3(std::string &hh_data, unsigned char, unsigned long long num,
                              hpack_enc_table *enc = nullptr);

bool make_http2_headers_item4(std::string &hh_data, const std::string &key, const std::string &value);
bool make_http2_headers_item4(std::string &hh_data, unsigned char, const std::string &value);
bool make_http2_headers_item4(std::string &hh_data, unsigned char, unsigned long long num);

bool set_http2_frame_streamid(std::string &hh_data, unsigned int streamid);
bool set_http2_headers_size(std::string &hh_data, unsigned int sizenum);
bool set_http2_headers_flag(std::string &hh_data, unsigned char flag);
bool or_http2_headers_flag(std::string &hh_data, unsigned char flag);
bool http2_split_headers(std::string &hh_data, unsigned int cap);
}// namespace http
#endif