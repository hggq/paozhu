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
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <cookie.h>
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

    class client
    {

    public:
        client(){};
        client(std::string_view url) : _url(url){};
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
        client &addfile(std::string);
        client &addfile(std::string, std::string);
        void assign_file(std::string, std::string);
        client &posttype(std::string);
        client &datatype(std::string);
        client &send();
        client &send(http::OBJ_VALUE parmter);
        client &senddatato();
        client &sendssldatato();
        client &save(std::string);
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
        http::OBJ_VALUE header;
        http::OBJ_VALUE cookie;
        http::OBJ_VALUE parameter;
        http::OBJ_VALUE data;
        void buildheader();
        void buildcontent();
        void timeout(unsigned int t) { exptime = t; };
        unsigned int timeout() { return exptime; };
        bool isfile() { return !state.istxt; };
        bool isjson() { return state.isjson; };
        bool parse();
        std::string getbody();
        std::string getheader();
        std::string gettempfile();
        std::map<std::string, std::string> getheaders();
        std::string gettype();
        unsigned int getlength();
        unsigned int getstate();
        std::string getstatus();
        Cookie getcookie();
        http::OBJ_VALUE json();
      

        std::string scheme;
        std::string host;
        std::string path;
        std::string query;
        std::string port;

        std::string _url;
        unsigned int statecode;
        std::stringstream sendcontent;
        std::stringstream postcontent;
        std::string response;
        std::string response_header;
        std::string request;
        asio::error_code ec;

        std::string temppath;
        struct state_t
        {
            unsigned int code = 0;
            unsigned int length = 0;
            bool istxt = false;
            bool isjson = false;
            bool chunked = false;
            bool keeplive = false;
            char encode;
            std::string content;
            std::string codemessage;
            std::map<std::string, std::string> header;
            Cookie cookie;
            upload_file page;
            http::OBJ_VALUE json;
        } state;

    private:
        unsigned int exptime = 0;

        std::string contenttype;
        std::string contentline;
        unsigned char requesttype = 0;
        unsigned char parsetojson = 0;
        std::list<upload_file> files;
        // struct postlist{
        //     unsigned char type;
        //     std::string content;
        //     std::string content;
        // };
        std::list<upload_file> senddata;
        std::string boundary;
        unsigned char headerfinish = 0;
        unsigned char machnum = 0;
        unsigned char islineend = 0;
        unsigned int readoffset = 0;
        unsigned char error = 0;
        FILE *rawfile = NULL;
        using temp_file = std::unique_ptr<std::FILE, decltype(&client::close_file)>;
        // temp_file ptr_(fopen("demo.txt", "r"),&close_file);
        // temp_file fp(std::fopen("demo.txt", "r"), &close_file);
        // temp_file tempfile(FILE *,decltype(&client::close_file));
    };
}
#endif // PROJECT_HTTPCLIENT_H
