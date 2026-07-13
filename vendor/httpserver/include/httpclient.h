#ifndef PROJECT_HTTPCLIENT_H
#define PROJECT_HTTPCLIENT_H

#include <string>
#include <cstddef>
#include <vector>
#include <mutex>
#include <string>
#include <string_view>
#include <list>
#include <map>
#include <sstream>
#include <functional>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/strand.hpp>
#include "datetime.h"
#include "cookie.h"
#include "request.h"

namespace http
{
struct upload_file
{
    std::string name;
    std::string filename;
    std::string tempfile;
    std::string type;
    unsigned long long size = 0;
    unsigned char error     = 0;
};

class client : public std::enable_shared_from_this<client>
{

  public:
    client();
    ~client();
    client(std::string_view url);
    client &get(std::string_view url, http::obj_val parmter);
    client &get(std::string_view url);
    client &get(std::string_view url, unsigned int time_out_num);
    client &post(std::string_view url, http::obj_val parmter);
    client &post(std::string_view url);
    client &get_json(std::string_view url, http::obj_val parmter);
    client &get_json(std::string_view url);
    client &post_json(std::string_view url, http::obj_val parmter);
    client &post_json(std::string_view url);

    client &set_cookie(std::string, std::string);
    client &set_header(std::string, std::string);
    client &add_header(std::string, std::string);
    client &add_header(std::string);
    client &set_body(std::string_view);
    client &set_body(std::string &&);
    client &set_body(const std::string &);

    void query(std::string_view url);

    client &add_cookie(const std::string &k, const std::string &v);
    client &requst_clear();
    client &clear();

    void set_host(const std::string &name)
    {
        host = name;
    }
    void set_path(const std::string &urlpath)
    {
        path = urlpath;
    }
    void set_port(const std::string &p)
    {
        port = p;
    }
    void set_port(unsigned int p)
    {
        port = std::to_string(p);
    }
    void set_query(const std::string &p)
    {
        query_ = p;
    }

    client &add_post(std::string_view, std::string_view);
    client &add_file(std::string);
    client &add_file(std::string, std::string);
    void assign_file(std::string, std::string);
    client &post_type(std::string);
    client &data_type(std::string);
    client &send();
    asio::awaitable<void> async_send();

    client &send_data();
    client &send_ssl_data();
    client &save(std::string);

    client &build_query(http::obj_val parmter);
    client &build_query(const std::string &a);
    client &build_query(const std::map<std::string, std::string> &);
    std::string get_query();

    bool parse_header_fields(const std::string &line_temp);
    void readheaderline(const char *buffer, unsigned int buffersize);
    void respreadtocontent(const char *buffer, unsigned int buffersize);
    void respreadtofile(const char *buffer, unsigned int buffersize);
    void responseheader(std::string_view, std::string_view);
    void respcookieprocess(std::string_view);
    void respattachmentprocess(std::string_view);
    void respcontenttypeprocess(std::string_view);
    void processcode();
    void finishprocess();
    bool process(const char *buffer, unsigned int buffersize);
    void close_file(std::FILE *fp) { std::fclose(fp); }
    void close_connect();
    void buildheader();
    void buildcontent();
    void timeout(unsigned int t) { exptime = t; };
    unsigned int timeout() { return exptime; };
    void reset_timeout()
    {
        timeout_end.store(timeid() + exptime);
    };
    void reset_connect_timeout()
    {
        timeout_end.fetch_add(exptime, std::memory_order_relaxed);
        timeout_count = timeout_end.load();
    };
    void set_timeout(unsigned int t)
    {
        timeout_end.store(timeid() + t);
    };
    unsigned int get_timeout()
    {
        return timeout_end.load();
    };
    void set_limit_time(unsigned short t) { timeout_total = t; };
    void clear_limit_time_count() { timeout_count = 0; };

    bool is_file() { return !page.istxt; };
    bool is_json() { return page.isjson; };
    bool init_http_sock();
    bool init_https_sock();

    asio::awaitable<bool> async_init_http_sock();
    asio::awaitable<bool> async_init_https_sock();
    asio::awaitable<void> async_send_data();
    asio::awaitable<void> async_send_ssl_data();
    bool parse();
    std::string get_body();
    const std::string &ref_body();
    std::string get_header();
    std::string get_tempfile();
    std::map<std::string, std::string> get_headers();
    std::string get_type();
    unsigned int get_length();
    unsigned int get_status();
    std::string get_status_msg();
    http::cookie get_cookie();
    std::string &get_cookie(const std::string &cookie_key);
    http::obj_val json();

    bool connect(std::string_view url, unsigned int time_out_num = 0);
    bool connect();
    asio::awaitable<bool> async_connect(std::string_view url, unsigned int time_out_num = 0);
    asio::awaitable<bool> async_connect();

    asio::awaitable<unsigned int> async_write(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_write(std::string_view value);
    asio::awaitable<unsigned int> async_read(unsigned char *data, unsigned int buffersize);
    asio::awaitable<unsigned int> async_read(std::string &data);

    unsigned int write(unsigned char *data, unsigned int buffersize);
    unsigned int write(std::string_view value);
    unsigned int read(unsigned char *data, unsigned int buffersize);
    unsigned int read(std::string &data);

  public:
    http::obj_val header;
    std::map<std::string, std::string> cookie;
    http::obj_val parameter;
    std::vector<std::pair<std::string, std::string>> postdata;
    std::string scheme;
    std::string host;
    std::string path;
    std::string query_;
    std::string port;

    std::string _url;
    // unsigned int statecode   = 0;
    std::stringstream sendcontent;
    std::stringstream postcontent;
    std::string response;
    std::string response_header;
    std::string request;
    asio::error_code ec;

    std::string temppath;

    std::atomic<unsigned int> timeout_end = 0;

    unsigned int exptime              = 0;
    unsigned int linktype             = 0;
    unsigned int serial_number        = 0;
    unsigned int durtime              = 0;
    unsigned short timeout_total      = 0;
    unsigned short timeout_count      = 0;
    unsigned char requesttype         = 0;
    bool isssl                        = false;
    bool ischunked                    = false;
    std::atomic_bool iswait_exit      = false;
    std::atomic_flag socket_read_lock = ATOMIC_FLAG_INIT;
    struct page_t
    {
        long long length  = 0;
        unsigned int code = 0;

        bool issse    = false;
        bool istxt    = false;
        bool isxml    = false;
        bool isjson   = false;
        bool chunked  = false;
        bool keeplive = false;
        char encode   = 0;
        char padd     = 0;
        std::string rawheader;
        std::string content;
        std::string codemessage;
        std::map<std::string, std::string> header;
        http::cookie cookie;
        upload_file file;
        http::obj_val json;
    } page;

    std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
    std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};

    std::function<void(const std::string &, std::shared_ptr<client>)> onload                         = nullptr;
    std::function<bool(const char *buffer, unsigned int readoffset, unsigned int httpcode)> onheader = nullptr;
    std::function<void(std::string &header_str)> onrequest                                           = nullptr;
    std::function<void(unsigned long long, unsigned long long)> upload_process                       = nullptr;
    std::function<void(unsigned long long, unsigned long long)> download_process                     = nullptr;

    asio::strand<asio::io_context::executor_type> strand_;
    std::function<void(std::shared_ptr<client>)> dur_time_loop_fun                        = nullptr;
    std::function<asio::awaitable<void>(std::shared_ptr<client>)> async_dur_time_loop_fun = nullptr;
    std::function<void(std::shared_ptr<client>)> run_task_fun                             = nullptr;//time-consuming task

    std::string use_certificate_file;
    std::string use_private_key_file;
    std::string load_verify_file;
    std::string error_msg;
    std::list<upload_file> files;
    std::list<upload_file> senddata;

  public:
    std::string contenttype;
    std::string contentline;

    unsigned char parsetojson = 0;

    std::string boundary;
    unsigned char headerfinish = 0;
    unsigned char machnum      = 0;
    unsigned char islineend    = 0;
    unsigned char iserror      = 0;
    unsigned int readoffset    = 0;
    //FILE *rawfile = NULL;
    std::unique_ptr<std::FILE, int (*)(FILE *)> rawfile;
    std::mutex lock_close;
};
}// namespace http
#endif// PROJECT_HTTPCLIENT_H
