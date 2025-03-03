#ifndef PROJECT_HTTPCLIENT_H
#define PROJECT_HTTPCLIENT_H

#include <string>
#include <cstddef>
#include <vector>
#include <string>
#include <string_view>
#include <list>
#include <map>
#include <sstream>
#include <functional>
#include <asio.hpp>
#include <asio/ssl.hpp>
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
    unsigned int size;
    unsigned char error;
};

class client : public std::enable_shared_from_this<client>
{

  public:
    client() : rawfile(nullptr, std::fclose){};
    ~client();
    client(std::string_view url) : _url(url), rawfile(nullptr, std::fclose){};
    client &get(std::string_view url, http::OBJ_VALUE parmter);
    client &get(std::string_view url);
    client &post(std::string_view url, http::OBJ_VALUE parmter);
    client &post(std::string_view url);
    client &getjson(std::string_view url, http::OBJ_VALUE parmter);
    client &getjson(std::string_view url);
    client &postjson(std::string_view url, http::OBJ_VALUE parmter);
    client &postjson(std::string_view url);

    client &setcookie(std::string, std::string);
    client &setheader(std::string, std::string);
    client &addheader(std::string, std::string);
    client &addheader(std::string);
    client &setBody(const std::string &);

    client &addcookie(const std::string &k, const std::string &v);
    client &requst_clear();

    client &addfile(std::string);
    client &addfile(std::string, std::string);
    void assign_file(std::string, std::string);
    client &posttype(std::string);
    client &datatype(std::string);
    client &send();
    client &send(http::OBJ_VALUE parmter);

    asio::awaitable<void> async_send();
    asio::awaitable<void> async_send(http::OBJ_VALUE parmter);

    client &senddatato();
    client &sendssldatato();
    client &save(std::string);

    client &build_query(http::OBJ_VALUE parmter);
    client &build_query(const std::string &a);
    client &build_query(const std::map<std::string, std::string> &);
    std::string get_query();

    void readheaderline(const char *buffer, unsigned int buffersize);
    void respreadtocontent(const char *buffer, unsigned int buffersize);
    void respreadtofile(const char *buffer, unsigned int buffersize);
    void responseheader(std::string_view, std::string_view);
    void respcookieprocess(std::string_view);
    void respattachmentprocess(std::string_view);
    void respcontenttypeprocess(std::string_view);
    void processcode();
    void finishprocess();
    void process(const char *buffer, unsigned int buffersize);
    void close_file(std::FILE *fp) { std::fclose(fp); }

    void buildheader();
    void buildcontent();
    void timeout(unsigned int t) { exptime = t; };
    unsigned int timeout() { return exptime; };
    bool isfile() { return !state.istxt; };
    bool isjson() { return state.isjson; };
    bool init_http_sock();
    bool init_https_sock();

    asio::awaitable<bool> co_init_http_sock();
    asio::awaitable<bool> co_init_https_sock();
    asio::awaitable<void> co_senddatato();
    asio::awaitable<void> co_sendssldatato();
    bool parse();
    std::string getBody();
    std::string getHeader();
    std::string getTempfile();
    std::map<std::string, std::string> getHeaders();
    std::string getType();
    unsigned int getLength();
    unsigned int getStatus();
    std::string getStatus_msg();
    Cookie getCookie();
    http::OBJ_VALUE json();

  public:
    http::OBJ_VALUE header;
    std::map<std::string, std::string> cookie;
    http::OBJ_VALUE parameter;
    http::OBJ_VALUE data;
    std::string scheme;
    std::string host;
    std::string path;
    std::string query;
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

    unsigned int exptime       = 0;
    unsigned int timeout_end   = 0;
    unsigned int linktype      = 0;
    unsigned int serial_number = 0;
    unsigned char requesttype  = 0;
    bool isssl                 = false;
    struct state_t
    {
        unsigned int code   = 0;
        unsigned int length = 0;
        bool istxt          = false;
        bool isjson         = false;
        bool chunked        = false;
        bool keeplive       = false;
        char encode;
        std::string content;
        std::string codemessage;
        std::map<std::string, std::string> header;
        Cookie cookie;
        upload_file page;
        http::OBJ_VALUE json;
    } state;

    std::shared_ptr<asio::ip::tcp::socket> sock                       = {nullptr};
    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsock = {nullptr};
    std::shared_ptr<asio::ssl::context> ssl_context                   = {nullptr};

    std::function<void(const std::string &, std::shared_ptr<http::client>)> onload = nullptr;
    std::function<void(unsigned long long, unsigned long long)> upload_process     = nullptr;
    std::string use_certificate_file;
    std::string use_private_key_file;
    std::string load_verify_file;
    std::string error_msg;
    std::list<upload_file> files;
    std::list<upload_file> senddata;

  private:
    std::string contenttype;
    std::string contentline;

    unsigned char parsetojson = 0;

    std::string boundary;
    unsigned char headerfinish = 0;
    unsigned char machnum      = 0;
    unsigned char islineend    = 0;
    unsigned char error        = 0;
    unsigned int readoffset    = 0;
    //FILE *rawfile = NULL;
    std::unique_ptr<std::FILE, int (*)(FILE *)> rawfile;
};
}// namespace http
#endif// PROJECT_HTTPCLIENT_H
