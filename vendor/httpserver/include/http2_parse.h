#ifndef __HTTP2_PARSE_H
#define __HTTP2_PARSE_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <filesystem>
#include <atomic>
#include <string_view>

#include <vector>
#include <cmath>
#include <cctype>
#include <thread>
#include <chrono>
#include <cstring>

#include "cost_define.h"
#include "http2_frame.h"
#include "client_session.h"
#include "httppeer.h"

#ifdef WIN32
#define stat _stat
#endif

namespace http
{

union HTTP2_HEADER_FLAG
{
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char END_STREAM : 1;
        unsigned char e3 : 1;
        unsigned char END_HEADERS : 1;
        unsigned char PADDED : 1;
        unsigned char e4 : 1;
        unsigned char PRIORITY : 1;
    };
    char value;
};

//0x01
struct HTTP2_HEADER_FRAME_T
{
    unsigned int length      = 0;
    unsigned char frame_type = 0;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char END_STREAM : 1;
        unsigned char e3 : 1;
        unsigned char END_HEADERS : 1;
        unsigned char PADDED : 1;
        unsigned char e4 : 1;
        unsigned char PRIORITY : 1;

    } flags;
    unsigned int stream_id         = 0;
    unsigned char padded_length    = 0;
    unsigned int stream_dependency = 0;
    unsigned char priority_weight  = 0;
    std::string content;
};

//0x00
struct HTTP2_DATA_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char e5 : 1;
        unsigned char PADDED : 1;
        unsigned char e6 : 1;
        unsigned char END_STREAM : 1;
    } flags;
    unsigned int stream_id;
    unsigned char padded_length;
    std::string content;
};

//0x03
struct HTTP2_RST_STREAM_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int error_code;
};

//0x04
struct HTTP2_SETTING_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char e5 : 1;
        unsigned char PADDED : 1;
        unsigned char e6 : 1;
        unsigned char END_STREAM : 1;
    } flags;
    unsigned int stream_id;
    std::string content;
};

//0x05
struct HTTP2_PUSH_PROMISE_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    struct
    {
        unsigned char e1 : 1;
        unsigned char e2 : 1;
        unsigned char e3 : 1;
        unsigned char e4 : 1;
        unsigned char PADDED : 1;
        unsigned char END_HEADERS : 1;
        unsigned char e6 : 1;
        unsigned char e7 : 1;
    } flags;
    unsigned int stream_id;
    unsigned char padded_length;
    std::string content;
};
//0x06
struct HTTP2_PING_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned char data;
};
//0x07
struct HTTP2_GOWAY_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int last_stream_id;
    unsigned int error_code;
    std::string content;
};

//0x08
struct HTTP2_WINDOW_UPDATE_FRAME_T
{
    unsigned int length;
    unsigned char frame_type;
    unsigned char flags;
    unsigned int stream_id;
    unsigned int window_size_increment;
};

//data pack
struct HTTP2_PACK_DATA_T
{
    unsigned char frame_type = 0;
    unsigned char flags      = 0;
    unsigned int length      = 0;
    unsigned int stream_id   = 0;
    std::string subpad;
    std::string payload;
};

struct HTTP2_POST_DATA_T
{
    bool isfile                   = false;
    bool end_stream               = false;
    unsigned char posttype        = 0;
    unsigned char mach_pos        = 0;
    unsigned int stream_id        = 0;
    unsigned int field_offset     = 0;
    unsigned long long exp_length = 0;//需要的长度
    unsigned long long cur_length = 0;//当前长度
    std::string_view content;
    std::string chartset;
    std::string mimetype;
    std::string boundary;
    std::string pre_content;//为匹配剩余的字节
    std::string filename;
    std::string field_name;//post name;
    std::string field_item;//post value
    std::string temp_filename;
    std::unique_ptr<std::FILE, int (*)(FILE *)> fp;
    std::shared_ptr<httppeer> peer;

    HTTP2_POST_DATA_T() : fp(nullptr, std::fclose) {};
};

class http2parse
{

  public:
    http2parse()
    {
    }

    ~http2parse()
    {
    }
    static std::string str_tolower(std::string_view str)
    {
        std::string result;
        result.resize(str.size());
        std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return result;
    }
    void setsession(std::shared_ptr<client_session>);
    void readheaders(const HTTP2_PACK_DATA_T &temp_pack_data);
    // void setstaticheader(const unsigned char, unsigned int, unsigned int);
    void readsetting(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readpriority(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readpostdata(const HTTP2_PACK_DATA_T &temp_pack_data);
    void reset_uploadfile(HTTP2_POST_DATA_T &temp_pack_data);
    void post_data_process(HTTP2_POST_DATA_T &temp_pack_data, unsigned char islast_pack);
    void multipart_post_file_field(HTTP2_POST_DATA_T &temp_post_data);
    void post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value, std::shared_ptr<httppeer> steam_httppeer);
    void post_www_form_urlencoded(HTTP2_POST_DATA_T &temp_post_data);
    void post_multipart_formdata(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack);
    void post_multipart_itemcontent_append(HTTP2_POST_DATA_T &temp_post_data);
    void post_multipart_itemcontent(HTTP2_POST_DATA_T &temp_post_data, bool);
    void readrawfileformdata(HTTP2_POST_DATA_T &temp_pack_data, unsigned char islast_pack);
    void readgoaway(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readping(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readrst_stream(const HTTP2_PACK_DATA_T &temp_pack_data);
    void readwinupdate(const HTTP2_PACK_DATA_T &temp_pack_data);

    void headertype1(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype2(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype3(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void headertype4(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);
    void dynamic_table_size_update(unsigned char c, std::string_view buffer, unsigned int &begin, std::shared_ptr<httppeer>);

  public:
    void headers_parse(const HTTP2_HEADER_FRAME_T &, std::shared_ptr<httppeer>);
    // 返回本次真正消费掉的字节数（出错时停在坏帧末尾），调用方据此保留未解析的尾部
    unsigned int process(const unsigned char *buffer, unsigned int buffersize);
    // 置连接级错误（默认走 GOAWAY 断连）
    void set_conn_error(unsigned int code, unsigned int h2 = 0x1);
    // 置流级错误（随 RST_STREAM 重置该流，保留连接）；sid 为出错流 id
    void set_stream_error(unsigned int code, unsigned int sid, unsigned int h2 = 0x1);
    // HPACK（RFC 7541）解码层失败：内部编码坏了、索引越界、整数/字符串长度解不出来，
    // 错误码必须是 COMPRESSION_ERROR(0x9)；头字段本身的违例（§8.1.2）不走这里，仍是 PROTOCOL_ERROR。
    void hpack_fail(unsigned int code)
    {
        error         = code;
        h2_error_code = 0x9;
    }
    // RST_STREAM 后回收该流的全部状态，避免 http_data/http_post_data/窗口记账随请求数无限增长，
    // 也防止流还在 stream_list 队列时被 spawn 成孤儿。
    void cleanup_stream(unsigned int sid);
    // 复位错误状态到默认值（error=0、连接级、无流、PROTOCOL_ERROR）。
    // 流级错误处理完一轮后调用，便于主循环继续复用本解析器处理后续帧。
    void clear_error();
    void data_process();
    bool header_host_process(const std::string &header_value, std::shared_ptr<httppeer>);
    void getacceptencoding(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void header_process(std::string header_name, std::string header_value, int, std::shared_ptr<httppeer>);
    void cookie_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getacceptlanguage(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void range_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void path_process(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getaccept(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getcontenttype(const std::string &, const std::string &, std::shared_ptr<httppeer>);
    void getifnonematch(const std::string &, const std::string &, std::shared_ptr<httppeer>);

    void callposttype(const std::string &header_value, std::shared_ptr<httppeer>, HTTP2_POST_DATA_T &pd);
    void clsoesend();
    void procssparamter(std::string_view buffer_key, std::string_view buffer_value, std::shared_ptr<httppeer> steam_httppeer);
    void process_pack();
    void read_pack_data(const unsigned char *buffer, unsigned int buffersize);

  public:
    unsigned int error      = 0;
    // 连接级错误对应的 HTTP/2 错误码，随 GOAWAY 带出；默认 PROTOCOL_ERROR(0x1)。
    // 各类校验在置 error 时同时设置它（如 FRAME_SIZE_ERROR=0x6），否则 GOAWAY 会发成 NO_ERROR(0)。
    unsigned int h2_error_code = 0x1;
    // 流级错误归属：出错流 id（连接级为 0）+ 是否连接级。
    // 默认 error_is_conn=true，即所有未显式改用 set_stream_error() 的 error 站点仍走 GOAWAY 断连，
    // 保证「没改到的站点」行为不变，只有逐站改写的请求级校验才会降级为 RST_STREAM。
    unsigned int error_stream_id = 0;
    bool        error_is_conn    = true;
    unsigned int readoffset = 0;

    // // 头部块是否进行中：某流 HEADERS/CONTINUATION 尚未 END_HEADERS 时为真。
    // // 用于 RFC 9113 §6.10：进行中收到非 CONTINUATION 帧即连接级 PROTOCOL_ERROR。
    // // 用单标志替代每帧遍历 http2_header_recvs，避免占用正常分发路径。
    // bool header_block_pending = false;

    unsigned int steam_count = 0;
    unsigned int isfinsish   = 0;

    // Rapid Reset(CVE-2023-44487) protection: count RST_STREAM frames per connection.
    // When it exceeds the threshold the connection is closed via GOAWAY.
    unsigned char rst_stream_count = 0;

    struct http2_setting_t setting_data;

    //std::vector<std::pair<std::string, std::string>> header_lists;
    std::list<std::pair<std::string, std::string>> dynamic_lists;
    unsigned int dynamic_table_max_size = 4096;
    // struct http2_goaway_t goaway_data;
    // unsigned long long content_length;
    std::atomic_bool task_in               = false;
    // 本连接的「发送侧可能需要重新评估」标记：由读协程在处理帧时置位，
    // 在 http2 读循环末尾消费 —— 消费点会调 httpserver::requeue_parked 回灌挂起的
    // 发送对象，再唤醒读协程自己。改名前的名字 need_wakeup_send_data 会让人以为
    // 它直接唤醒发送线程，其实它只是同一连接上的一条位信号。
    std::atomic_bool need_wakeup_send_threads = false;
    std::shared_ptr<client_session> peer_session;

    // ---- 本端接收窗口记账（RFC 9113 §6.9）----
    // 单靠一个全局计数器不行：HEADERS 时无条件重置、DATA 扣减不分流、补窗口
    // 却整笔补给「恰好让全局计数越线的那一条流」。两个 POST 并发时，后到的 HEADERS
    // 会把前一条流已消费的额度抹掉，导致前一条流再也不会被补窗口，对端发完自身
    // 窗口后停发，服务端 http_post_data 永远等不到 last_pack（静默挂死）。
    // 记账拆成「连接级 + 每流」两套独立：
    //   conn_recv_window_num  连接级剩余额度，目标是 CONST_HTTP2_LOCAL_INITIAL_WINDOW
    //   stream_recv_window    每流剩余额度，懒初始化为同一个目标水位
    unsigned int conn_recv_window_num = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    // 连接级窗口是否已从 RFC 初值 65535 抬到目标水位。连接级窗口只能用
    // stream id = 0 的 WINDOW_UPDATE 抬升，且必须「只做一次」：每来一个请求就重发
    // 会让对端连接窗口顶穿 2^31-1，对端必须按 FLOW_CONTROL_ERROR 断连。
    bool conn_recv_window_raised = false;
    std::map<unsigned int, unsigned int> stream_recv_window;

    // 对端「累计授予」的连接级发送窗口额度，只在本连接收到首个 SETTINGS 时落一次
    // RFC 初值。这个判断不能借「window_update_num 恰好等于 0」：该字段是累计值、构造即
    // 65535 且只增不减，而 WebSocket 与原生 socket 路径还把这两个字段当 URL 数字
    // 参数清零使用，两条不相干的路径共用同一格会把初值判断带偏。
    bool conn_send_window_inited = false;

    // 客户端已使用过的最大流 id。RFC 9113 §5.1.1 要求客户端新开的流 id 必须
    // 严格递增（客户端只能使用奇数流 id），否则必须按协议错误处理。
    unsigned int max_client_stream_id = 0;
    // 落在「未知 / 已结束」流上的 WINDOW_UPDATE 被丢弃的次数。远程对端可以随意给
    // 任意流 id 发 WINDOW_UPDATE，本端没有流状态机，只能拿「本连接见过的最大客户端
    // 流 id + 该流是否还活着」近似判定；计数用来排查「窗口给了却不发」。
    unsigned int winupdate_dropped = 0;
    std::map<unsigned int, std::shared_ptr<httppeer>> http_data;
    std::map<unsigned int, std::weak_ptr<httppeer>> http_data_weak;
    std::map<unsigned int, HTTP2_POST_DATA_T> http_post_data;

    //new data
    std::map<unsigned int, HTTP2_HEADER_FRAME_T> http2_header_recvs;
    std::map<unsigned int, HTTP2_DATA_FRAME_T> http2_data_recvs;
    struct HTTP2_PACK_DATA_T pack_data;

    std::queue<unsigned int> stream_list;

    std::mutex http2loop_mutex;
    std::atomic<bool> http2_loop_in = false;
};
}// namespace http
#endif
