#ifndef HTTP_PEER_H
#define HTTP_PEER_H

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>

#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>
#include <list>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <string_view>
#include <array>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <chrono>
#include <source_location>

#include "request.h"
#include "client_session.h"

namespace http
{
template <typename T>
concept VALNUM_T = std::is_floating_point_v<T> || std::is_integral_v<T>;

class httppeer : public std::enable_shared_from_this<httppeer>
{
  public:
    unsigned char get_fileinfo();
    void send_files(std::string);
    void status(unsigned int);
    unsigned int get_status();
    void type(const std::string &);
    void length(unsigned long long);
    void scheme(unsigned char);
    std::shared_ptr<httppeer> get_ptr();
    std::string make_http2_header(unsigned char flag_code = 0);
    std::string make_http1_header();
    void set_header(const std::string &, const std::string &);
    std::string get_header(std::string_view);
    // 追加式写 Vary（逗号列表）：Vary 是"多值声明"，覆盖式 set_header 会把别处已经声明的
    // 那几项冲掉（CORS 的 Origin 与业务的 Accept-Language 互相吃掉），合并时按 token 去重
    void add_vary(std::string_view values);
    void get_cookie(const std::string &);

    void flush_out();

    void set_cookie(std::string key,
                    std::string val,
                    long long exptime      = 0,
                    std::string path       = "",
                    std::string domain     = "",
                    bool secure            = false,
                    bool httponly          = true,
                    std::string issamesite = "");
    // 会话 cookie 的 SameSite：站点配了 cors_credentials 且当前是 HTTPS 时给 "None"，其余情况空串（不发该属性）
    std::string session_samesite();
    bool is_ssl();
    std::list<std::string> cookietoheader();
    std::string get_hosturl();
    std::string get_sitepath();
    unsigned long long get_siteid();
    unsigned long long get_groupid();
    std::string get_theme();
    std::string get_themeurl();
    void theme_view(const std::string &a);

    // 路由预查：带点的 URL 先落盘查一次（盘上那个文件还在、没过期就一次表都不查，让位静态文件），
    // 然后两段查表（原样名 + 扩展名剥离再查）。
    // 返回 >=0 是命中的 handler 下标（交给 router::co_resolve 直接进链），
    // <0 是未命中或让位磁盘 —— 命中时 pathinfos/urlpath 已原地换成注册名
    int prefetch_routing();
    void goto_url(const std::string &url, unsigned char second = 0, const std::string &msg = "");
    bool isset_type();

    void parse_session();
    void save_session();
    void clear_session();
    std::string get_session_id();
    void set_session_id(const std::string &a);
    void parse_session_file(const std::string &sessionfile_id);
    void parse_session_memory(const std::string &sessionfile_id);
    void save_session_memory(const std::string &sessionfile);
    void save_session_file(const std::string &sessionfile);

    void view(const std::string &a);
    void view(const std::string &a, obj_val &b);
    std::string fetchview(const std::string &a);
    std::string fetchview(const std::string &a, obj_val &b);
    void send(const std::string &a);
    httppeer &out(std::string);
    httppeer &out(const std::string &a);

    httppeer &operator<<(obj_val &a);
    httppeer &operator<<(std::string &&a);
    httppeer &operator<<(std::string &a);
    httppeer &operator<<(std::string_view a);
    httppeer &operator<<(char a);
    httppeer &operator<<(unsigned char a);
    httppeer &operator<<(const char *a);
    httppeer &operator<<(const std::string &a);
    httppeer &operator<<(unsigned int a);
    httppeer &operator<<(int a);
    httppeer &operator<<(unsigned long long a);
    httppeer &operator<<(unsigned long a);
    httppeer &operator<<(long long a);
    httppeer &operator<<(float a);
    httppeer &operator<<(double a);

    template <typename T>
    httppeer &operator<<(VALNUM_T auto a);

    template <typename T>
    httppeer &operator<<(T a);
    template <typename T>
    httppeer &operator<<(T *a);

    httppeer &get_peer();
    void out_json(obj_val &a);
    void out_json();
    void json_type();

    // 业务手动指定跨域响应头（一般不用，自动判定见 cors_origin_process）：
    //   name      写入 Access-Control-Allow-Origin
    //   header_v  写入 Access-Control-Allow-Headers，留空按 "*" 放行全部请求头
    // 三个头统一走 set_header()，h2 下会自动落到 HPACK 静态表的索引槽
    void cors_domain(const std::string &name, const std::string &header_v = "");
    // 手动回一份预检响应头（Allow-Methods / Max-Age，可选 Allow-Headers）：
    // 自动预检走 httpserver::send_cors_domain()，业务一般无需自己调；
    // header_v 非空时追加 Access-Control-Allow-Headers，即本接口要放行的请求头列表
    // Allow-Methods 的内容不再由代码固定：取自本站点配置项 cors_allow_methods
    //（缺省 POST, GET, OPTIONS, QUERY），与 send_cors_domain() 校验
    // Access-Control-Request-Method 用的是同一份名单，加方法改配置即可
    void cors_method(const std::string &header_v = "");
    // 方法链的压栈/出栈，给业务做任务编排（按图执行）用：pop 出来自己决定下一个跑谁。
    void push_flow(const std::string &);
    void push_front_flow(const std::string &);
    std::string pop_flow();
    unsigned char add_timeloop_task(const std::string &, unsigned int);
    void clear_timeloop_task();
    unsigned int get_timeloop_count();
    void add_timeloop_count(unsigned int a = 1);
    void clsoesend();
    void clear();
    bool find_host_index();
    // 解析到 Origin 请求头时立即调用（http_parse / http2_parse 头名长度 6 的首字符分支）：
    // 按本站点 cors_domain 白名单判断该 Origin 是否放行，命中就把 Access-Control-Allow-Origin
    // 直接设进响应头，不落成员变量；没有 Origin 的请求完全不进 CORS 代码路径。
    // 注：OPTIONS 预检的 Allow-Origin 不由本函数决定，而是 send_cors_domain() 重新按
    //     cors_allow_origin() 输出（预检可能因 Request-Method 校验失败而不带 ACAO）。
    void cors_origin_process(std::string_view request_origin);
    // 判定本体：按 host_index 所指站点的白名单算出 ACAO 并写进响应头，不看 host 是否已解析。
    // 只由 cors_origin_process() 和 h1 的挂起补判调用。
    void cors_origin_allow(std::string_view request_origin);
    unsigned int check_upload_limit();

    // 常规发送（Content-Length）：先发 header，再发 body，最后 end 结束
    // 使用前须先调 peer->length(file_size) 设置 Content-Length
    void send_make_header();
    void send_make_body(std::string_view data);
    void send_make_end();
    asio::awaitable<void> async_send_make_header();
    asio::awaitable<void> async_send_make_body(std::string_view data);
    asio::awaitable<void> async_send_make_end();

    // 流式发送（chunked transfer）：先发 header，再逐块发 body，最后 end 结束
    void send_chunk_header();
    void send_chunk_body(std::string_view data);
    void send_chunk_end();
    asio::awaitable<void> async_send_chunk_header();
    asio::awaitable<void> async_send_chunk_body(std::string_view data);
    asio::awaitable<void> async_send_chunk_end();

    // SSE 流式输出
    asio::awaitable<void> async_make_sse_header();
    asio::awaitable<void> async_make_sse_body(std::string_view data);
    asio::awaitable<void> async_make_sse_end();
    void make_sse_header();
    void make_sse_body(std::string_view data);
    void make_sse_end();

  private:
    std::string make_http2_data(unsigned int sid, std::string_view payload, bool is_end);

  public:
    std::string host;
    std::string url;
    std::string urlpath;
    std::string querystring;

    std::string content_type;
    std::string etag;

    std::string chartset;
    std::string accept_type;
    std::string rawcontent;
    std::map<std::string, std::string> header;
    http::obj_val get;
    http::obj_val post;
    http::obj_val files;
    http::obj_val json;
    http::obj_val val;
    http::obj_val session;
    http::cookie cookie;

    std::vector<std::string> pathinfos;

    bool issendheader = false;
    bool ischunked    = false;
    bool isfinish     = false;
    bool issend       = false;
    // 每流关闭标志：被读协程（连接级解析）置位以中止「已派发到 http2loop 的流」的出站响应，
    // 发送线程按它丢弃该流已入队的响应。改 atomic 是因为读协程写、发送线程读是真正的跨线程访问，
    // 原先靠执行顺序巧合不重叠才没出事；流级 RST_STREAM 让这次写读变成并发。
    std::atomic<bool> isclose{false};
    bool isssl        = false;
    bool keepalive    = true;
    bool isso         = false;
    bool iscors       = false;
    bool isfile       = false;
    // 仅 HTTP/1 用：Origin 早于 Host 到达时挂起判定（值已在 header["origin"] 里），
    // 等 getheaderhost() 解析出 Host、host_index 定下后补判，避免每请求都查一次 header
    bool cors_origin_pending  = false;
    bool ishas_content_length = false;

    unsigned char posttype   = 0;
    unsigned char compress     = 0;
    unsigned int host_index    = 0;
    unsigned int stream_id     = 0;
    unsigned int status_code   = 0;
    // 仅 HTTP/2 用：站点上传限额在头块阶段就判掉这条请求时，这里存要回的状态码（0 = 不拒）。
    // 非 0 即代表「正文一帧都不收」：读循环按它直接派发状态页，解析器按它丢弃后到的 DATA。
    unsigned int reject_status = 0;
    // 这两个数由业务线程（执行间隔任务的池线程）写、由 tick 线程在 clientlooptasks 扫描里读，
    // 两边不在同一把锁下，所以必须是原子的。tick 只读、业务只写自己这条 peer，不要求跨字段一致。
    std::atomic<unsigned int> timeloop_num{0};
    std::atomic<unsigned int> timecount_num{0};
    // 间隔任务的单飞旗：tick 抢到才投递，跑完（所有出口）由池放下。
    // 没有它，一条比自己的间隔还慢的任务会被两个池线程同时跑，而且越攒越多。
    std::atomic<bool> timeloop_inflight{false};
    // 间隔任务要跑的那个 regfun 的名字。它过去借的是 pathinfos[0]，而 pathinfos 是每个请求
    // 由解析器重填的（http_parse.cpp:520、http2_parse.cpp:409），keep-alive 上的下一个请求
    // 就把任务名换成了新 URL 的段，登记因此活不过下一次请求；这里改成任务自己的一份。
    std::string timeloop_taskname;
    unsigned int request_time  = 0;
    //unsigned int time_limit             = 0;
    unsigned long long content_length   = 0;
    unsigned long long sessionfile_time = 0;
    unsigned long long upload_length    = 0;

    //std::atomic_uint time_limit  = 0;
    struct headstate_t state;

    //cookie send_cookie;
    std::list<std::string> send_cookie_lists;
    std::map<std::string, std::string> send_header;
    std::map<unsigned char, std::string> http2_send_header;

    struct stat fileinfo;
    std::string output;
    std::string sitepath;
    std::string sendfilename;
    // 路由预查「先落盘」那一步 stat 过的文件路径：get_fileinfo() 拼出的 sendfilename 与它相同时
    // 直接复用 fileinfo，同一条带点 URL 不在一次请求里 stat 第二遍
    std::string prefetch_statfile;
    // 路由预查（prefetch_routing）已经对当前 urlpath 做过「原样精确」那一次查表并确认未命中时置 true；
    // 剥扩展名那一次在不在其后都做数（!isfile、末段无点、点在最前这三个出口只查了一次就返回 -1，同样带 true）。
    // co_resolve → resolve_idx 据此跳过重复的那次 lookup_exact，省一次 hash 查找；rpc / 同步链没跑预查，
    // 由 co_resolve 在消费时复位成 false，保证它们仍走完整查找。
    bool route_exact_checked = false;
    // 编排栈不常用，所以做成智能指针懒分配：没压过栈的请求，链尾只付一次指针判空
    std::unique_ptr<std::list<std::string>> flow_method;
    unsigned char sendfiletype;
    unsigned char linktype;
    unsigned char method;
    unsigned char httpv;

    std::string server_ip;
    std::string client_ip;
    unsigned int client_port;
    unsigned int server_port;

    std::shared_ptr<client_session> socket_session = nullptr;

    std::list<asio::detail::awaitable_handler<asio::any_io_executor, size_t>> user_code_handler_call;
    std::mutex pop_user_handleer_mutex;
    // 等待滑动窗口
    // std::atomic_bool window_update_bool = false;
    // std::list<std::future<int>> window_update_results;
    // std::promise<int> window_update_promise;

    // ---- RFC 7540 §8.1.2 伪头跨状态校验标记（HTTP/2 头解析完即置 true 后不再写）----
    // Pseudo-header tracking — set during header parsing, never touched after dispatch.
    bool h2_method_seen  = false;  // :method 至少出现过一次
    bool h2_method_dup   = false;  // :method 重复出现过（h2spec 8.1.2.3 #5）
    bool h2_scheme_seen  = false;  // :scheme 至少出现过一次
    bool h2_scheme_dup   = false;  // :scheme 重复出现过（h2spec 8.1.2.3 #6）
    bool h2_path_seen    = false;  // :path 至少出现过一次
    bool h2_path_dup     = false;  // :path 重复出现过（h2spec 8.1.2.3 #7）
    bool h2_regular_seen = false;  // 是否已出现普通头（伪头顺序校验 h2spec 8.1.2.1 #4）
};
// — v6 router —
enum class fn_kind : uint8_t
{
    none = 0,
    sync = 1,
    coro = 2
};

struct reg_methold_mid_t
{
    fn_kind pre_kind = fn_kind::none;
    std::function<std::string(std::shared_ptr<httppeer>)> pre;
    std::function<asio::awaitable<std::string>(std::shared_ptr<httppeer>)> co_pre;
    fn_kind reg_kind = fn_kind::none;
    std::function<std::string(std::shared_ptr<httppeer>)> regfun;
    std::function<asio::awaitable<std::string>(std::shared_ptr<httppeer>)> co_regfun;
    // 注册条目自带的业务掩码：路由不读它，和 flow_method 那三个方法一样留给任务编排（图执行）用
    unsigned int mask_code = 0;
    // 注册点溯源：reg_raw 的尾默认参在调用方行求值，所以这里记的是
    // common/autocontrolmethod.hpp（或手写注册表）里那一行，不是本头文件。
    // file_name() 是静态字面量可以直接存指针；function_name() 的生命周期标准未保证，按值存。
    const char *reg_file  = "";
    unsigned int reg_line = 0;
    std::string reg_fn;
};

struct SiteSlot
{
    std::map<std::string, unsigned int> path_map;// path → handler idx
};

// v6 全局变量
extern std::vector<reg_methold_mid_t> _handlers;
extern std::vector<SiteSlot> _slots;
extern std::map<unsigned int, std::vector<std::string>> _urlpath_map;
extern std::map<std::string, unsigned int> _site_to_slot;
// 注册面记账：site 不在 conf 里被丢弃的条数 / 同名注册保留首次而丢弃的条数
extern unsigned int _route_reg_dropped;
extern unsigned int _route_reg_dup;

// v6 helper（httppeer.cpp 实现）
void router_init_sites();
// 未知 site 返回 -1：注册面据此丢弃，不落全局 slot
int site_to_slot(const std::string &site);
// 放置一条注册并做 site/same-name 门禁，返回该 path 实际生效的 handler idx；整条被丢弃返回 (unsigned)-1
unsigned int reg_add(const std::string &site, const std::string &path, reg_methold_mid_t &&entry);
void reg_urlpath(unsigned int idx, std::vector<std::string> names);
void reg_urlpath(const std::string &site, const std::string &path, std::vector<std::string> names);
void make_404_content(std::shared_ptr<httppeer> peer);

// v6 注册：唯一底层函数 reg_raw，pre/reg 各自独立指定 fn_kind
// 同步函数传 pre_sync / reg_sync（另一个传 nullptr）；协程函数传 co_pre / co_reg（另一个传 nullptr）
// autopickmethod 根据注解类型自动组合；手写注册也直接调这个
// 注册面门禁（未知 site 丢弃、同名保留首次）在 reg_add 里，source_location 必须在这里求值
inline unsigned int reg_raw(
    const std::string &site, const std::string &path, std::function<std::string(std::shared_ptr<httppeer>)> pre_sync, std::function<asio::awaitable<std::string>(std::shared_ptr<httppeer>)> co_pre, std::function<std::string(std::shared_ptr<httppeer>)> reg_sync, std::function<asio::awaitable<std::string>(std::shared_ptr<httppeer>)> co_reg, const std::source_location &loc = std::source_location::current())
{
    reg_methold_mid_t e;
    if (pre_sync)
    {
        e.pre_kind = fn_kind::sync;
        e.pre      = std::move(pre_sync);
    }
    else if (co_pre)
    {
        e.pre_kind = fn_kind::coro;
        e.co_pre   = std::move(co_pre);
    }
    else
    {
        e.pre_kind = fn_kind::none;
    }

    if (reg_sync)
    {
        e.reg_kind = fn_kind::sync;
        e.regfun   = std::move(reg_sync);
    }
    else if (co_reg)
    {
        e.reg_kind  = fn_kind::coro;
        e.co_regfun = std::move(co_reg);
    }

    e.reg_file = loc.file_name();
    e.reg_line = (unsigned int)loc.line();
    e.reg_fn   = loc.function_name();

    return reg_add(site, path, std::move(e));
}

// — 便捷宏（autopickmethod 不用，手写注册可用）—
#define REG_SYNC_SYNC(site, path, pre, reg) http::reg_raw(site, path, pre, nullptr, reg, nullptr)
#define REG_SYNC_CORO(site, path, pre, reg) http::reg_raw(site, path, pre, nullptr, nullptr, reg)
#define REG_CORO_SYNC(site, path, pre, reg) http::reg_raw(site, path, nullptr, pre, reg, nullptr)
#define REG_CORO_CORO(site, path, pre, reg) http::reg_raw(site, path, nullptr, pre, nullptr, reg)

// 公共工具函数：判断路径是否为普通文件 — shared util: check if path is a regular file
inline bool stat_is_regfile(const std::string &filepath)
{
    struct stat tempfileinfo;
    memset(&tempfileinfo, 0, sizeof(tempfileinfo));
    if (stat(filepath.c_str(), &tempfileinfo) == 0)
    {
        return (tempfileinfo.st_mode & S_IFREG) != 0;
    }
    return false;
}
}// namespace http
#endif
