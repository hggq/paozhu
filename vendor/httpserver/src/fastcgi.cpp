/**
 *  @copyright copyright 2023, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file fastcgi.cpp
 *  @date 2023-09-28
 *
 *
 */
#include <cstdarg>
#include <cstring>
#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include "client_context.h"
#include "fastcgi.h"
#include "func.h"
#include "serverconfig.h"
namespace http
{
void fastcgi::make_Header(FASTCGI_Header &header_temp, int type, int request_id, int contentLength, int paddingLength)
{
    header_temp.version = FASTCGI_VERSION;
    header_temp.type    = (unsigned char)type;

    header_temp.requestIdB1 = (unsigned char)((request_id >> 8) & 0xff);
    header_temp.requestIdB0 = (unsigned char)(request_id & 0xff);

    header_temp.contentLengthB1 = (unsigned char)((contentLength >> 8) & 0xff);
    header_temp.contentLengthB0 = (unsigned char)((contentLength & 0xff));

    header_temp.paddingLength = (unsigned char)paddingLength;
    header_temp.reserved      = 0;
}
void fastcgi::make_BeginRequestBody(FASTCGI_BeginRequestBody &body_temp, int role, int keepConnection)
{

    body_temp.roleB1 = (unsigned char)((role >> 8) & 0xff);
    body_temp.roleB0 = (unsigned char)(role & 0xff);

    body_temp.flags = (unsigned char)((keepConnection > 0 ? FASTCGI_KEEP_CONN : 0));
    memset(body_temp.reserved, 0, sizeof(body_temp.reserved));
}
asio::awaitable<bool> fastcgi::co_init_http_sock()
{
    error_msg.clear();
    auto executor                  = co_await asio::this_coro::executor;
    constexpr auto tuple_awaitable = asio::as_tuple(asio::use_awaitable);
    asio::error_code ec;
    //.sock
    if (host.size() > 0 && host.back() == 'k')
    {
        issock    = true;
        socklocal = std::make_shared<asio::local::stream_protocol::socket>(executor);
        socklocal->connect(asio::local::stream_protocol::endpoint(host.c_str()));
    }
    else
    {
        asio::ip::tcp::resolver resolver(executor);
        asio::ip::tcp::resolver::iterator iter = co_await resolver.async_resolve(host, port, asio::use_awaitable);
        asio::ip::tcp::resolver::iterator end;
        asio::ip::tcp::endpoint endpoint;
        sock = std::make_shared<asio::ip::tcp::socket>(executor);

        if (iter == end)
        {
            error_msg = "resolver " + host + port;
            DEBUG_LOG("%s", error_msg.c_str());
            co_return false;
        }

        while (iter != end)
        {
            endpoint     = *iter++;
            std::tie(ec) = co_await sock->async_connect(endpoint, tuple_awaitable);
            if (ec)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if (ec)
    {
        error_msg = "Unable to connect\r\n";
        DEBUG_LOG("%s", error_msg.c_str());
        co_return false;
    }
    co_return true;
}

asio::awaitable<void> fastcgi::co_send()
{
    if (sock == nullptr && socklocal == nullptr)
    {
        DEBUG_LOG("co_await co_init_http_sock");
        bool isinit = co_await co_init_http_sock();
        DEBUG_LOG("co_init_http_sock: %s", error_msg.c_str());
        if (!isinit)
        {
            add_error_msg(error_msg);
            co_return;
        }
    }
    DEBUG_LOG("co_init_http_sock");
    unsigned int ret;
    std::shared_ptr<httppeer> peer = peer_ptr.lock();

    if (!peer)
    {
        add_error_msg("fastcgi::co_send httppeer empty;\r\n");
        co_return;
    }

    std::string params_body;
    std::string respbody;
    serverconfig &sysconfigpath = getserversysconfig();
    co_await send_startheader();

    std::vector<std::pair<std::string, std::string>> params;
    document_root = sysconfigpath.sitehostinfos[peer->host_index].php_root_document.size() > 0 ? sysconfigpath.sitehostinfos[peer->host_index].php_root_document : sysconfigpath.sitehostinfos[peer->host_index].wwwpath;
    if (document_root.size() > 0 && document_root.back() == '/')
    {
        document_root.resize(document_root.size() - 1);
    }
    request_url.clear();
    if (peer->linktype == 12)
    {
        request_url.push_back('/');
        request_url.append("index.php");
        params.push_back({"SCRIPT_NAME", request_url});
        params.push_back({"SCRIPT_FILENAME", (document_root + request_url)});
        DEBUG_LOG("request_url: %s", request_url.c_str());
    }
    else if (peer->linktype == 15)
    {
        request_url.push_back('/');
        request_url.append(peer->sendfilename);
        params.push_back({"SCRIPT_NAME", request_url});
        params.push_back({"SCRIPT_FILENAME", (document_root + request_url)});
        DEBUG_LOG("request_url: %s", request_url.c_str());
    }
    else if (peer->linktype == 19)
    {
        respbody.clear();
        request_url.push_back('/');
        request_url.append(sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists[0].second);

        params.push_back({"SCRIPT_NAME", request_url});
        respbody = sysconfigpath.sitehostinfos[peer->host_index].php_root_document + sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists[0].second;
        params.push_back({"SCRIPT_FILENAME", respbody});

        request_url.clear();
        request_url.push_back('/');
        request_url.append(peer->sendfilename);
        params.push_back({"PATH_INFO", request_url});
    }
    else if (peer->linktype > 19 && peer->linktype < 40)
    {
        ret = peer->linktype - 20;
        if (ret >= sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists.size())
        {
            ret = 0;
        }
        respbody.clear();
        request_url.push_back('/');
        request_url.append(sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists[ret].second);

        params.push_back({"SCRIPT_NAME", request_url});
        respbody = sysconfigpath.sitehostinfos[peer->host_index].php_root_document;
        respbody.append(request_url.substr(1));
        params.push_back({"SCRIPT_FILENAME", respbody});
        if (sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists[ret].first.size() < peer->sendfilename.size())
        {
            params.push_back({"PATH_INFO", peer->sendfilename.substr(sysconfigpath.sitehostinfos[peer->host_index].rewrite_php_lists[ret].first.size())});
        }

        request_url.clear();
        request_url.push_back('/');
        request_url.append(peer->sendfilename);
        params.push_back({"REDIRECT_URL", request_url});
    }
    else if (peer->linktype > 49)
    {
        respbody.clear();
        ret = peer->linktype - 50;
        for (unsigned int i = 0; i < peer->pathinfos.size(); i++)
        {
            request_url.append("/");
            request_url.append(peer->pathinfos[i]);
            if (i <= ret)
            {
                respbody.append("/");
                respbody.append(peer->pathinfos[i]);
            }
        }
        params.push_back({"SCRIPT_NAME", respbody});
        params.push_back({"SCRIPT_FILENAME", (document_root + respbody)});
        respbody.clear();

        for (unsigned int i = ret + 1; i < peer->pathinfos.size(); i++)
        {
            respbody.append("/");
            respbody.append(peer->pathinfos[i]);
        }
        params.push_back({"PATH_INFO", respbody});

        DEBUG_LOG("SCRIPT_NAME: %s", request_url.c_str());
    }
    else
    {
        request_url = peer->urlpath;
        params.push_back({"SCRIPT_NAME", request_url});
        params.push_back({"SCRIPT_FILENAME", (document_root + request_url)});
    }

    params.push_back({"DOCUMENT_ROOT", document_root});
    params.push_back({"GATEWAY_INTERFACE", "FastCGI/1.0"});
    params.push_back({"SERVER_SOFTWARE", "pz"});
    params.push_back({"REQUEST_URI", request_url});
    params.push_back({"QUERY_STRING", peer->querystring});
    params.push_back({"SERVER_NAME", peer->host});

    if (peer->method == 2)
    {
        params.push_back({"REQUEST_METHOD", "POST"});
        params.push_back({"CONTENT_LENGTH", std::to_string(peer->output.size())});
        params.push_back({"CONTENT_TYPE", peer->header["Content-Type"]});
        DEBUG_LOG("POST size: %zu", peer->output.size());
    }
    else if (peer->method == 3)
    {
        params.push_back({"REQUEST_METHOD", "OPTIONS"});
    }
    else
    {
        params.push_back({"REQUEST_METHOD", "GET"});
    }

    if (peer->httpv == 2)
    {
        params.push_back({"SERVER_PROTOCOL", "HTTP/2"});
    }
    else
    {
        params.push_back({"SERVER_PROTOCOL", "HTTP/1.1"});
    }

    if (peer->isssl)
    {
        params.push_back({"REQUEST_SCHEME", "https"});
    }
    else
    {
        params.push_back({"REQUEST_SCHEME", "http"});
    }
    params.push_back({"SERVER_ADDR", peer->server_ip});
    params.push_back({"SERVER_PORT", std::to_string(peer->server_port)});
    params.push_back({"REMOTE_ADDR", peer->client_ip});
    params.push_back({"REMOTE_PORT", std::to_string(peer->client_port)});

    for (const auto &[first, second] : peer->header)
    {
        DEBUG_LOG("%s: %s", first.c_str(), second.c_str());
        if (first.size() == 0)
        {
            continue;
        }
        respbody.clear();
        if (first[0] == ':')
        {
            respbody.append(first);
            std::transform(respbody.begin(), respbody.end(), respbody.begin(), ::tolower);
        }
        else
        {
            respbody.append("HTTP_");
            respbody.append(first);
            std::transform(respbody.begin(), respbody.end(), respbody.begin(), ::toupper);
        }

        params_body.append(pack_length(respbody.size(), second.size()));
        params_body.append(respbody);
        params_body.append(second);
    }

    for (unsigned i = 0; i < params.size(); i++)
    {
        DEBUG_LOG("%s: %s", params[i].first.c_str(), params[i].second.c_str());
        params_body.append(pack_length(params[i].first.size(), params[i].second.size()));
        params_body.append(params[i].first);
        params_body.append(params[i].second);
    }

    unsigned int params_size = params_body.size();

    FASTCGI_Header paramsHeader{};

    paramsHeader.version     = FASTCGI_VERSION;
    paramsHeader.type        = FASTCGI_PARAMS;
    paramsHeader.requestIdB1 = (request_id >> 8) & 0xFF;
    paramsHeader.requestIdB0 = request_id & 0xFF;

    paramsHeader.contentLengthB1 = (params_size >> 8) & 0xFF;
    paramsHeader.contentLengthB0 = params_size & 0xFF;

    paramsHeader.paddingLength = 0;
    paramsHeader.reserved      = 0;

    if (issock)
    {
        ret = co_await async_write(*socklocal, asio::buffer((char *)&paramsHeader, sizeof(paramsHeader)), asio::use_awaitable);
        ret = co_await async_write(*socklocal, asio::buffer(params_body), asio::use_awaitable);
    }
    else
    {
        ret = co_await async_write(*sock, asio::buffer((char *)&paramsHeader, sizeof(paramsHeader)), asio::use_awaitable);
        ret = co_await async_write(*sock, asio::buffer(params_body), asio::use_awaitable);
    }

    co_await send_endheader();
    if (peer->method == 2)
    {
        co_await send_postbody(peer);
    }
    peer->output.clear();
    FASTCGI_Header respHeader{};

    unsigned int contentLen;
    char paddingBuf[8];
    bool isbodycontent            = false;
    unsigned char headerend_count = 0;
    respbody.clear();
    params_body.clear();
    try
    {
        while (true)
        {
            ret = 0;
            if (issock)
            {
                ret = co_await socklocal->async_read_some(asio::buffer(&respHeader, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
            }
            else
            {
                ret = co_await sock->async_read_some(asio::buffer(&respHeader, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
            }
            if (ret == 0)
            {
                break;
            }
            if (respHeader.type == FASTCGI_STDOUT)
            {
                contentLen = (respHeader.contentLengthB1 << 8) + (respHeader.contentLengthB0);
                params_body.clear();
                DEBUG_LOG("FASTCGI_STDOUT %u", contentLen);
                if (issock)
                {
                    std::string temp_read_some;
                    temp_read_some.resize(contentLen);
                    //params_body.reserve(contentLen);
                    for (unsigned int offset = 0; offset < contentLen;)
                    {
                        unsigned int n_read_num = contentLen - offset;
                        ret                     = co_await socklocal->async_read_some(asio::buffer(&temp_read_some[0], n_read_num), asio::use_awaitable);
                        temp_read_some.resize(ret);
                        if (ret == contentLen)
                        {
                            params_body = std::move(temp_read_some);
                            break;
                        }
                        if (offset == 0)
                        {
                            //may be need read multiple times
                            params_body.reserve(contentLen);
                        }
                        offset = offset + ret;
                        params_body.append(temp_read_some);
                        if (offset >= contentLen)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    std::string temp_read_some;
                    temp_read_some.resize(contentLen);
                    //params_body.reserve(contentLen);
                    for (unsigned int offset = 0; offset < contentLen;)
                    {
                        unsigned int n_read_num = contentLen - offset;
                        ret                     = co_await sock->async_read_some(asio::buffer(&temp_read_some[0], n_read_num), asio::use_awaitable);
                        temp_read_some.resize(ret);
                        if (ret == contentLen)
                        {
                            params_body = std::move(temp_read_some);
                            break;
                        }
                        if (offset == 0)
                        {
                            //may be need read multiple times
                            params_body.reserve(contentLen);
                        }
                        offset = offset + ret;
                        params_body.append(temp_read_some);
                        if (offset >= contentLen)
                        {
                            break;
                        }
                    }
                    // params_body.resize(contentLen);
                    // ret = co_await sock->async_read_some(asio::buffer(&params_body[0], params_body.size()), asio::use_awaitable);
                    // params_body.resize(ret);
                }

                if (respHeader.paddingLength > 0)
                {
                    if (issock)
                    {
                        ret = co_await socklocal->async_read_some(asio::buffer(paddingBuf, respHeader.paddingLength), asio::use_awaitable);
                    }
                    else
                    {
                        ret = co_await sock->async_read_some(asio::buffer(paddingBuf, respHeader.paddingLength), asio::use_awaitable);
                    }
                }

                //if body content
                if (isbodycontent)
                {
                    peer->output.append(params_body);
                }
                else
                {
                    ret = 0;
                    for (; ret < params_body.size(); ret++)
                    {
                        if ((params_body.size() > (ret + 2)) && params_body[ret] == '\r' && params_body[ret + 1] == '\n' && params_body[ret + 2] == '\r' && params_body[ret + 3] == '\n')
                        {
                            ret += 4;
                            isbodycontent   = true;
                            headerend_count = 0;
                            respbody.append("\r\n\r\n");
                            break;
                        }
                        else if (headerend_count > 0)
                        {
                            //find header end offset
                            for (; ret < params_body.size(); ret++)
                            {
                                respbody.push_back(params_body[ret]);
                                if (params_body[ret] == '\r' || params_body[ret] == '\n')
                                {
                                    headerend_count += 1;
                                }
                                else
                                {
                                    break;
                                }
                                if (headerend_count == 4)
                                {
                                    break;
                                }
                            }
                            if (headerend_count == 4)
                            {
                                ret += 1;
                                isbodycontent   = true;
                                headerend_count = 0;
                                break;
                            }
                            headerend_count = 0;
                            continue;
                        }
                        else if (params_body[ret] == '\r')
                        {
                            headerend_count += 1;
                            unsigned int j = ret + 1;
                            respbody.push_back(params_body[ret]);
                            //find header end offset, may be broken
                            if (j < params_body.size() && params_body[j] == '\n')
                            {
                                headerend_count += 1;
                                ret += 1;
                                j = ret + 1;
                                respbody.push_back(params_body[ret]);

                                if (j < params_body.size() && params_body[j] == '\r')
                                {
                                    headerend_count += 1;
                                    j = ret + 1;
                                    respbody.push_back(params_body[ret]);
                                }
                            }
                            if (j != params_body.size())
                            {
                                headerend_count = 0;
                            }
                            continue;
                        }

                        respbody.push_back(params_body[ret]);
                    }
                    if (ret < params_body.size())
                    {
                        peer->output = params_body.substr(ret);
                    }
                }
            }
            else if (respHeader.type == FASTCGI_STDERR)
            {
                contentLen = (respHeader.contentLengthB1 << 8) + (respHeader.contentLengthB0);
                DEBUG_LOG("FASTCGI_STDERR %u", contentLen);
                params_body.clear();
                params_body.resize(contentLen);
                if (issock)
                {
                    ret = co_await socklocal->async_read_some(asio::buffer(&params_body[0], params_body.size()), asio::use_awaitable);
                }
                else
                {
                    ret = co_await sock->async_read_some(asio::buffer(&params_body[0], params_body.size()), asio::use_awaitable);
                }
                params_body.resize(ret);
                if (respHeader.paddingLength > 0)
                {
                    if (issock)
                    {
                        ret = co_await socklocal->async_read_some(asio::buffer(paddingBuf, respHeader.paddingLength), asio::use_awaitable);
                    }
                    else
                    {
                        ret = co_await sock->async_read_some(asio::buffer(paddingBuf, respHeader.paddingLength), asio::use_awaitable);
                    }
                }
                error_msg.append(params_body);
                error_msg.append("\r\n");
            }
            else if (respHeader.type == FASTCGI_END_REQUEST)
            {
                FASTCGI_EndRequestBody endRequestBody{};
                if (issock)
                {
                    ret = co_await socklocal->async_read_some(asio::buffer(&endRequestBody, sizeof(endRequestBody)), asio::use_awaitable);
                }
                else
                {
                    ret = co_await sock->async_read_some(asio::buffer(&endRequestBody, sizeof(endRequestBody)), asio::use_awaitable);
                }
                break;
            }
        }
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
        DEBUG_LOG("std::exception %s", e.what());
        peer->output = params_body;
    }
    DEBUG_LOG("php fastcgi return %zu", respbody.size());
    ret = 0;
    std::string line_header;
    std::string line_name;
    bool is_headervalue = true;
    for (; ret < respbody.size(); ret++)
    {
        if ((ret + 2) < respbody.size() && respbody[ret] == '\r' && respbody[ret + 1] == '\n' && respbody[ret + 2] == '\r' && respbody[ret + 3] == '\n')
        {
            ret += 4;
            DEBUG_LOG("%s:s%s", line_name.c_str(), line_header.c_str());
            if (line_name.size() == 12 && str_casecmp(line_name, "Content-type"))
            {
                peer->type(line_header);
            }
            else if (line_name.size() == 6 && str_casecmp(line_name, "Status"))
            {
                peer->status(str2int(&line_header[0], line_header.size()));
            }
            else if (line_name.size() == 10 && str_casecmp(line_name, "Set-Cookie"))
            {
                peer->send_cookie_lists.emplace_back(line_header);
            }
            else
            {
                peer->set_header(line_name, line_header);
            }
            line_header.clear();
            line_name.clear();
            is_headervalue = true;
            break;
        }
        else if (respbody[ret] == '\r' && respbody[ret + 1] == '\n')
        {
            ret += 1;
            DEBUG_LOG("%s:s%s", line_name.c_str(), line_header.c_str());
            if (line_name.size() == 12 && str_casecmp(line_name, "Content-type"))
            {
                peer->type(line_header);
            }
            else if (line_name.size() == 6 && str_casecmp(line_name, "Status"))
            {
                peer->status(str2int(&line_header[0], line_header.size()));
            }
            else if (line_name.size() == 10 && str_casecmp(line_name, "Set-Cookie"))
            {
                peer->send_cookie_lists.emplace_back(line_header);
            }
            else
            {
                peer->set_header(line_name, line_header);
            }
            line_header.clear();
            line_name.clear();
            is_headervalue = true;
            continue;
        }

        if (is_headervalue && respbody[ret] == ':')
        {
            line_name = line_header;
            line_header.clear();
            ret += 1;
            is_headervalue = false;
            for (; ret < respbody.size(); ret++)
            {
                if (respbody[ret] == ' ')
                {
                    continue;
                }
                break;
            }
        }
        line_header.push_back(respbody[ret]);
    }

    //peer->output = respbody.substr(ret);
    co_await send_exit(peer);
    add_error_msg(error_msg);
    co_return;
}
asio::awaitable<void> fastcgi::send_exit(std::shared_ptr<httppeer> peer)
{
    auto ex = asio::get_associated_executor(peer->user_code_handler_call.front());
    asio::dispatch(ex,
                   [handler = std::move(peer->user_code_handler_call.front())]() mutable -> void
                   {
                       /////////////
                       handler(1);
                       //////////
                   });
    peer->user_code_handler_call.pop_front();
    peer->compress = 0;
    peer->linktype = 0;
    co_return;
}
asio::awaitable<void> fastcgi::send_postbody(std::shared_ptr<httppeer> peer)
{
    DEBUG_LOG("bodypost size: %zu", peer->output.size());
    unsigned int bodylen = 0;
    unsigned int len = peer->output.size(), bodyoffset = 0;
    size_t ret;
    try
    {
        while (len > 0)
        {
            if (len > FASTCGI_MAX_LENGTH)
            {
                bodylen = FASTCGI_MAX_LENGTH;
            }
            else
            {
                bodylen = len;
            }

            FASTCGI_Header header_temp_body;
            make_Header(header_temp_body, FASTCGI_STDIN, request_id, bodylen, 0);
            if (issock)
            {
                ret = co_await async_write(*socklocal, asio::buffer((char *)&header_temp_body, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
            }
            else
            {
                ret = co_await async_write(*sock, asio::buffer((char *)&header_temp_body, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
            }
            if (ret != FASTCGI_HEADER_LENGTH)
            {
                co_return;
            }

            if (issock)
            {
                ret = co_await async_write(*socklocal, asio::buffer((char *)&peer->output[bodyoffset], bodylen), asio::use_awaitable);
            }
            else
            {
                ret = co_await async_write(*sock, asio::buffer((char *)&peer->output[bodyoffset], bodylen), asio::use_awaitable);
            }

            if (ret != bodylen)
            {
                co_return;
            }

            len -= bodylen;
            bodyoffset += bodylen;
        }

        FASTCGI_Header header_temp_body;
        make_Header(header_temp_body, FASTCGI_STDIN, request_id, 0, 0);
        if (issock)
        {
            ret = co_await async_write(*socklocal, asio::buffer((char *)&header_temp_body, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
        }
        else
        {
            ret = co_await async_write(*sock, asio::buffer((char *)&header_temp_body, FASTCGI_HEADER_LENGTH), asio::use_awaitable);
        }
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
    }
    co_return;
}
asio::awaitable<void> fastcgi::send_startheader()
{
    try
    {
        FASTCGI_BeginRequestRecord beginRequestRecord{};
        make_Header(beginRequestRecord.header, FASTCGI_BEGIN_REQUEST, request_id, sizeof(beginRequestRecord.body), 0);
        make_BeginRequestBody(beginRequestRecord.body, FASTCGI_RESPONDER, 0);
        if (issock)
        {
            co_await async_write(*socklocal, asio::buffer((char *)&beginRequestRecord, sizeof(beginRequestRecord)), asio::use_awaitable);
        }
        else
        {
            co_await async_write(*sock, asio::buffer((char *)&beginRequestRecord, sizeof(beginRequestRecord)), asio::use_awaitable);
        }
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
    }
    co_return;
}
asio::awaitable<void> fastcgi::send_endheader()
{
    try
    {
        FASTCGI_Header endHeader{};
        make_Header(endHeader, FASTCGI_PARAMS, request_id, 0, 0);
        if (issock)
        {
            co_await async_write(*socklocal, asio::buffer((char *)&endHeader, sizeof(endHeader)), asio::use_awaitable);
        }
        else
        {
            co_await async_write(*sock, asio::buffer((char *)&endHeader, sizeof(endHeader)), asio::use_awaitable);
        }
    }
    catch (std::exception &e)
    {
        error_msg.append(e.what());
    }
    co_return;
}
std::string fastcgi::pack_length(unsigned int a, unsigned int b)
{
    std::string temp;
    if (a < 128)
    {
        temp.push_back((unsigned char)(a & 0xFF));
    }
    else
    {
        /* nameLen 用 4 个字节保存 */
        temp.push_back((unsigned char)(((a >> 24) & 0xFF) | 0x80));
        temp.push_back((unsigned char)((a >> 16) & 0xFF));
        temp.push_back((unsigned char)((a >> 8) & 0xFF));
        temp.push_back((unsigned char)(a & 0xFF));
    }

    if (b < 128)
    {
        temp.push_back((unsigned char)(b & 0xFF));
    }
    else
    {
        /* nameLen 用 4 个字节保存 */
        temp.push_back((unsigned char)(((b >> 24) & 0xFF) | 0x80));
        temp.push_back((unsigned char)((b >> 16) & 0xFF));
        temp.push_back((unsigned char)((b >> 8) & 0xFF));
        temp.push_back((unsigned char)(b & 0xFF));
    }
    return temp;
}

}// namespace http