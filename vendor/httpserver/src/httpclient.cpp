/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file httpclient.cpp
 *  @date 2022-05-15
 *
 *  http client supper http(s) file
 *
 *
 */
#include <algorithm>
#include <sstream>
#include <ctime>
#include <array>
#include <string>
#include <string_view>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "http_socket.h"
#include "request.h"
#include "httpclient.h"
#include "urlcode.h"
#include "md5.h"
#include "zlib.h"
#include "http_mime.h"
#include "datetime.h"
#include "gzip.h"

namespace http
{
client &client::get(std::string_view url)
{
    requesttype = 0;
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::get(std::string_view url, http::OBJ_VALUE param)
{
    requesttype = 0;
    data        = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::post(std::string_view url)
{
    requesttype = 1;

    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::post(std::string_view url, http::OBJ_VALUE param)
{
    requesttype = 1;
    parameter   = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::getjson(std::string_view url)
{
    requesttype = 0;
    parsetojson = 1;
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::getjson(std::string_view url, http::OBJ_VALUE param)
{
    requesttype = 0;
    parsetojson = 1;
    data        = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::postjson(std::string_view url)
{

    requesttype            = 1;
    parsetojson            = 1;
    header["Content-Type"] = "application/json";
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}
client &client::postjson(std::string_view url, http::OBJ_VALUE param)
{
    requesttype            = 1;
    header["Content-Type"] = "application/json";

    parsetojson = 1;

    parameter = std::move(param);
    if (url.length() > 0)
    {
        _url = url;
        parse();
    }
    else if (_url.length() > 0)
    {
        parse();
    }
    return *this;
}

client &client::send()
{

    if (requesttype == 1)
    {
        buildcontent();
        buildheader();
    }
    else
    {
        buildheader();
    }

    if (scheme == "https")
    {
        sendssldatato();
    }
    else
    {
        senddatato();
    }

    return *this;
}

client &client::send(http::OBJ_VALUE param)
{
    data = std::move(param);
    if (requesttype == 1)
    {
        buildcontent();
        buildheader();
    }
    else
    {
        buildheader();
    }
    if (scheme == "https")
    {
        sendssldatato();
    }
    else
    {
        senddatato();
    }
    return *this;
}

client &client::senddatato()
{
    try
    {
        response_header.clear();
        state.code     = 0;
        state.length   = 0;
        state.istxt    = false;
        state.isjson   = false;
        state.chunked  = false;
        state.keeplive = false;
        state.encode   = 0x00;

        state.content.clear();
        state.codemessage.clear();
        state.cookie.clear();

        state.page.name.clear();
        state.page.filename.clear();
        state.page.tempfile.clear();
        state.page.type.clear();
        state.page.size  = 0;
        state.page.error = 0;
        state.json.clear();

        asio::io_context clientio_context(1);
        asio::signal_set signals(clientio_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { clientio_context.stop(); });

        asio::ip::tcp::socket socket(clientio_context);
        asio::ip::tcp::resolver resolver(clientio_context);

        asio::ip::tcp::resolver::query checkquery(host, port);
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(checkquery);
        asio::ip::tcp::resolver::iterator end;
        asio::ip::tcp::endpoint endpoint;
        while (iter != end)
        {
            endpoint = *iter++;
            // at here  maybe under code
        }
        // asio::error_code ec;
        socket.connect(endpoint, ec);
        if (ec)
        {
            std::cerr << "Unable to connect:  \r\n";
            return *this;
        }
        if (exptime < 0 || exptime > 30)
        {
            exptime = 10;
        }
        if (exptime > 0)
        {

            std::thread(
                [&, timenum = exptime]()
                {
                    std::this_thread::sleep_for(std::chrono::seconds(timenum));
                    if (socket.is_open())
                    {
                        socket.close();
                    }
                    clientio_context.stop();
                })
                .detach();
        }
        socket.write_some(asio::buffer(request));

        char data[2051];
        unsigned int n;
        if (requesttype == 1)
        {
            for (auto &p : senddata)
            {
                switch (p.error)
                {
                case 0: socket.write_some(asio::buffer(p.tempfile)); break;
                case 3: socket.write_some(asio::buffer(p.tempfile)); break;
                case 1:
                {
                    unsigned int readnum = 0;
                    socket.write_some(asio::buffer(p.tempfile));
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(p.filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (!fp)
                    {
                        data[0] = 0x0D;
                        data[1] = 0x0A;
                        asio::write(socket, asio::buffer(data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    data[2048] = 0x00;
                    while (readnum < n)
                    {

                        auto nread = fread(data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            data[nread] = 0x0D;
                            nread += 1;
                            data[nread] = 0x0A;
                            nread += 1;
                        }
                        asio::write(socket, asio::buffer(data, nread));
                    }

                    // fclose(fp);
                }
                break;
                case 2:
                {
                    unsigned int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(p.filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (!fp)
                    {
                        data[0] = 0x0D;
                        data[1] = 0x0A;
                        asio::write(socket, asio::buffer(data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    data[2048] = 0x00;
                    while (readnum < n)
                    {

                        auto nread = fread(data, 1, 2048, fp.get());
                        readnum += nread;
                        asio::write(socket, asio::buffer(data, nread));
                    }

                    // fclose(fp);
                }
                break;
                }
            }
        }

        // unsigned long long totalsize;
        // rawfile = NULL;
        while (true)
        {
            if (state.page.size > 0 && (state.page.size - state.content.size() < 2048))
            {
                n = socket.read_some(asio::buffer(data, state.page.size - state.content.size()), ec);
            }
            else
            {
                n = socket.read_some(asio::buffer(data, 2048), ec);
            }
            // n = socket.read_some(asio::buffer(data, 2048), ec);
            // read(sp, asio::buffer(data, 2048), asio::detail::transfer_at_least_t(1));
            data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            process(data, n);
        }
        finishprocess();
        if (socket.is_open())
        {
            socket.close();
        }
        clientio_context.stop();
    }
    catch (std::exception &e)
    {
        // std::printf("Exception: %s\n", e.what());
        std::cerr << "Exception:  " << e.what() << "\r\n";
    }

    return *this;
}
client &client::sendssldatato()
{
    // ssl请求
    try
    {
        response_header.clear();
        state.code     = 0;
        state.length   = 0;
        state.istxt    = false;
        state.isjson   = false;
        state.chunked  = false;
        state.keeplive = false;
        state.encode   = 0x00;

        state.content.clear();
        state.codemessage.clear();
        state.cookie.clear();

        state.page.name.clear();
        state.page.filename.clear();
        state.page.tempfile.clear();
        state.page.type.clear();
        state.page.size  = 0;
        state.page.error = 0;
        state.json.clear();

        asio::io_context io_context;
        asio::ssl::context ssl_context(asio::ssl::context::sslv23);

        asio::ssl::stream<asio::ip::tcp::socket> socket(io_context, ssl_context);
        ssl_context.set_default_verify_paths();
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(host.c_str(), port);

        SSL_set_tlsext_host_name(socket.native_handle(), host.c_str());

        asio::connect(socket.lowest_layer(), endpoints);
        socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));

        ssl_context.set_verify_mode(asio::ssl::verify_peer);
        ssl_context.set_verify_callback(asio::ssl::host_name_verification(host));

        socket.handshake(asio::ssl::stream_base::client, ec);

        if (ec)
        {
            return *this;
        }

        if (exptime < 0 || exptime > 30)
        {
            exptime = 10;
        }
        if (exptime > 0)
        {
            std::thread(
                [&, timenum = exptime]()
                {
                    try
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(timenum));
                        if (socket.lowest_layer().is_open())
                        {
                            socket.shutdown();
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                })
                .detach();
        }

        int n;
        n = socket.write_some(asio::buffer(request));

        char data[2051];

        if (requesttype == 1)
        {
            for (auto &p : senddata)
            {

                switch (p.error)
                {
                case 0: socket.write_some(asio::buffer(p.tempfile)); break;
                case 3: socket.write_some(asio::buffer(p.tempfile)); break;
                case 1:
                {
                    int readnum = 0;
                    socket.write_some(asio::buffer(p.tempfile));
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(p.filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (!fp)
                    {
                        data[0] = 0x0D;
                        data[1] = 0x0A;
                        asio::write(socket, asio::buffer(data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    data[2048] = 0x00;
                    while (readnum < n)
                    {

                        auto nread = fread(data, 1, 2048, fp.get());
                        readnum += nread;
                        if (readnum >= n)
                        {
                            data[nread] = 0x0D;
                            nread += 1;
                            data[nread] = 0x0A;
                            nread += 1;
                        }
                        asio::write(socket, asio::buffer(data, nread));
                    }

                    // fclose(fp);
                }
                break;
                case 2:
                {
                    int readnum = 0;
                    // FILE *fp = fopen(p.filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(p.filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (!fp)
                    {
                        data[0] = 0x0D;
                        data[1] = 0x0A;
                        asio::write(socket, asio::buffer(data, 2));
                        break;
                    }

                    fseek(fp.get(), 0, SEEK_END);
                    n = ftell(fp.get());
                    fseek(fp.get(), 0, SEEK_SET);

                    data[2048] = 0x00;
                    while (readnum < n)
                    {

                        auto nread = fread(data, 1, 2048, fp.get());
                        readnum += nread;
                        asio::write(socket, asio::buffer(data, nread));
                    }

                    // fclose(fp);
                }
                break;
                }
            }
        }
        // asio::co_spawn(io_context, httpsslclientecho(std::ref(socket),std::ref(ssl_context)), asio::detached);

        // unsigned long long totalsize;
        // rawfile = NULL;
        while (true)
        {
            if (state.page.size > 0 && (state.page.size - state.content.size() < 2048))
            {
                n = socket.read_some(asio::buffer(data, state.page.size - state.content.size()), ec);
            }
            else
            {
                n = socket.read_some(asio::buffer(data, 2048), ec);
            }
            // n = socket.read_some(asio::buffer(data, 2048), ec);
            data[2048] = 0x00;
            if (n == 0)
            {
                break;
            }
            readoffset = 0;
            process(data, n);
        }
        finishprocess();

        socket.shutdown();
        io_context.stop();
        return *this;
    }
    catch (std::exception &e)
    {
        std::printf("Exception: %s\n", e.what());
    }

    return *this;
}
void client::processcode()
{
    std::string key, value;
    unsigned int code = 0;
    unsigned int j    = 0;
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] == 0x20)
        {
            break;
        }
        value.push_back(contentline[j]);
    }
    if (value[0] != 'H' && value[0] != 'h')
    {
        state.code = 0;
        error      = 1;

        return;
    }

    j++;
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] != 0x20)
        {

            break;
        }
    }
    value.clear();
    for (; j < contentline.size(); j++)
    {
        if (contentline[j] == 0x20)
        {
            break;
        }
        if (contentline[j] >= '0' && contentline[j] <= '9')
        {
            code = code * 10 + (contentline[j] - '0');
        }
    }
    state.code = code;
    if (code == 0)
    {

        error = 1;
        return;
    }
    j++;
    for (; j < contentline.size(); j++)
    {
        state.codemessage.push_back(contentline[j]);
    }
}
void client::readheaderline(const char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    if (machnum > 0)
    {
        if (machnum % 2 == 0)
        {
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
                machnum++;
                if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                    machnum++;
                }
            }
        }
        else
        {
            if (buffer[i] == 0x0A && i < buffersize)
            {
                i++;
                machnum++;
                if (buffer[i] == 0x0D && i < buffersize)
                {
                    i++;
                    machnum++;
                    if (buffer[i] == 0x0A && i < buffersize)
                    {
                        i++;
                        machnum++;
                    }
                }
            }
        }
    }
    if (machnum > 3)
    {
        headerfinish = 1;
        readoffset   = i;
        return;
    }
    machnum     = 0;
    int lineend = 0;
    for (; i < buffersize; i++)
    {
        if (buffer[i] == 0x0D)
        {
            lineend = 1;
            break;
        }
        contentline.push_back(buffer[i]);
    }
    if (lineend == 1)
    {

        if (contentline.size() > 0)
        {
            response_header.append(contentline);
            response_header.append("\r\n");
            if (state.code == 0)
            {

                processcode();
                if (state.code == 0)
                {
                    error      = 1;
                    readoffset = i;
                    return;
                }
            }
            else
            {
                std::string key, value;
                bool isfirst;
                isfirst = true;
                for (unsigned int j = 0; j < contentline.size(); j++)
                {
                    if (isfirst && contentline[j] == ':')
                    {
                        key = value;
                        j++;
                        for (; j < contentline.size(); j++)
                        {
                            if (contentline[j] != 0x20)
                            {
                                j -= 1;
                                break;
                            }
                        }
                        value.clear();
                        isfirst = false;
                        continue;
                    }
                    value.push_back(contentline[j]);
                }
                if (key.size() > 0)
                {
                    if (key.size() == 10 && key == "Set-Cookie")
                    {
                        respcookieprocess(value);
                    }
                    else
                    {
                        state.header[key] = value;
                        responseheader(key, value);
                    }
                }
            }
        }
        contentline.clear();
        machnum++;
        i++;
        if (buffer[i] == 0x0A && i < buffersize)
        {
            i++;
            machnum++;
            if (buffer[i] == 0x0D && i < buffersize)
            {
                i++;
                machnum++;
                if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                    machnum++;
                }
            }
        }
        if (machnum > 3)
        {
            headerfinish = 1;
            readoffset   = i;
            return;
        }
    }
    readoffset = i;
}
void client::respcookieprocess(std::string_view str)
{
    // Set-Cookie: name=aabb; expires=Mon, 28-Mar-2022 12:10:55 GMT; Max-Age=3600; path=/; domain=localhost; secure;
    // HttpOnly
    std::string name, value;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            name = value;

            value.clear();
            continue;
        }
        if (str[i] == ';')
        {

            break;
        }
        value.push_back(str[i]);
    }
    if (name.size() > 0)
    {
        name  = url_decode(name.data(), name.size());
        value = url_decode(value.data(), value.size());
        state.cookie.set(name, value);
    }
}
void client::respattachmentprocess(std::string_view str)
{
    // Content-Disposition: attachment; filename="filename.jpg"
    std::string name, value;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            name = value;

            value.clear();
            continue;
        }
        if (name.size() > 0 && str[i] == 0x22)
        {
            // value.clear();
            continue;
        }
        if (str[i] == ';')
        {
            if (value == "attachment")
            {
                state.istxt = false;
            }
            value.clear();
            continue;
        }
        value.push_back(str[i]);
    }
    if (name == "filename")
    {
        state.istxt         = false;
        state.page.filename = std::move(value);
    }
}
void client::respcontenttypeprocess(std::string_view str)
{
    // text/html; charset=UTF-8 //application/xhtml+xml, application/json, application/xml
    std::string name, value;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (str[i] == '=')
        {
            name = value;

            value.clear();
            continue;
        }

        if (str[i] == ';')
        {
            if (name.empty())
            {
                state.page.type = value;
            }
            value.clear();
            continue;
        }
        value.push_back(str[i]);
    }
    if (name.empty())
    {
        if (state.page.type.empty())
        {
            state.page.type = value;
        }
    }
    switch (state.page.type.size())
    {
    case 22:
        if (state.page.type == "application/javascript")
        {
            state.istxt = true;
        }
        break;
    case 21:
        if (state.page.type == "application/xhtml+xml")
        {
            state.istxt = true;
        }
        break;
    case 16:
        if (state.page.type == "application/json")
        {
            state.istxt  = true;
            state.isjson = true;
        }
        break;
    case 15:
        if (state.page.type == "application/xml")
        {
            state.istxt = true;
        }
        break;
    case 10:
        if (state.page.type == "text/plain")
        {
            state.istxt = true;
        }
        break;
    case 9:
        if (state.page.type == "text/html")
        {
            state.istxt = true;
        }
        break;
    case 8:
        if (state.page.type == "text/css")
        {
            state.istxt = true;
        }
        break;
    case 7:
        if (state.page.type == "text/js")
        {
            state.istxt = true;
        }
        break;
    }
    if (state.istxt == false)
    {
        if (state.page.filename.empty())
        {
            std::string::size_type lastpos = path.find_last_of(0x2F);
            if (lastpos != std::string::npos)
            {
                state.page.filename = path.substr(lastpos + 1);
                if (state.page.filename.size() > 120)
                {
                    state.page.filename.resize(120);
                }
            }
        }
    }
}
void client::responseheader(std::string_view key, std::string_view value)
{
    // content-encoding
    switch (key.size())
    {
    case 19:

        if (key == "Content-Disposition")
        {
            respattachmentprocess(value);
        }
        break;
    case 17:

        if (key == "Transfer-Encoding")
        {
            if (value == "chunked")
            {
                state.chunked = true;
            }
        }
        break;
    case 16:

        if (key == "Content-Encoding")
        {
            switch (value.size())
            {
            case 2:
                if (value[0] == 'b')
                {
                    // state.br=true;
                    state.encode = 'b';
                }

                break;
            case 4:
                if (value[0] == 'g')
                {
                    //  state.gzip=true;
                    state.encode = 'g';
                }

                break;
            case 7:
                if (value[0] == 'd')
                {
                    //     state.deflate=true;
                    state.encode = 'd';
                }

                break;
            default:;
            }
        }
        break;
    case 14:
        if (key == "Content-Length")
        {
            try
            {
                state.page.size = std::stoi(value.data());
            }
            catch (...)
            {
                state.page.size = 0;
            }
        }
        break;
    case 12:
        if (key == "Content-Type")
        {
            respcontenttypeprocess(value);
        }
        break;
    case 10:
        if (key == "Connection")
        {
            // Keep-Alive
            if (value[0] == 'K' || value[0] == 'k')
            {
                //    if(value[1]=='e'&&value[2]=='e'){
                state.keeplive = true;
                //     }
            }
            else
            {
                state.keeplive = false;
            }
        }
        break;
    }
}
unsigned int client::getlength() { return state.page.size; }
unsigned int client::getstate() { return state.code; }
std::string client::getstatus() { return state.codemessage; }
std::string client::getheader() { return response_header; }
std::string client::gettempfile() { return state.page.tempfile; }
std::map<std::string, std::string> client::getheaders() { return state.header; }
std::string client::gettype() { return state.page.type; }
std::string client::getbody()
{
    if (state.istxt)
    {
        return state.content;
    }
    else
    {
        // FILE *ffp = fopen(state.page.tempfile.c_str(), "rb");
        std::unique_ptr<std::FILE, decltype(&std::fclose)> ffp(fopen(state.page.tempfile.c_str(), "rb"), &std::fclose);
        if (!ffp)
        {
            return "";
        }
        fseek(ffp.get(), 0, SEEK_END);
        unsigned int nsize = ftell(ffp.get());
        fseek(ffp.get(), 0, SEEK_SET);

        state.content.resize(nsize);

        unsigned int nread = fread(&state.content[0], 1, nsize, ffp.get());
        state.content.resize(nread);
        // fclose(ffp);
    }
    return state.content;
}
Cookie client::getcookie() { return state.cookie; }
http::OBJ_VALUE client::json() { return state.json; }
void client::respreadtocontent(const char *buffer, unsigned int buffersize)
{
    unsigned int i = readoffset;
    // unsigned int offset = buffersize - i;

    if (state.chunked)
    {
        if (state.length == 0)
        {
            int n = 0;
            for (; i < buffersize; i++)
            {
                if (buffer[i] == 0x0D || buffer[i] == 0x0A)
                {
                    break;
                }
                if (buffer[i] >= '0' && buffer[i] <= '9')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - '0');
                }
                else if (buffer[i] >= 'A' && buffer[i] <= 'F')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - 'A') + 10;
                }
                else if (buffer[i] >= 'a' && buffer[i] <= 'f')
                {
                    n = n << 4;
                    n = n + (unsigned char)(buffer[i] - 'a') + 10;
                }
            }

            state.length = n;

            if (buffer[i] == 0x0D)
            {
                i++;
                if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                }
            }
            else if (buffer[i] == 0x0A && i < buffersize)
            {
                i++;
            }

            if (state.length == 0)
            {
                if (buffer[i] == 0x0D)
                {
                    i++;
                    if (buffer[i] == 0x0A && i < buffersize)
                    {
                        i++;
                    }
                }
                else if (buffer[i] == 0x0A && i < buffersize)
                {
                    i++;
                }
                return;
            }
            readoffset = i;
        }

        for (; i < buffersize && state.length > 0; i++, state.length -= 1)
        {
            state.content.push_back(buffer[i]);
        }
        readoffset = i;
        if (readoffset < buffersize)
        {
            respreadtocontent(buffer, buffersize);
        }
    }
    else
    {
        for (; i < buffersize; i++)
        {
            state.content.push_back(buffer[i]);
        }
    }
}
void client::respreadtofile(const char *buffer, unsigned int buffersize)
{

    unsigned int i      = readoffset;
    unsigned int offset = 0;

    if (i >= buffersize)
    {
        i = buffersize;
    }
    offset = buffersize - i;
    if (offset > buffersize)
    {
        offset = 0;
    }
    if (!rawfile)
    {
        unsigned long long tetime = time((time_t *)NULL);
        srand(tetime);
        unsigned long long randnum = rand() % 100000 + 100000;

        state.page.tempfile = "c" + std::to_string(tetime) + std::to_string(randnum);
        state.page.tempfile.append("_t");

        struct stat s;
        std::string tempfilename;
        if (temppath.empty())
        {
            temppath = "./temp";
        }
        if (stat(temppath.c_str(), &s) == 0)
        {

            if (s.st_mode & S_IFDIR)
            {
                if (temppath.back() == '/')
                {
                    tempfilename = temppath;
                    tempfilename.append(state.page.tempfile);
                    state.page.tempfile = tempfilename;
                }
                else
                {
                    tempfilename = temppath;
                    tempfilename.push_back('/');
                    tempfilename.append(state.page.tempfile);
                    state.page.tempfile = tempfilename;
                }
            }
        }

        // rawfile = fopen(state.page.tempfile.c_str(), "wb");
        rawfile.reset(fopen(state.page.tempfile.c_str(), "wb"));
    }
    if (i < buffersize && rawfile)
    {

        fwrite(&buffer[i], offset, 1, rawfile.get());
    }
}
void client::finishprocess()
{
    if (rawfile)
    {
        rawfile.reset(nullptr);
        // fclose(rawfile);
        // rawfile = NULL;
    }
    if (state.encode == 'g')
    {
        std::string content;
        uncompress(state.content, content);
        state.content = std::move(content);
    }
    if (state.isjson || parsetojson == 1)
    {
        state.json.from_json(state.content);
        if (parsetojson == 1)
            state.isjson = true;
    }
}
void client::process(const char *buffer, unsigned int buffersize)
{
    if (readoffset >= buffersize)
    {
        return;
    }
    if (error > 0)
    {
        return;
    }
    if (headerfinish == 0)
    {
        for (; readoffset < buffersize;)
        {
            readheaderline(buffer, buffersize);
            if (error > 0)
            {
                break;
            }
            if (headerfinish == 1)
            {
                break;
            }
        }
    }
    if (headerfinish == 1 && error == 0 && readoffset < buffersize)
    {
        // type length chunked
        if (state.istxt)
        {
            respreadtocontent(buffer, buffersize);
        }
        else
        {
            respreadtofile(buffer, buffersize);
        }
    }
}
client &client::datatype(std::string str = "")
{

    if (str.empty())
    {
        parsetojson = 1;
    }
    else
    {
        if (str == "json")
        {
            parsetojson = 1;
        }
    }
    return *this;
}
client &client::posttype(std::string str = "")
{

    if (str.empty())
    {
        header["Content-Type"] = "application/x-www-form-urlencoded";
    }
    else
    {
        header["Content-Type"] = str;
    }
    return *this;
}
client &client::save(std::string path = "")
{

    // fix prepath safe path
    if (path.empty())
    {
        path.append("./");
        path.append(state.page.filename);
    }

    struct stat s;
    if (stat(path.c_str(), &s) == 0)
    {

        if (s.st_mode & S_IFDIR)
        {
            if (path.back() == '/')
            {
                path.append(state.page.filename);
            }
            else
            {
                path.push_back('/');
                path.append(state.page.filename);
            }
        }
    }
    if (state.page.tempfile.size() > 0 && state.page.tempfile.size() < 138)
    {
        rename(state.page.tempfile.c_str(), path.c_str());
    }

    return *this;
}

client &client::setheader(std::string key, std::string value)
{

    header[key] = value;
    return *this;
}
client &client::addheader(std::string key, std::string value)
{

    header[key] = value;
    return *this;
}
client &client::addheader(std::string vvalue)
{
    std::string key;
    std::string value;

    for (unsigned int j = 0; j < vvalue.size(); j++)
    {
        if (vvalue[j] == 0x0A)
        {
            continue;
        }
        if (vvalue[j] == 0x0D)
        {
            header[key] = value;
            key.clear();
            value.clear();
        }
        else if (vvalue[j] == ':')
        {
            key = value;
            value.clear();
        }
        else
        {
            value.push_back(vvalue[j]);
        }
    }

    if (key.size() > 0 && value.size() > 0)
    {
        header[key] = value;
    }

    return *this;
}
bool client::parse()
{
    if (_url.length() == 0)
    {
        return false;
    }
    unsigned int i = 0;
    for (; i < _url.length(); i++)
    {
        if (_url[i] != 0x20)
        {
            break;
        }
    }
    scheme.clear();
    host.clear();
    port.clear();
    path.clear();
    query.clear();

    if (_url[i] == 'h' && _url[i + 1] == 't' && _url[i + 2] == 't' && _url[i + 3] == 'p' && _url[i + 4] == ':')
    {
        scheme = "http";
        i += 5;
    }
    if (_url[i] == 'h' && _url[i + 1] == 't' && _url[i + 2] == 't' && _url[i + 3] == 'p' && _url[i + 4] == 's' &&
        _url[i + 5] == ':')
    {
        scheme = "https";
        i += 6;
    }
    if (scheme.empty())
    {
        scheme = "http";
    }
    for (; i < _url.length(); i++)
    {
        if (_url[i] == 0x2F)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    for (; i < _url.length(); i++)
    {
        if (_url[i] == 0x2F || _url[i] == 0x3A || _url[i] == 0x3F)
        {

            break;
        }
        host.push_back(_url[i]);
    }

    if (_url[i] == 0x3A)
    {
        i++;
        for (; i < _url.length(); i++)
        {
            if (_url[i] == 0x2F)
            {
                break;
            }
            port.push_back(_url[i]);
        }
    }

    if (_url[i] == 0x3F)
    {
        path = "/";
        i++;
        for (; i < _url.length(); i++)
        {
            query.push_back(_url[i]);
        }
    }
    else if (_url[i] != 0x2F)
    {
        path = "/";
    }
    else
    {
        for (; i < _url.length(); i++)
        {
            if (_url[i] == 0x3F)
            {
                break;
            }
            path.push_back(_url[i]);
        }

        if (_url[i] == 0x3F)
        {
            i++;
            for (; i < _url.length(); i++)
            {
                query.push_back(_url[i]);
            }
        }
    }
    if (port.empty())
    {
        if (scheme == "https")
        {
            port = "443";
        }
        else
        {
            port = "80";
        }
    }

    transform(host.begin(), host.end(), host.begin(), ::tolower);
    return true;
}
void client::buildheader()
{
    sendcontent.str("");
    request.clear();
    std::string tempport;
    if (requesttype == 0)
    {
        request.append("GET ");
        request.append(path);
        if (query.empty())
        {

            bool isfirst = true;
            for (auto [first, second] : data.as_array())
            {
                if (isfirst)
                {
                    request.push_back('?');
                    isfirst = false;
                }
                else
                {
                    request.push_back('&');
                }
                if (!second.is_array())
                {
                    tempport = data.get_keyname(first);
                    tempport = url_encode(tempport.data(), tempport.size());
                    request.append(tempport);
                    tempport = second.to_string();
                    tempport = url_encode(tempport.data(), tempport.size());
                    request.push_back('=');

                    request.append(tempport);
                }
            }
        }
        else
        {
            request.push_back('?');
            request.append(query);
            for (auto [first, second] : data.as_array())
            {
                request.push_back('&');
                if (!second.is_array())
                {
                    tempport = data.get_keyname(first);
                    tempport = url_encode(tempport.data(), tempport.size());
                    request.append(tempport);
                    tempport = second.to_string();
                    request.push_back('=');
                    tempport = url_encode(tempport.data(), tempport.size());
                    request.append(tempport);
                }
            }
        }
        request.append(" HTTP/1.1\r\n");
    }
    else
    {
        request.append("POST ");
        request.append(path);
        if (query.empty())
        {
            request.append(" HTTP/1.1\r\n");
        }
        else
        {

            request.append("?");
            request.append(query);
            request.append(" HTTP/1.1\r\n");
        }
    }

    request.append("Host: ");
    request.append(host);

    tempport = port;
    if (!port.empty())
    {
        if (scheme == "https")
        {

            if (port == "443")
            {
                tempport.clear();
            }
        }
        else
        {
            if (port == "80")
            {
                tempport.clear();
            }
        }
    }

    if (tempport.empty())
    {
        request.append("\r\n");
    }
    else
    {
        request.push_back(':');
        request.append(tempport);
        request.append("\r\n");
    }

    if (header.isset("User-Agent"))
    {
        request.append("User-Agent: ");
        request.append(header["User-Agent"].as_string());
        request.append("\r\n");
        header.unset("User-Agent");
    }
    else
    {
        request.append("User-Agent: httpclient\r\n");
    }

    if (!header.isset("Accept"))
    {
        header["Accept"] = "text/html, application/xhtml+xml, application/json, application/xml;q=0.9, */*;q=0.8";
    }

    for (auto [first, second] : header.as_array())
    {
        if (!second.is_array())
        {
            request.append(header.get_keyname(first));
            request.append(": ");
            request.append(second.to_string());
            request.append("\r\n");
        }
    }
    if (!header.isset("Connection"))
    {
        request.append("Connection: close\r\n");
    }
    request.append("\r\n");
}
client &client::addfile(std::string filename)
{
    std::string key = "upfile[]";
    assign_file(std::move(key), std::move(filename));
    return *this;
}
client &client::addfile(std::string key, std::string filename)
{

    assign_file(std::move(key), std::move(filename));
    return *this;
}
void client::assign_file(std::string key, std::string filename)
{
    upload_file temp;
    temp.name     = key;
    temp.filename = filename;

    std::string::size_type found = filename.find_last_of('.');
    std::string extfile;
    if (found != std::string::npos)
    {
        extfile = filename.substr(found + 1);
        transform(extfile.begin(), extfile.end(), extfile.begin(), ::tolower);

        temp.type = mime_map[extfile];
        if (temp.type.empty())
        {
            temp.type = "application/octet-stream";
        }
    }
    else
    {
        temp.type = "application/octet-stream";
    }

    // FILE *f = fopen(filename.c_str(), "rb");
    std::unique_ptr<std::FILE, decltype(&std::fclose)> f(fopen(filename.c_str(), "rb"), &std::fclose);
    if (f)
    {
        fseek(f.get(), 0, SEEK_END);
        temp.size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);

        // fclose(f);
    }
    else
    {
        temp.size = 0;
    }

    files.emplace_back(std::move(temp));
    header["Content-Type"] = "multipart/form-data";
}
void client::buildcontent()
{

    if (!header.isset("Content-Type"))
    {
        if (files.size() == 0)
        {
            contenttype = "application/x-www-form-urlencoded";
        }
        else
        {
            contenttype = "multipart/form-data";
        }
    }
    else
    {
        contenttype = header["Content-Type"].as_string();
        header.unset("Content-Type");
    }

    if (contenttype == "multipart/form-data")
    {
        unsigned long long tetime = time((time_t *)NULL);
        srand(tetime);
        unsigned long long randnum = rand() % 100000 + 100000;
        boundary                   = "----------httpclient21198" + std::to_string(tetime) + std::to_string(randnum);

        header["Content-Type"] = (contenttype + "; boundary=" + boundary);
    }
    else
    {

        header["Content-Type"] = contenttype;
    }

    unsigned int beginpos      = 0;
    unsigned int contentlength = 0;
    std::string tempstr;
    upload_file ptemp;
    // FILE *fp;
    switch (contenttype.length())
    {
    case 33:
        beginpos      = 0;
        contentlength = 0;
        ptemp.error   = 3;
        ptemp.tempfile.clear();
        ptemp.filename.clear();
        ptemp.name.clear();
        ptemp.size = 0;
        ptemp.type.clear();

        for (auto [first, second] : data.as_array())
        {

            // ptemp.error=3;
            // ptemp.tempfile.clear();
            // ptemp.filename.clear();
            // ptemp.name.clear();
            // ptemp.size=0;
            // ptemp.type.clear();
            if (beginpos > 0)
            {
                ptemp.tempfile.push_back('&');
            }
            tempstr = data.get_keyname(first);
            tempstr = url_encode(tempstr.data(), tempstr.size());
            // ptemp.name=tempstr;
            ptemp.tempfile.append(tempstr);
            tempstr = second.to_string();
            tempstr = url_encode(tempstr.data(), tempstr.size());
            ptemp.tempfile.push_back('=');
            ptemp.tempfile.append(tempstr);
            // ptemp.tempfile=tempstr;
            // contentlength+=ptemp.name.size()+1+ptemp.tempfile.size();
            // senddata.push_back(ptemp);
            beginpos++;
        }
        beginpos = 0;

        for (auto [first, second] : parameter.as_array())
        {
            // ptemp.error=3;
            // ptemp.tempfile.clear();
            // ptemp.filename.clear();
            // ptemp.name.clear();
            // ptemp.size=0;
            // ptemp.type.clear();
            // tempstr=parameter.get_keyname(first);
            // tempstr=url_encode(tempstr.data(),tempstr.size());
            // ptemp.name=tempstr;
            // tempstr=second.to_string();
            // tempstr=url_encode(tempstr.data(),tempstr.size());
            // ptemp.tempfile=tempstr;
            // contentlength+=ptemp.name.size()+1+ptemp.tempfile.size();
            // senddata.push_back(ptemp);
            // beginpos++;

            if (beginpos > 0)
            {
                ptemp.tempfile.push_back('&');
            }
            tempstr = parameter.get_keyname(first);
            tempstr = url_encode(tempstr.data(), tempstr.size());

            ptemp.tempfile.append(tempstr);
            tempstr = second.to_string();
            tempstr = url_encode(tempstr.data(), tempstr.size());
            ptemp.tempfile.push_back('=');
            ptemp.tempfile.append(tempstr);
            beginpos++;
        }
        contentlength            = ptemp.tempfile.size();
        header["Content-Length"] = contentlength;
        senddata.push_back(ptemp);

        break;
    case 24:
        if (contenttype == "application/octet-stream")
        {

            if (files.size() > 0)
            {

                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();

                ptemp.size = files.front().size;
                if (ptemp.size < 32768)
                {
                    // fp = fopen(files.front().filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(files.front().filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (fp)
                    {
                        ptemp.tempfile.resize(ptemp.size);
                        unsigned int n = fread(&ptemp.tempfile[0], 1, ptemp.size, fp.get());
                        ptemp.tempfile.resize(n);
                        // fclose(fp);
                    }

                    contentlength = ptemp.tempfile.size();
                }
                else
                {
                    ptemp.error    = 2;
                    ptemp.filename = files.front().filename;
                    contentlength  = ptemp.size;
                }
                senddata.push_back(ptemp);

                header["Content-Length"] = contentlength;
            }
            else
            {

                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();
                if (!data.is_array())
                {
                    ptemp.tempfile = data.to_string();
                }
                else
                {
                    for (auto [first, second] : data.as_array())
                    {
                        if (!second.is_array())
                        {
                            ptemp.tempfile = second.to_string();
                            break;
                        }
                    }
                }

                contentlength            = ptemp.tempfile.size();
                header["Content-Length"] = contentlength;
                senddata.push_back(ptemp);
            }
        }
        break;
    case 19:

        if (contenttype == "multipart/form-data")
        {

            contentlength = 0;
            for (auto [first, second] : data.as_array())
            {

                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();
                ptemp.tempfile.append("--");
                ptemp.tempfile.append(boundary);
                ptemp.tempfile.append("\r\n");

                ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                ptemp.tempfile.append(data.get_keyname(first));
                ptemp.tempfile.append("\"\r\n\r\n");

                if (!second.is_array())
                {
                    if (second.is_num())
                    {
                        ptemp.tempfile.append(second.to_string());
                    }
                    else if (second.is_string())
                    {
                        ptemp.tempfile.append(second.as_string());
                    }
                }
                ptemp.tempfile.append("\r\n");
                contentlength += ptemp.tempfile.size();
                senddata.push_back(ptemp);
            }

            for (auto [first, second] : parameter.as_array())
            {

                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();
                ptemp.tempfile.append("--");
                ptemp.tempfile.append(boundary);
                ptemp.tempfile.append("\r\n");

                ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                ptemp.tempfile.append(parameter.get_keyname(first));
                ptemp.tempfile.append("\"\r\n\r\n");

                if (!second.is_array())
                {
                    if (second.is_num())
                    {
                        ptemp.tempfile.append(second.to_string());
                    }
                    else if (second.is_string())
                    {
                        ptemp.tempfile.append(second.as_string());
                    }
                }
                ptemp.tempfile.append("\r\n");
                contentlength += ptemp.tempfile.size();
                senddata.push_back(ptemp);
            }
            if (files.size() > 0)
            {

                for (auto finfo : files)
                {
                    ptemp.error = 0;
                    ptemp.tempfile.clear();
                    ptemp.filename.clear();
                    ptemp.name.clear();
                    ptemp.size = 0;
                    ptemp.type.clear();
                    if (finfo.size > 16384)
                    {
                        ptemp.error    = 1;
                        ptemp.filename = finfo.filename;
                        ptemp.name     = finfo.name;
                        ptemp.size     = finfo.size;
                        ptemp.type     = finfo.type;

                        ptemp.tempfile.append("--");
                        ptemp.tempfile.append(boundary);
                        ptemp.tempfile.append("\r\n");

                        ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                        ptemp.tempfile.append(finfo.name);
                        ptemp.tempfile.append("\"; filename=\"");
                        ptemp.tempfile.append(finfo.filename);
                        ptemp.tempfile.append("\"\r\nContent-Type: ");
                        ptemp.tempfile.append(finfo.type);
                        ptemp.tempfile.append("\r\n\r\n");
                        contentlength += ptemp.tempfile.size() + finfo.size + 2;
                    }
                    else
                    {
                        ptemp.error    = 0;
                        ptemp.filename = finfo.filename;
                        ptemp.name     = finfo.name;
                        ptemp.size     = finfo.size;
                        ptemp.type     = finfo.type;

                        ptemp.tempfile.append("--");
                        ptemp.tempfile.append(boundary);
                        ptemp.tempfile.append("\r\n");

                        ptemp.tempfile.append("Content-Disposition: form-data; name=\"");
                        ptemp.tempfile.append(finfo.name);
                        ptemp.tempfile.append("\"; filename=\"");
                        ptemp.tempfile.append(finfo.filename);
                        ptemp.tempfile.append("\"\r\nContent-Type: ");
                        ptemp.tempfile.append(finfo.type);
                        ptemp.tempfile.append("\r\n\r\n");
                        // fp = fopen(finfo.filename.c_str(), "rb");
                        std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(finfo.filename.c_str(), "rb"),
                                                                              &std::fclose);
                        if (fp)
                        {
                            unsigned int n = ptemp.tempfile.size();
                            ptemp.tempfile.resize(n + ptemp.size);
                            n = fread(&ptemp.tempfile[n], 1, ptemp.size, fp.get());

                            // fclose(fp);
                        }
                        ptemp.tempfile.append("\r\n");
                        contentlength += ptemp.tempfile.size();
                    }

                    senddata.push_back(ptemp);
                }
            }

            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();

            ptemp.tempfile.append("--");
            ptemp.tempfile.append(boundary);
            ptemp.tempfile.append("--");
            ptemp.size = ptemp.tempfile.size();
            contentlength += ptemp.size;
            senddata.push_back(ptemp);
            header["Content-Length"] = contentlength;
        }
        break;
    case 16:
        if (contenttype == "application/json")
        {

            data.set_array();
            parameter.set_array();

            if (data.size() > 0)
            {

                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();

                ptemp.tempfile.append(data.to_json());

                ptemp.size = ptemp.tempfile.size();
                contentlength += ptemp.size;
                senddata.push_back(ptemp);
            }
            else if (parameter.size() > 0)
            {
                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();

                ptemp.tempfile.append(parameter.to_json());

                ptemp.size = ptemp.tempfile.size();
                contentlength += ptemp.size;
                senddata.push_back(ptemp);
            }
            else if (files.size() > 0)
            {
                ptemp.error = 0;
                ptemp.tempfile.clear();
                ptemp.filename.clear();
                ptemp.name.clear();
                ptemp.size = 0;
                ptemp.type.clear();

                ptemp.size = files.front().size;
                if (ptemp.size < 32768)
                {
                    // fp = fopen(files.front().filename.c_str(), "rb");
                    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(files.front().filename.c_str(), "rb"),
                                                                          &std::fclose);
                    if (fp)
                    {
                        ptemp.tempfile.resize(ptemp.size);
                        unsigned int n = fread(&ptemp.tempfile[0], 1, ptemp.size, fp.get());
                        ptemp.tempfile.resize(n);
                        // fclose(fp);
                    }
                    contentlength = ptemp.tempfile.size();
                }
                else
                {
                    ptemp.error    = 2;
                    ptemp.filename = files.front().filename;
                    contentlength  = ptemp.size;
                }
                senddata.push_back(ptemp);
            }
            header["Content-Length"] = contentlength;
        }

        break;
    default:
        if (files.size() > 0)
        {

            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();

            ptemp.size = files.front().size;
            if (ptemp.size < 32768)
            {
                // fp = fopen(files.front().filename.c_str(), "rb");
                std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(fopen(files.front().filename.c_str(), "rb"),
                                                                      &std::fclose);
                if (fp)
                {
                    ptemp.tempfile.resize(ptemp.size);
                    unsigned int n = fread(&ptemp.tempfile[0], 1, ptemp.size, fp.get());
                    ptemp.tempfile.resize(n);
                    // fclose(fp);
                }
                contentlength = ptemp.tempfile.size();
            }
            else
            {
                ptemp.error    = 2;
                ptemp.filename = files.front().filename;
                contentlength  = ptemp.size;
            }
            senddata.push_back(ptemp);

            header["Content-Length"] = contentlength;
        }
        else
        {
            ptemp.error = 0;
            ptemp.tempfile.clear();
            ptemp.filename.clear();
            ptemp.name.clear();
            ptemp.size = 0;
            ptemp.type.clear();
            if (!data.is_array())
            {
                ptemp.tempfile = data.to_string();
            }
            else
            {
                for (auto [first, second] : data.as_array())
                {
                    if (!second.is_array())
                    {
                        ptemp.tempfile = second.to_string();
                        break;
                    }
                }
            }

            contentlength = ptemp.tempfile.size();

            header["Content-Length"] = contentlength;
            senddata.push_back(ptemp);
        }

        ;
    }
}
}// namespace http