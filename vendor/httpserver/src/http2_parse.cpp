/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http2_parse.cpp
 *  @date 2022-10-12
 *
 *  http2 protocol parse file
 *
 *
 */
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

#include <string_view>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstring>

#include "http2_frame.h"
#include "http2_parse.h"
#include "client_session.h"
#include "http2_huffman.h"

#include "terminal_color.h"
#include "debug_log.h"
#include "server_localvar.h"
#include "urlcode.h"
#include "request.h"
#include "func.h"
#include "cost_define.h"
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

namespace http
{
// 连接级错误：直接 GOAWAY 断连（RFC 9113 §5.4.1）。
// 例：在 idle / 偶数 / 未知流上收到 DATA（PROTOCOL_ERROR）。
void http2parse::set_conn_error(unsigned int code, unsigned int h2)
{
    error           = code;
    error_is_conn   = true;
    error_stream_id = 0;
    h2_error_code   = h2;
}

// 流级错误：只 RST_STREAM 重置该流，保留连接（RFC 9113 §5.4.2）。
// 例：在已 open / half-closed 的流上又收到 DATA（STREAM_CLOSED）。
void http2parse::set_stream_error(unsigned int code, unsigned int sid, unsigned int h2)
{
    error           = code;
    error_is_conn   = false;
    error_stream_id = sid;
    h2_error_code   = h2;
}

void http2parse::setsession(std::shared_ptr<client_session> peer_sock) { peer_session = peer_sock; }

// multipart 的 boundary 归一化：去引号、限长。
// 与 HTTP/1 (http_parse.cpp 的 boundary_normalize) 保持完全一致的口径。
// 但 HTTP/2 的 getcontenttype 当时漏了，这里补上，避免两边再次分叉。
// 放在匿名 namespace 里，既不影响其他翻译单元，也能在 unity 构建下与 HTTP/1 的同名函数共存。
namespace
{
bool http2_boundary_normalize(std::string &out, const std::string &raw)
{
    std::string temp = raw;
    if (temp.size() >= 2 && temp.front() == '"' && temp.back() == '"')
    {
        temp = temp.substr(1, temp.size() - 2);
    }
    if (temp.size() == 0 || temp.size() > 72)
    {
        return false;
    }
    out = temp;
    return true;
}

// HPACK 整数续位解析（RFC 7541 §5.1）
// 调用方已读出前缀 initial_value (= first_byte & prefix_mask)，begin 已指向续位字节位置
// 若 initial_value != prefix_mask → 无前缀续位，直接成功（out_result=initial_value）
// 否则循环读续位字节直到遇到无续位位 或 溢出 或 越界
// 返回 true 成功；失败时 err_code 写入 error_out，begin 前进位置不确定
// h2_error_out 同步写成 COMPRESSION_ERROR(0x9)：整数解不出来是 HPACK 压缩会话的错，
// 不是 §8.1 的语义错，GOAWAY 带 0x1 会让对端按协议违例去处理，重发策略判错方向。
static bool hpack_int_decode(std::string_view data, unsigned int &begin,
                               unsigned int initial_value, int prefix_bits,
                               unsigned int &out_result,
                               unsigned int &error_out, unsigned int &h2_error_out, int err_code)
{
    unsigned int prefix_mask = (1u << prefix_bits) - 1;
    if (initial_value != prefix_mask)
    {
        out_result = initial_value;
        return true;
    }

    unsigned long long acc   = initial_value;
    unsigned long long shift = 0;

    while (true)
    {
        if (begin >= data.size())
        {
            error_out     = err_code;
            h2_error_out  = 0x9;
            return false;
        }
        unsigned char b = (unsigned char)data[begin];

        if (shift > 56)
        {
            error_out     = err_code;
            h2_error_out  = 0x9;
            return false;
        }

        acc += (unsigned long long)(b & 0x7F) * (1ULL << shift);
        shift += 7;
        begin += 1;

        if ((b & 0x80) == 0)
        {
            break;
        }
    }

    if (acc > 0xFFFFFFFFULL)
    {
        error_out     = err_code;
        h2_error_out  = 0x9;
        return false;
    }
    out_result = (unsigned int)acc;
    return true;
}
}// namespace

void http2parse::clsoesend()
{
    for (auto iter = http_data.begin(); iter != http_data.end();)
    {
        if (iter->second->issend)
        {
            try
            {
                iter->second->isclose = true;
                iter->second->clsoesend();
            }
            catch (const std::exception &e)
            {
                DEBUG_LOG("http2parse user_code_handler_call error");
            }
        }
        iter++;
    }
}

void http2parse::readheaders(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    auto iter = http2_header_recvs.find(temp_pack_data.stream_id);
    // 保留位掩码按帧型取，不能共用一张：HEADERS 的合法位是 END_STREAM(0x01)/END_HEADERS(0x04)/
    // PADDED(0x08)/PRIORITY(0x20)，保留面 0xD2；CONTINUATION 只允许 END_HEADERS，保留面 0xFB。
    // 用 0xD2 判 CONTINUATION 时，它带 END_STREAM/PADDED/PRIORITY 会被当成合法帧收下。
    unsigned char reserved_mask = (temp_pack_data.frame_type == 0x09) ? 0xFB : 0xD2;
    if ((temp_pack_data.flags & reserved_mask) != 0)
    {
        //非头部内容
        error = 40015;
        return;
    }

    // ---- 流合法性校验（RFC 9113 §5.1.1 / §6.10）----
    // 必须校验流 id：不校验时客户端可以用偶数 id、0、以及已用过的 id 反复开新流，
    // 只要只发 HEADERS 不结束 body，就不会计入 server.cpp 的 steam_count，
    // http_data / http_post_data / http2_header_recvs 会被无上限撑大。
    if (iter == http2_header_recvs.end())
    {
        if (temp_pack_data.frame_type != 0x01)
        {
            // CONTINUATION 必须紧跟在 HEADERS 之后，不能凭空出现
            error = 40008;
            return;
        }
        if (temp_pack_data.stream_id == 0 || (temp_pack_data.stream_id & 0x1) == 0)
        {
            // 客户端发起的流必须是奇数；流 0 只能用于连接级帧
            error = 40006;
            return;
        }
        if (temp_pack_data.stream_id <= max_client_stream_id)
        {
            // 新流的 id 必须严格递增，重复或倒退的 id 属于协议错误
            error = 40006;
            return;
        }
        if (http_data.size() >= CONST_HTTP2_MAX_STREAMS || http2_header_recvs.size() >= CONST_HTTP2_MAX_STREAMS)
        {
            // 「body 未结束的流」和「头部块未结束的流」都必须计数
            error = 40010;
            return;
        }
        max_client_stream_id = temp_pack_data.stream_id;
    }

    if (iter == http2_header_recvs.end())
    {
        HTTP2_HEADER_FRAME_T temp;
        temp.length     = temp_pack_data.length;
        temp.frame_type = temp_pack_data.frame_type;

        temp.flags.END_STREAM  = temp_pack_data.flags & HTTP2_HEADER_END_STREAM;
        temp.flags.END_HEADERS = temp_pack_data.flags & HTTP2_HEADER_END_HEADERS;
        temp.flags.PADDED      = temp_pack_data.flags & HTTP2_HEADER_PADDED;
        temp.flags.PRIORITY    = temp_pack_data.flags & HTTP2_HEADER_PRIORITY;

        temp.stream_id = temp_pack_data.stream_id;
        temp.content   = temp_pack_data.payload;

        http2_header_recvs.insert_or_assign(temp_pack_data.stream_id, std::move(temp));
        iter = http2_header_recvs.find(temp_pack_data.stream_id);
    }
    else
    {
        if (iter->second.flags.END_HEADERS > 0)
        {
            error = 40008;
            return;
        }
        iter->second.content.append(temp_pack_data.payload);
    }

    if (iter->second.content.size() > CONST_HTTP_HEADER_BODY_SIZE)
    {
        //two check header payload size
        error = 40014;
        return;
    }

    // HEADERS 帧才带 END_STREAM / PADDED / PRIORITY，CONTINUATION 只带 END_HEADERS。
    // 这些位只能在 HEADERS 帧上取；用「当前帧」的 flags 无条件覆盖全部标志时，
    // HEADERS(PADDED|END_STREAM) + CONTINUATION 会把 PADDED/END_STREAM 抹掉，
    // padding 字节被当成头部块正文解析，整块 HPACK 错位。
    if (temp_pack_data.frame_type == 0x01)
    {
        iter->second.flags.END_STREAM = (temp_pack_data.flags & HTTP2_HEADER_END_STREAM) ? 1 : 0;
        iter->second.flags.PRIORITY   = (temp_pack_data.flags & HTTP2_HEADER_PRIORITY) ? 1 : 0;
        iter->second.flags.PADDED     = (temp_pack_data.flags & HTTP2_HEADER_PADDED) ? 1 : 0;
    }
    iter->second.flags.END_HEADERS = (temp_pack_data.flags & HTTP2_HEADER_END_HEADERS) ? 1 : 0;

    if (iter->second.flags.END_HEADERS == 0)
    {
        //未结束
        // header_block_pending = true;  // 已注释：§6.10 头部块中途帧校验暂不做
        return;
    }

    unsigned header_offset = 0;
    if (iter->second.flags.PADDED)
    {
        if (iter->second.content.size() == 0)
        {
            error = 40013;
            return;
        }
        iter->second.padded_length = iter->second.content[header_offset];
        header_offset++;
    }

    if (iter->second.flags.PRIORITY)
    {
        header_offset += 5;
    }

    unsigned int new_size_num = header_offset + iter->second.padded_length;

    if (new_size_num > 0)
    {
        if (new_size_num >= iter->second.content.size())
        {
            error = 40019;
            return;
        }
        new_size_num         = iter->second.content.size() - new_size_num;
        std::string temp_str = iter->second.content.substr(header_offset, new_size_num);
        iter->second.content = temp_str;
    }

    auto steam_httppeer = std::make_shared<httppeer>();
    if (!http_data.emplace(iter->second.stream_id, steam_httppeer).second)
    {
        // 同一流 id 已经建过请求对象：emplace 会静默失败，新 peer 变成没人应答的孤儿，
        // 而 http_data 里留着旧的。这里直接判协议错误（正常情况下前面的递增校验已拦住）。
        error = 40006;
        return;
    }
    steam_httppeer->content_length = 0;
    steam_httppeer->stream_id      = iter->second.stream_id;
    steam_httppeer->httpv          = 2;
    steam_httppeer->socket_session = peer_session;
    // isssl 的唯一事实来源是 socket（h1 侧也是从 socket 取值）。它原先由 :scheme 的头值反推，
    // 于是明文连接上对端发索引 7（:scheme https）就能把 is_ssl() 伪造成 true，TLS 连接上发索引 6
    // 又把真连接写成 false；而字面量形式的 :scheme 压根不落到那行，同一个请求两种编码两个答案。
    steam_httppeer->isssl          = peer_session->isssl;

    headers_parse(iter->second, steam_httppeer);

    // 站点上传限额（upload_max_size）判的是头块里声明的 Content-Length，头块解完就已经够条件判，
    // 不必等正文——超限的请求一帧正文都不该收（h1 也在头完成处调同一个 check_upload_limit()）。
    // 这是站点策略违例，不是协议违例，所以只作废这一条流，连接照常复用；
    // headers_parse 已经报错时（error 非 0）不接管：错误分流得先走，不能一边断连一边回页面。
    // 没声明 Content-Length 的请求这里判不了（body 长度未知），留给正文收尾那条比对。
    if (error == 0 && steam_httppeer->ishas_content_length &&
        (steam_httppeer->method == 2 || steam_httppeer->method == 4))
    {
        unsigned int limit_status = steam_httppeer->check_upload_limit();
        if (limit_status > 0)
        {
            steam_httppeer->reject_status = limit_status;
            steam_httppeer->isfinish      = true;
            // 正文收集状态必须一并撤掉：留着它，随后到达的 DATA 会被当成 body 收下，
            // 收尾那条 content-length 比对就把连接级错误引出来了。
            http_post_data.erase(steam_httppeer->stream_id);
            // HEADERS 自带 END_STREAM 的 POST 没有正文，headers_parse 已经就地派发过一次。
            if (!iter->second.flags.END_STREAM)
            {
                stream_list.emplace(steam_httppeer->stream_id);
            }
        }
    }
    // isuse_fastcgi() 已删除：路由预查挪到 loop 里，php 处理挪到 server 新方法
    http2_header_recvs.erase(iter);
    // header_block_pending = false;  // 已注释：§6.10 头部块中途帧校验暂不做
}
void http2parse::headers_parse(const HTTP2_HEADER_FRAME_T &header_block_obj, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int header_stream_id = steam_httppeer->stream_id;
    for (unsigned int h_begin = 0; h_begin < (unsigned int)header_block_obj.content.size(); h_begin++)
    {
        unsigned char c = header_block_obj.content[h_begin];

        if (c & 0x80)
        {
            headertype1(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xC0) == 0x40)
        {
            headertype2(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xF0) == 0x10)
        {
            headertype3(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xF0) == 0)
        {
            headertype4(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else if ((c & 0xE0) == 0x20)
        {
            // 001xxxxx : Dynamic Table Size Update (RFC 7541 §6.3)
            // 这是独立于 4 种 header 表示的控制指令，不是第 5 种 header 类型
            dynamic_table_size_update(c, header_block_obj.content, h_begin, steam_httppeer);
        }
        else
        {
            error = 40205;
            return;
        }

        if (error > 0)
        {
            return;
        }
    }

    // CORS：OPTIONS 预检的 ACAO 由 send_cors_domain() 按白名单判定输出，
    // 普通请求的 ACAO 在解析到 origin 头那一刻就判定了（头名长度 6 的 'o' 分支），
    // 这里请求头虽已解完，但不再需要任何 CORS 处理

    if (steam_httppeer->method == 2 || steam_httppeer->method == 4)
    {
        DEBUG_LOG("http2 post client: %s %ud", steam_httppeer->url.c_str(), header_stream_id);
        // HEADERS 自带 END_STREAM：客户端这一侧已发完，流进入 half-closed(remote)，
        // 请求体长度就是 0。必须在这里就地派发，而且**绝不能**建 http_post_data：
        // 建了就等于对外宣布"本流还在收请求体"，随后到达的 DATA 会被当成 body 收下并
        // 再触发一次派发，同一个流回两份响应（RFC 9113 §5.1 要求此时回
        // RST_STREAM(STREAM_CLOSED)，h2spec 6.1.2 判失败）；只发 HEADERS(END_STREAM)
        // 的 POST 也会一直等一个永远不会来的 DATA 而静默挂死。
        if (header_block_obj.flags.END_STREAM)
        {
            steam_httppeer->isfinish = true;
            stream_list.emplace(header_stream_id);
            return;
        }

        // 这里不再重置/授予接收窗口：
        //  * 新流的流级额度已由本端广告的 SETTINGS_INITIAL_WINDOW_SIZE 给出，
        //    不需要（也不应该）再补一次；
        //  * 每来一个 POST/PUT 就把整条连接的接收记账重置成窗口目标值、并发出
        //    「连接级 + 流级」两个 WINDOW_UPDATE 的话：既抹掉其他流已消费的额度（并发 POST
        //    会静默挂死），又把对端连接窗口每请求顶高 16MB（约 129 次后超 2^31-1，
        //    对端必须按 FLOW_CONTROL_ERROR 断连）。
        need_wakeup_send_threads = true;

        auto iter = http_post_data.find(header_stream_id);
        if (iter == http_post_data.end())
        {
            HTTP2_POST_DATA_T temp_p;
            temp_p.peer      = steam_httppeer;
            temp_p.stream_id = header_stream_id;
            http_post_data.insert_or_assign(steam_httppeer->stream_id, std::move(temp_p));
        }
    }
    else
    {
        DEBUG_LOG("http2 get client: %s", steam_httppeer->url.c_str());
        steam_httppeer->isfinish = true;
        stream_list.emplace(header_stream_id);
    }

    // ---- RFC 7540 §8.1.2.3 请求伪头跨头块校验（headertypeX 全跑完才知道）----
    // Finalize: duplicate / missing / empty pseudo-headers.
    // 重复伪头 → PROTOCOL_ERROR（h2spec 8.1.2.3 #5/#6/#7）
    if (steam_httppeer->h2_method_dup ||
        steam_httppeer->h2_scheme_dup ||
        steam_httppeer->h2_path_dup)
    {
        set_conn_error(40025, 0x1);
        return;
    }
    // :method / :scheme / :path 缺失 → PROTOCOL_ERROR（h2spec 8.1.2.3 #2/#3/#4）
    // CONNECT 允许没有 :path 和 :scheme（RFC 9113 §8.5），但我们不处理 CONNECT，
    // 所以一律要求这三个伪头必须各出现一次。
    if (!steam_httppeer->h2_method_seen ||
        !steam_httppeer->h2_scheme_seen ||
        !steam_httppeer->h2_path_seen)
    {
        set_conn_error(40025, 0x1);
        return;
    }
    // :path 空值 → PROTOCOL_ERROR（h2spec 8.1.2.3 #1）
    // :path 不能是空串（CONNECT 除外，但我们不处理 CONNECT）
    if (steam_httppeer->urlpath.empty())
    {
        set_conn_error(40025, 0x1);
        return;
    }
}
void http2parse::cookie_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("cookie_process:%s:%s", header_name.c_str(), header_value.c_str());
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    std::string buffer_key;
    std::string buffer_value;

    if (steam_httppeer->header["cookie"].empty())
    {
    }
    else
    {
        steam_httppeer->header["cookie"].append("; ");
    }
    steam_httppeer->header["cookie"].append(header_value);
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3D)
        {
            buffer_key = http::url_decode(buffer_value.data(), buffer_value.length());
            // Safe filtering allowed: A-Z a-z 0-9 _-
            std::string safe_key;
            safe_key.reserve(buffer_key.size());
            for (unsigned char c : buffer_key)
            {
                if ((c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_' || c == '-')
                {
                    safe_key.push_back(static_cast<char>(c));
                }
            }
            buffer_key.swap(safe_key);
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3B)
        {
            buffer_value = http::url_decode(buffer_value.data(), buffer_value.length());
            if (buffer_key.size() > 48)
            {
                error = 40157;
                return;
            }
            if (buffer_key.empty())
            {
                error = 40157;
                return;
            }
            steam_httppeer->cookie[buffer_key] = buffer_value;
            buffer_key.clear();
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        buffer_value = http::url_decode(buffer_value.data(),
                                        buffer_value.length());
        if (buffer_key.size() > 48)
        {
            error = 40158;
            return;
        }
        if (buffer_key.empty())
        {
            error = 40158;
            return;
        }
        steam_httppeer->cookie[buffer_key] = buffer_value;
    }
    else
    {
        if (buffer_key.size() > 48)
        {
            error = 40159;
            return;
        }
        if (buffer_key.size() > 0)
        {
            steam_httppeer->cookie[buffer_key] = "";
        }
        buffer_key.clear();
    }
}
void http2parse::path_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("path_process:%s:%s", header_name.c_str(), header_value.c_str());

    std::string buffer_key;
    std::string buffer_value;
    unsigned char headerstep = 0;
    steam_httppeer->pathinfos.clear();
    steam_httppeer->url.clear();
    unsigned int linesize = static_cast<unsigned int>(header_value.size());
    unsigned int ioffset  = 0;

    // 按原始 '/' 逐段切分，每段交给 http::url_segments_normalize()（func.cpp 统一实现）
    for (; ioffset < linesize; ioffset++)
    {
        if (header_value[ioffset] == 0x3F)// '?'
        {
            headerstep = 6;
            break;
        }
        if (header_value[ioffset] == 0x2F)// '/'
        {
            if (buffer_key.size() > 255)
            {
                error = 40007;
                return;
            }
            if (buffer_key.size() > 0)
            {
                if (!http::url_segments_normalize(steam_httppeer->pathinfos, buffer_key))
                {
                    error = 40095;
                    return;
                }
                buffer_key.clear();
            }
        }
        else
        {
            buffer_key.push_back(header_value[ioffset]);
        }
    }
    if (buffer_key.size() > 0)
    {
        if (buffer_key.size() > 255)
        {
            error = 40090;
            return;
        }
        if (!http::url_segments_normalize(steam_httppeer->pathinfos, buffer_key))
        {
            error = 40095;
            return;
        }
    }

    steam_httppeer->header["urlpath"] = header_value.substr(0, ioffset);

    if (steam_httppeer->pathinfos.size() > 0)
    {
        steam_httppeer->urlpath.clear();
        steam_httppeer->urlpath.reserve(header_value.size() > 64 && header_value.size() < 10000 ? header_value.size() : 0);

        for (size_t nn = 0; nn < steam_httppeer->pathinfos.size(); nn++)
        {
            steam_httppeer->urlpath.push_back('/');
            steam_httppeer->urlpath.append(steam_httppeer->pathinfos[nn]);
        }
        if (steam_httppeer->urlpath.size() == 0)
        {
            steam_httppeer->urlpath = "/";
        }
    }
    else
    {
        steam_httppeer->urlpath.clear();
        steam_httppeer->urlpath = "/";
    }

    // 最后一段含 '.' 标记为文件请求；.php 结尾标 compress=10 交给 loop 处理 —
    // last segment has '.' means file-like URL; .php suffix marks fastcgi candidate
    if (!steam_httppeer->pathinfos.empty())
    {
        auto const &last       = steam_httppeer->pathinfos.back();
        steam_httppeer->isfile = (last.find('.') != std::string::npos);
#ifdef ENABLE_FASTCGI
        // 超轻量：只比后缀 4 字符，不查路由表不查磁盘 —
        // ultra-light: only check suffix, no route lookup no disk stat

        if (last.size() > 4 && last[last.size() - 1] == 'p' && last[last.size() - 2] == 'h' &&
            last[last.size() - 3] == 'p' && last[last.size() - 4] == '.')
        {
            steam_httppeer->compress = 10;
        }
#endif

    }

    steam_httppeer->url = steam_httppeer->urlpath;

    buffer_key.clear();
    if (headerstep == 6)
    {
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] == 0x3F)
            {
                continue;
            }
            break;
        }
        headerstep = 0;
        steam_httppeer->querystring.clear();
        steam_httppeer->header["querystring"].clear();
        steam_httppeer->querystring.append(&header_value[ioffset], (linesize - ioffset));// http::url_decode(&header_value[ioffset], (linesize - ioffset));
        steam_httppeer->header["querystring"].append(steam_httppeer->querystring);
        steam_httppeer->url.push_back(0x3F);
        steam_httppeer->url.append(steam_httppeer->querystring);
        buffer_key.clear();
        buffer_value.clear();
        unsigned int jj = 0;
        for (; ioffset < linesize; ioffset++)
        {
            if (header_value[ioffset] == 0x3D)
            {
                for (; ioffset < linesize; ioffset++)
                {
                    if (header_value[ioffset] == 0x3D)
                    {
                        continue;
                    }
                    else
                    {
                        ioffset -= 1;
                        break;
                    }
                }
                buffer_key = http::url_decode(buffer_value.data(),
                                              buffer_value.length());
                buffer_value.clear();
                headerstep = 1;
                jj         = 0;
                continue;
            }
            else if (header_value[ioffset] == 0x26)
            {
                for (; ioffset < linesize; ioffset++)
                {
                    if (header_value[ioffset] == 0x26)
                    {
                        continue;
                    }
                    else
                    {
                        ioffset -= 1;
                        break;
                    }
                }

                buffer_value =
                    http::url_decode(buffer_value.data(),
                                     buffer_value.length());
                if (buffer_key.size() > 48)
                {
                    error = 40173;
                    return;
                }
                procssparamter(buffer_key, buffer_value, steam_httppeer);
                buffer_key.clear();
                buffer_value.clear();
                headerstep = 2;
                jj         = 0;
                continue;
            }
            buffer_value.push_back(header_value[ioffset]);

            if (headerstep == 0 || headerstep == 2)
            {
                //key name too long
                if (jj > 72)
                {
                    error = 40174;
                    return;
                }
            }
            jj++;
        }

        if (headerstep == 1)
        {
            // full kv
            buffer_value = http::url_decode(buffer_value.data(),
                                            buffer_value.length());
            if (buffer_key.size() > 48)
            {
                error = 40175;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
        else if (headerstep == 2)
        {
            // half k
            buffer_key = http::url_decode(buffer_value.data(),
                                          buffer_value.length());
            buffer_value.clear();
            if (buffer_key.size() > 48)
            {
                error = 40176;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
        else if (buffer_value.size() > 0)
        {
            // only one k
            buffer_key = http::url_decode(buffer_value.data(),
                                          buffer_value.length());
            buffer_value.clear();
            if (buffer_key.size() > 48)
            {
                error = 40177;
                return;
            }
            procssparamter(buffer_key, buffer_value, steam_httppeer);
        }
    }
}
void http2parse::procssparamter(std::string_view buffer_key, std::string_view buffer_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool isgroup = true;
    if (buffer_key.length() > 72)
    {
        error = 40011;
        return;
    }

    for (size_t j = 0; j < buffer_key.length(); j++)
    {
        if (buffer_key[j] == '[')
        {
            isgroup = false;
        }
    }

    if (isgroup)
    {
        steam_httppeer->get[buffer_key] = buffer_value;
        return;
    }

    isgroup = true;
    std::string objname;
    for (size_t j = 0; j < buffer_key.length(); j++)
    {
        if (buffer_key[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < buffer_key.length(); n++)
            {
                if (buffer_key[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (buffer_key[n] == '[')
                {

                    break;
                }
                if (buffer_key[n] != 0x22)
                {
                    key1name.push_back(buffer_key[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < buffer_key.length())
                {
                    if (buffer_key[m] == '[')
                    {
                        m += 1;
                        for (; m < buffer_key.length(); m++)
                        {
                            if (buffer_key[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (buffer_key[m] == '[')
                            {

                                break;
                            }
                            if (buffer_key[m] != 0x22)
                            {
                                key2name.push_back(buffer_key[m]);
                            }
                        }

                        if (ishaskey2 && m == buffer_key.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40095;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->get[objname][key1name].set_object();

                            iii      = steam_httppeer->get[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40096;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40097;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40098;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40099;
                                return;
                            }

                            steam_httppeer->get[objname].set_object();
                            steam_httppeer->get[objname][key1name].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname][key1name].size());
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40100;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40101;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40102;
                                return;
                            }
                            steam_httppeer->get[objname].set_object();
                            steam_httppeer->get[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = buffer_value;

                            steam_httppeer->get[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == buffer_key.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40103;
                            return;
                        }

                        steam_httppeer->get[objname].set_object();
                        unsigned int iii = static_cast<unsigned int>(steam_httppeer->get[objname].size());
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = buffer_value;

                        steam_httppeer->get[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40104;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40105;
                            return;
                        }

                        steam_httppeer->get[objname].set_object();

                        http::obj_val objtemp;
                        objtemp = buffer_value;
                        steam_httppeer->get[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // 没有数组
                }
            }
            if (isgroup)
            {
                objname.push_back(buffer_key[j]);
            }
        }
        else
        {
            objname.push_back(buffer_key[j]);
        }
    }
    if (isgroup)
    {
        steam_httppeer->get[buffer_key] = buffer_value;
    }
}
void http2parse::range_process([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("range_process:%s:%s", header_name.c_str(), header_value.c_str());
    // 语法不合法时按 RFC 9110 §14.2 忽略该头：本端没有「单条请求头错误」通道，
    // 走 error 会把整条连接按协议错误断开（403 + GOAWAY），比忽略严重得多。
    parse_range_header(header_value, steam_httppeer->state);
}

bool http2parse::header_host_process(const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool ishasport = false;
    steam_httppeer->host.clear();
    // 注意：这里必须是 unsigned int。用 unsigned char 做下标时，header_value 长度超过 255
    // 后会从 255 回绕到 0，循环永远不结束，host 字符串被无上限 push_back（远程死循环 + 内存耗尽）。
    unsigned int i = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x3A)
        {
            ishasport = true;
            i++;
            break;
        }
        else if (header_value[i] >= '0' && header_value[i] <= '9')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] >= 'a' && header_value[i] <= 'z')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] >= 'A' && header_value[i] <= 'Z')
        {
            steam_httppeer->host.push_back(header_value[i] + 32);
        }
        else if (header_value[i] == '.' || header_value[i] == '-')
        {
            steam_httppeer->host.push_back(header_value[i]);
        }
        else if (header_value[i] == '[')
        {
            if (steam_httppeer->host.size() > 0)
            {
                error = 40160;
                return false;
            }

            i++;
            for (; i < header_value.size(); i++)
            {
                if (header_value[i] >= '0' && header_value[i] <= '9')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] >= 'a' && header_value[i] <= 'f')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] >= 'A' && header_value[i] <= 'F')
                {
                    steam_httppeer->host.push_back(header_value[i] + 32);
                }
                else if (header_value[i] == ':')
                {
                    steam_httppeer->host.push_back(header_value[i]);
                }
                else if (header_value[i] == ']')
                {
                    i++;
                    if (i < header_value.size() && header_value[i] == 0x3A)
                    {
                        ishasport = true;
                        i++;
                        break;
                    }
                    break;
                }
                else
                {
                    error = 40161;
                    return false;
                }
            }
        }
        else
        {
            error = 40162;
            return false;
        }
    }

    unsigned int port_temp = 0;
    if (ishasport)
    {
        for (; i < header_value.size(); i++)
        {
            if (header_value[i] < 0x3A && header_value[i] > 0x2F)
            {
                port_temp = port_temp * 10 + (header_value[i] - 0x30);
                if (port_temp > 65535)
                {
                    break;
                }
            }
        }
    }
    // state.port = port;
    steam_httppeer->state.port = port_temp;
    return true;
}

void http2parse::header_process(std::string header_name, std::string header_value, int table_num, std::shared_ptr<httppeer> steam_httppeer)
{
    DEBUG_LOG("header:%s:%s|%d", header_name.c_str(), header_value.c_str(), table_num);

    // ---- RFC 7540 §8.1.2 单元素头合规校验（每条头一解码完立刻能判，不跨状态）----
    // All static-table entries (table_num > 0) are HPACK-defined lowercase headers,
    // so these guards apply universally — a tainted encoder would have to use the
    // literal-insert paths (table_num == 0) to smuggle them in.
    //
    // ① 头名里有任何大写 → PROTOCOL_ERROR（h2spec 8.1.2 #1）。HTTP/2 头名必须全小写。
    // Header names MUST be lowercase per RFC 7540 §8.1.2.
    for (unsigned char c : header_name)
    {
        if (c >= 'A' && c <= 'Z')
        {
            set_conn_error(40025, 0x1);  // PROTOCOL_ERROR
            return;
        }
    }
    // ② 响应伪头（:status）出现在请求里、未知伪头 → PROTOCOL_ERROR（h2spec 8.1.2.1 #1/#2）
    // Pseudo-headers are request-only (:method/:scheme/:path/:authority / response-only :status).
    if (!header_name.empty() && header_name[0] == ':')
    {
        static constexpr const char *const k_req_pseudos[] = {
            ":method", ":scheme", ":path", ":authority", ":protocol"
        };
        bool known = false;
        for (const char *p : k_req_pseudos)
        {
            if (header_name == p)
            {
                known = true;
                break;
            }
        }
        if (!known)
        {
            set_conn_error(40025, 0x1);  // PROTOCOL_ERROR — 未知伪头 / 响应伪头 :status
            return;
        }
    }
    // ③ 连接层头（RFC 7540 §8.1.2.2）— HTTP/1.1 专用，HTTP/2 必须拆帧走不同机制
    // Connection-specific headers are HTTP/1.1 only; HTTP/2 has distinct frames.
    static constexpr const char *const k_conn_hop_by_hop[] = {
        "connection", "keep-alive", "proxy-connection", "te", "transfer-encoding", "upgrade"
    };
    for (const char *h : k_conn_hop_by_hop)
    {
        if (header_name == h)
        {
            // TE 例外：值为 trailers 是 RFC 7540 §8.1.2.2 允许的，其它值是 PROTOCOL_ERROR
            // TE is allowed only with value "trailers" (RFC 7540 §8.1.2.2).
            if (header_name == "te" && header_value == "trailers")
            {
                break;
            }
            set_conn_error(40025, 0x1);  // PROTOCOL_ERROR
            return;
        }
    }

    // ---- RFC 7540 §8.1.2 跨状态计数（单元素校验已过，这里只计状态）----
    // 伪头在普通头之后出现 → PROTOCOL_ERROR（h2spec 8.1.2.1 #4）
    // Pseudo-headers MUST precede any regular header field.
    if (!header_name.empty() && header_name[0] == ':')
    {
        if (steam_httppeer->h2_regular_seen)
        {
            set_conn_error(40025, 0x1);  // PROTOCOL_ERROR
            return;
        }
        if (header_name == ":method")
        {
            if (steam_httppeer->h2_method_seen) steam_httppeer->h2_method_dup = true;
            else                                  steam_httppeer->h2_method_seen = true;
        }
        else if (header_name == ":scheme")
        {
            if (steam_httppeer->h2_scheme_seen) steam_httppeer->h2_scheme_dup = true;
            else                                  steam_httppeer->h2_scheme_seen = true;
        }
        else if (header_name == ":path")
        {
            if (steam_httppeer->h2_path_seen) steam_httppeer->h2_path_dup = true;
            else                                steam_httppeer->h2_path_seen = true;
        }
    }
    else
    {
        steam_httppeer->h2_regular_seen = true;
    }

    if (table_num > 0)
    {

        switch (table_num)
        {
        case 1:
            if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
            {
                error = 40163;
                return;
            }
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["host"]       = header_value;
                steam_httppeer->header[":authority"] = std::move(header_value);
                steam_httppeer->find_host_index();
            }
            else
            {
                if (error > 0)
                {
                    return;
                }
            }
            //block_steam_httppeer->host                 = header_value;
            break;
        case 2:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "put"))
            {
                steam_httppeer->method = 6;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                steam_httppeer->method = 7;
            }
            else if (str_casecmp(header_value, "QUERY"))
            {
                steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "trace"))
            {
                steam_httppeer->method = 8;
            }
            else if (str_casecmp(header_value, "connect"))
            {
                steam_httppeer->method = 9;
            }
            steam_httppeer->iscors            = (steam_httppeer->method == 3);
            steam_httppeer->header[":method"] = header_value;
            steam_httppeer->header["method"]  = std::move(header_value);
            break;
        case 3:
            if (str_casecmp(header_value, "OPTIONS"))
            {
                steam_httppeer->method = 3;
            }
            else if (str_casecmp(header_value, "GET"))
            {
                steam_httppeer->method = 1;
            }
            else if (str_casecmp(header_value, "POST"))
            {
                steam_httppeer->method = 2;
            }
            else if (str_casecmp(header_value, "head"))
            {
                steam_httppeer->method = 5;
            }
            else if (str_casecmp(header_value, "put"))
            {
                steam_httppeer->method = 6;
            }
            else if (str_casecmp(header_value, "delete"))
            {
                steam_httppeer->method = 7;
            }
            else if (str_casecmp(header_value, "QUERY"))
            {
                steam_httppeer->method = 4;
            }
            else if (str_casecmp(header_value, "trace"))
            {
                steam_httppeer->method = 8;
            }
            else if (str_casecmp(header_value, "connect"))
            {
                steam_httppeer->method = 9;
            }
            steam_httppeer->iscors            = (steam_httppeer->method == 3);
            steam_httppeer->header[":method"] = header_value;
            steam_httppeer->header["method"]  = std::move(header_value);
            break;
        case 4:
        case 5:
            path_process(header_name, header_value, steam_httppeer);
            // 与字面量路径同一个形状：path_process 写 urlpath/url/header["urlpath"]，
            // 原始 :path 值本身也要留在 header 里给业务读
            steam_httppeer->header[":path"] = header_value;
            break;
        case 6:
        case 7:
            // :scheme 只记账，不推导 isssl：这条连接是不是 TLS 只有 socket 知道。
            // 用头值反推的话，明文连接上对端发索引 7 就能把 is_ssl() 写成 true，
            // TLS 连接上发索引 6 又把它写成 false，路由和 secure cookie 都跟着错。
            steam_httppeer->header[":scheme"] = std::move(header_value);
            break;
        case 16:
            getacceptencoding(header_name, header_value, steam_httppeer);
            break;
        case 17:
            getacceptlanguage(header_name, header_value, steam_httppeer);
            break;
        case 19:
            getaccept(header_name, header_value, steam_httppeer);
            break;
        case 20:
            steam_httppeer->header["access-control-allow-origin"] = std::move(header_value);
            break;
        case 23:
            steam_httppeer->header["authorization"] = std::move(header_value);
            break;
        case 28:
        {
            unsigned long long temp_cl = 0;
            if (!str2uint64_strict(header_value, temp_cl) || temp_cl > CONST_HTTP_BODY_POST_SIZE)
            {
                error = 40187;
                return;
            }
            steam_httppeer->content_length           = temp_cl;
            steam_httppeer->ishas_content_length      = true;
            steam_httppeer->header["content-length"] = std::move(header_value);
            break;
        }
        case 31:
            getcontenttype(header_name, header_value, steam_httppeer);
            break;
        case 32:
            cookie_process(header_name, header_value, steam_httppeer);
            break;
        case 38:
            // 静态表 38 = "host"。host 值和 :authority 一样限 72 字节：
            // 超长值会进入 header_host_process（见该函数的死循环注释）。
            if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
            {
                error = 40163;
                return;
            }
            if (header_host_process(header_value, steam_httppeer))
            {
                steam_httppeer->header["host"] = std::move(header_value);
                steam_httppeer->find_host_index();
            }
            else
            {
                if (error > 0)
                {
                    return;
                }
            }
            break;
        case 41:
            getifnonematch(header_name, header_value, steam_httppeer);
            break;
        case 48:
            steam_httppeer->header["proxy-authenticate"] = std::move(header_value);
            break;
        case 49:
            steam_httppeer->header["proxy-authorization"] = std::move(header_value);
            break;
        case 50:
            range_process(header_name, header_value, steam_httppeer);
            break;
        case 51:
            steam_httppeer->header["referer"] = std::move(header_value);
            break;
        case 58:
            steam_httppeer->header["user-agent"] = std::move(header_value);
            break;
        case 61:
            steam_httppeer->header["www-authenticate"] = std::move(header_value);
            break;
        }
    }
    else
    {
        if (header_name.size() > 48)
        {
            error = 40164;
            return;
        }
        std::string lower_name = str_tolower(header_name);
        switch (header_name.size())
        {
        case 5:
            if (str_casecmp(header_name, ":path"))
            {
                path_process(header_name, header_value, steam_httppeer);
            }
            else if (str_casecmp(header_name, "range"))
            {
                range_process(header_name, header_value, steam_httppeer);
            }
            steam_httppeer->header[lower_name] = std::move(header_value);
            break;
        case 6:

            switch (header_name[0])
            {
            case 'c':
            case 'C':
                if (str_casecmp(header_name, "Cookie"))
                {
                    cookie_process(header_name, header_value, steam_httppeer);
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
                break;
            case 'a':
            case 'A':
                if (str_casecmp(header_name, "Accept"))
                {
                    getaccept(header_name, header_value, steam_httppeer);
                }
                else
                {
                    if (header_name[0] != ':')
                    {
                        steam_httppeer->header[lower_name] = std::move(header_value);
                    }
                }
                break;
            case 'o':
            case 'O':
                if (str_casecmp(header_name, "origin"))
                {
                    // 同 h1：跨域请求才走到这里，Origin 值就在手边直接判定。
                    // :authority 按规范必在普通头之前；万一违序或整单没带，host 为空，
                    // cors_origin_process() 在 h2 下不挂起，直接按路由回落的站点当场判定
                    steam_httppeer->cors_origin_process(header_value);
                }
                steam_httppeer->header[lower_name] = std::move(header_value);
                break;
            default:
                if (header_name[0] != ':')
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }

            break;
        case 7:
            if (str_casecmp(header_name, ":method"))
            {
                if (str_casecmp(header_value, "OPTIONS"))
                {
                    steam_httppeer->method = 3;
                }
                else if (str_casecmp(header_value, "GET"))
                {
                    steam_httppeer->method = 1;
                }
                else if (str_casecmp(header_value, "POST"))
                {
                    steam_httppeer->method = 2;
                }
                else if (str_casecmp(header_value, "head"))
                {
                    steam_httppeer->method = 5;
                }
                else if (str_casecmp(header_value, "put"))
                {
                    steam_httppeer->method = 6;
                }
                else if (str_casecmp(header_value, "delete"))
                {
                    steam_httppeer->method = 7;
                }
                else if (str_casecmp(header_value, "QUERY"))
                {
                    steam_httppeer->method = 4;
                }
                else if (str_casecmp(header_value, "trace"))
                {
                    steam_httppeer->method = 8;
                }
                else if (str_casecmp(header_value, "connect"))
                {
                    steam_httppeer->method = 9;
                }
                steam_httppeer->iscors            = (steam_httppeer->method == 3);
                steam_httppeer->header["method"]  = header_value;
                steam_httppeer->header[":method"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 10:
            if (str_casecmp(header_name, ":authority"))
            {
                if (header_value.size() > CONST_HTTP2_HOST_MAX_SIZE)
                {
                    error = 40165;
                    return;
                }

                if (header_host_process(header_value, steam_httppeer))
                {
                    steam_httppeer->header["host"]       = header_value;
                    steam_httppeer->header[":authority"] = std::move(header_value);
                    steam_httppeer->find_host_index();
                }
                else
                {
                    if (error > 0)
                    {
                        return;
                    }
                }
            }
            else if (str_casecmp(header_name, "User-Agent"))
            {
                steam_httppeer->header["user-agent"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 12:
            if (str_casecmp(header_name, "Content-Type"))
            {
                getcontenttype(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 13:
            if (str_casecmp(header_name, "if-none-match"))
            {
                getifnonematch(header_name, header_value, steam_httppeer);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;

        case 14:
            if (str_casecmp(header_name, "Content-Length"))
            {
                unsigned long long temp_cl = 0;
                if (!str2uint64_strict(header_value, temp_cl) || temp_cl > CONST_HTTP_BODY_POST_SIZE)
                {
                    error = 40187;
                    return;
                }
                steam_httppeer->content_length           = temp_cl;
                steam_httppeer->ishas_content_length      = true;
                steam_httppeer->header["content-length"] = std::move(header_value);
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }
            break;
        case 15:
            if (header_name[7] == 'e' || header_name[7] == 'E')
            {
                if (str_casecmp(header_name, "accept-encoding"))
                {
                    getacceptencoding(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }
            else if (header_name[7] == 'L' || header_name[7] == 'l')
            {
                if (str_casecmp(header_name, "Accept-Language"))
                {
                    getacceptlanguage(header_name, header_value, steam_httppeer);
                    break;
                }
                else
                {
                    steam_httppeer->header[lower_name] = std::move(header_value);
                }
            }
            else
            {
                steam_httppeer->header[lower_name] = std::move(header_value);
            }

            if (str_casecmp(header_name, "Accept-Encoding"))
            {
                getacceptencoding(header_name, header_value, steam_httppeer);
            }
            if (str_casecmp(header_name, "Accept-Language"))
            {
                getacceptlanguage(header_name, header_value, steam_httppeer);
            }

            break;
        default:
            steam_httppeer->header[lower_name] = std::move(header_value);
        }
    }
}

void http2parse::getacceptlanguage([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    steam_httppeer->header["accept-language"] = header_value;
    unsigned int i                            = 0;
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] == 0x2C)
        {
            break;
        }
        steam_httppeer->state.language[i] = header_value[i];
        if (i > 6)
        {
            break;
        }
    }
    for (; i < 8; i++)
    {
        steam_httppeer->state.language[i] = 0x00;
    }
}
void http2parse::getacceptencoding([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["accept-encoding"] = header_value;
    std::string buffer_value;
    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C)
        {

            switch (buffer_value.size())
            {
            case 2:
                if (buffer_value[0] == 'b')
                {
                    steam_httppeer->state.br = true;
                }
                break;
            case 4:
                if (buffer_value[0] == 'g')
                {
                    steam_httppeer->state.gzip = true;
                }
                else if (buffer_value[0] == 'z')
                {
                    steam_httppeer->state.zstd = true;
                }
                break;
            case 7:
                if (buffer_value[0] == 'd')
                {
                    steam_httppeer->state.deflate = true;
                }
                break;
            default:;
            }
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {

        switch (buffer_value.size())
        {
        case 2:
            if (buffer_value[0] == 'b')
            {
                steam_httppeer->state.br = true;
            }
            break;
        case 4:
            if (buffer_value[0] == 'g')
            {
                steam_httppeer->state.gzip = true;
            }
            else if (buffer_value[0] == 'z')
            {
                steam_httppeer->state.zstd = true;
            }
            break;
        case 7:
            if (buffer_value[0] == 'd')
            {
                steam_httppeer->state.deflate = true;
            }
            break;
        default:;
        }
    }
}
void http2parse::getifnonematch([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i                          = 0;
    steam_httppeer->header["if-none-match"] = header_value;
    steam_httppeer->etag.clear();
    if (header_value.size() < 2)
    {
        return;
    }
    if (header_value[i] == 'W' || header_value[i] == 'w')
    {
        if (header_value[i + 1] == 0x2F)
        {
            i += 2;
        }
    }
    for (; i < header_value.size(); i++)
    {
        if (header_value[i] != 0x22)
        {
            steam_httppeer->etag.push_back(header_value[i]);
        }
    }
}

void http2parse::callposttype(const std::string &buffer_value, std::shared_ptr<httppeer> steam_httppeer, HTTP2_POST_DATA_T &pd)
{
    switch (buffer_value.size())
    {
    case 33:
        if (str_casecmp(buffer_value, "application/x-www-form-urlencoded"))
        {
            steam_httppeer->content_type = "application/x-www-form-urlencoded";
            pd.posttype                  = 1;
            steam_httppeer->posttype     = 1;
            return;
        }
        break;
    case 24:
        if (str_casecmp(buffer_value, "application/octet-stream"))
        {
            steam_httppeer->content_type = "application/octet-stream";
            pd.posttype                  = 5;
            steam_httppeer->posttype     = 5;
            return;
        }
        break;
    case 19:
        if (str_casecmp(buffer_value, "multipart/form-data"))
        {
            steam_httppeer->content_type = "multipart/form-data";
            //block_data_info_ptr->posttype      = 2;
            steam_httppeer->posttype = 2;
            return;
        }
        break;
    case 16:
        if (str_casecmp(buffer_value, "application/json"))
        {
            steam_httppeer->content_type = "application/json";
            pd.posttype                  = 3;
            steam_httppeer->posttype     = 3;
            return;
        }
        break;
    case 15:
        if (str_casecmp(buffer_value, "application/xml"))
        {
            steam_httppeer->content_type = "application/xml";
            pd.posttype                  = 4;
            steam_httppeer->posttype     = 4;
            return;
        }
        break;
    case 8:
        if (str_casecmp(buffer_value, "text/xml"))
        {
            steam_httppeer->content_type = "text/xml";
            pd.posttype                  = 4;
            steam_httppeer->posttype     = 4;
            return;
        }
        break;
    case 6:
        if (str_casecmp(buffer_value, "binary"))
        {
            steam_httppeer->content_type = "binary";
            pd.posttype                  = 6;
            steam_httppeer->posttype     = 5;
            return;
        }
        break;
    default:
        steam_httppeer->content_type = "raw";
        pd.posttype                  = 7;
        steam_httppeer->posttype     = 5;
        return;
    }
    steam_httppeer->content_type = "raw";
    pd.posttype                  = 7;
    steam_httppeer->posttype     = 5;
    return;
}

void http2parse::getcontenttype([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["content-type"] = header_value;
    std::string buffer_value;
    unsigned char statetemp = 0;
    auto iter               = http_post_data.find(steam_httppeer->stream_id);
    if (iter == http_post_data.end())
    {
        HTTP2_POST_DATA_T temp_p;
        temp_p.peer      = steam_httppeer;
        temp_p.stream_id = steam_httppeer->stream_id;
        http_post_data.insert_or_assign(steam_httppeer->stream_id, std::move(temp_p));
        iter = http_post_data.find(steam_httppeer->stream_id);
        if (iter == http_post_data.end())
        {
            return;
        }
    }

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x3B)
        {

            if (statetemp == 0)
            {
                callposttype(buffer_value, steam_httppeer, iter->second);
            }
            else if (statetemp == 1)
            {
                iter->second.chartset = buffer_value;
                statetemp             = 0;
            }
            else if (statetemp == 2)
            {
                if (!http2_boundary_normalize(iter->second.boundary, buffer_value))
                {
                    error = 40099;
                    return;
                }
                statetemp = 0;
            }
            /////////////////////
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x3D)
        {
            if (str_casecmp(buffer_value, "charset"))
            {
                buffer_value.clear();
                statetemp = 1;
                continue;
            }
            else if (str_casecmp(buffer_value, "boundary"))
            {
                buffer_value.clear();
                statetemp = 2;
                continue;
            }
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        if (statetemp == 1)
        {
            iter->second.chartset = buffer_value;
        }
        else if (statetemp == 2)
        {
            if (!http2_boundary_normalize(iter->second.boundary, buffer_value))
            {
                error = 40099;
                return;
            }
        }
        else
        {
            callposttype(buffer_value, steam_httppeer, iter->second);
        }
    }
}
void http2parse::getaccept([[maybe_unused]] const std::string &header_name, const std::string &header_value, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int i = 0, linesize = static_cast<unsigned int>(header_value.size());
    steam_httppeer->header["accept"] = header_value;
    std::string buffer_value;

    for (; i < linesize; i++)
    {
        if (header_value[i] == 0x2C || header_value[i] == 0x3B)
        {
            switch (buffer_value.length())
            {
            case 16:
                //application/json
                if (str_casecmp(buffer_value, "application/json"))
                {
                    steam_httppeer->state.accept_json = true;
                }
                break;
            case 15:
                //application/xml
                if (str_casecmp(buffer_value, "application/xml"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 9:
                //text/json
                if (str_casecmp(buffer_value, "text/json"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 8:
                //text/xml
                if (str_casecmp(buffer_value, "text/xml"))
                {
                    steam_httppeer->state.accept_xml = true;
                }
                break;
            case 10:
                if (buffer_value[6] == 'a' &&
                    buffer_value[7] == 'v' &&
                    buffer_value[8] == 'i' && buffer_value[9] == 'f')
                {
                    steam_httppeer->state.avif = true;
                }
                else if (buffer_value[6] == 'w' &&
                         buffer_value[7] == 'e' &&
                         buffer_value[8] == 'b' &&
                         buffer_value[9] == 'p')
                {
                    steam_httppeer->state.webp = true;
                }
                break;
            default:;
            }
            buffer_value.clear();
            continue;
        }
        if (header_value[i] == 0x20)
        {
            continue;
        }
        buffer_value.push_back(header_value[i]);
    }
    if (buffer_value.size() > 0)
    {
        switch (buffer_value.length())
        {
        case 10:
            if (buffer_value[6] == 'a' && buffer_value[7] == 'v' &&
                buffer_value[8] == 'i' && buffer_value[9] == 'f')
            {
                steam_httppeer->state.avif = true;
            }
            else if (buffer_value[6] == 'w' &&
                     buffer_value[7] == 'e' &&
                     buffer_value[8] == 'b' && buffer_value[9] == 'p')
            {
                steam_httppeer->state.webp = true;
            }
            break;
        case 16:
            //application/json
            if (str_casecmp(buffer_value, "application/json"))
            {
                steam_httppeer->state.accept_json = true;
            }
            break;
        case 15:
            //application/xml
            if (str_casecmp(buffer_value, "application/xml"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        case 9:
            //text/json
            if (str_casecmp(buffer_value, "text/json"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        case 8:
            //text/xml
            if (str_casecmp(buffer_value, "text/xml"))
            {
                steam_httppeer->state.accept_xml = true;
            }
            break;
        default:;
        }
    }
}
void http2parse::dynamic_table_size_update(unsigned char c,
                                            std::string_view header_data,
                                            unsigned int &begin,
                                            std::shared_ptr<httppeer> steam_httppeer)
{
    (void)steam_httppeer;
    // RFC 7541 §6.3 : Dynamic Table Size Update，首字节 001xxxxx，5 位前缀
    unsigned int new_size = c & 0x1F;
    begin += 1;
    if (!hpack_int_decode(header_data, begin, new_size, 5, new_size, error, h2_error_code, 40160))
    {
        return;
    }
    // 子解析器统一把 begin 停在「最后一个已消费字节」上，由 headers_parse 的 h_begin++ 再前进一步；
    // 这里不退这一次，紧随其后的头字段首字节就会被当成整数尾巴跳掉，整个头块从此错位。
    begin -= 1;
    // 新尺寸不得超过解码端通过 SETTINGS_HEADER_TABLE_SIZE 通告的最大值
    if (new_size > setting_data.header_table_size)
    {
        hpack_fail(40209);
        return;
    }
    dynamic_table_max_size = new_size;

    // 按 RFC 7541 §4.1 逐出尾部条目：每条占用 name+value+32 字节，直至不超新上限
    unsigned int total = 0;
    for (const auto &entry : dynamic_lists)
    {
        total += (unsigned int)(entry.first.size() + entry.second.size()) + 32;
    }
    while (!dynamic_lists.empty() && total > dynamic_table_max_size)
    {
        const auto &back = dynamic_lists.back();
        total -= (unsigned int)(back.first.size() + back.second.size()) + 32;
        dynamic_lists.pop_back();
    }
}

void http2parse::headertype1(unsigned char c,
                             std::string_view header_data,
                             unsigned int &begin,
                             std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int a = c & 0x7F;
    // std::string name_key;
    // std::string value;
    begin += 1;
    if (!hpack_int_decode(header_data, begin, a, 7, a, error, h2_error_code, 40198))
    {
        return;
    }
    begin -= 1;

    // RFC 7541 §6.1：索引头字段表示里索引 0 是非法值（"Index 0 is not used"），
    // 必须按 HPACK 解压缩失败处理成连接级 COMPRESSION_ERROR(0x9)，不能落到静态表 [0]
    // 的 ":empty" 哨兵、再靠 header_process 的未知伪头检查误判成 PROTOCOL_ERROR(0x1)。
    if (a == 0)
    {
        hpack_fail(40201);
        return;
    }

    if (a < 62)
    {
        // 静态表条目和字面量条目必须走同一个 header_process：RFC 7540 §8.1.2 的大写头名、伪头白名单、
        // 连接层头、伪头顺序与重复计数全在那里面。内联一份精简副本就会漏掉这些检测，
        // 还会让同一个请求的两条编码路径给出不同形状（副本不写 header["method"]）。
        header_process(http2_header_static_table[a].key, http2_header_static_table[a].value, a, steam_httppeer);
    }
    else
    {
        a -= 62;
        if (a >= dynamic_lists.size())
        {
            hpack_fail(40196);
            return;
        }
        unsigned int j = 0;
        for (const auto &hinfo : dynamic_lists)
        {
            if (j == a)
            {
                header_process(hinfo.first, hinfo.second, 0, steam_httppeer);
                // name有可能缓存起来了 header name maybe cache to dynamic table
                break;
            }
            j++;
        }
    }
}
void http2parse::headertype2(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;

    a = c & 0x3f;
    if (a == 0)
    {
        begin += 1;
        if (begin >= header_data.size())
        {
            hpack_fail(40128);
            return;
        }
        item_length = header_data[begin] & 0x7F;

        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40129))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40178);
            return;
        }

        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        if (begin >= header_data.size())
        {
            hpack_fail(40131);
            return;
        }
        // huffman
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40132))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40179);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, 0, steam_httppeer);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
        if (dynamic_lists.size() > 255)
        {
            dynamic_lists.pop_back();
        }
    }
    else
    {
        a = header_data[begin] & 0x3F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, a, 6, a, error, h2_error_code, 40134))
        {
            return;
        }
        begin -= 1;
        if (a < 62)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {
            c = 0;
            a -= 62;
            if (a >= dynamic_lists.size())
            {
                hpack_fail(40197);
                return;
            }
            unsigned int j = 0;
            for (const auto &hinfo : dynamic_lists)
            {
                if (j == a)
                {
                    name_key = hinfo.first;
                    break;
                }
                j++;
            }
        }

        begin += 1;
        if (begin >= header_data.size())
        {
            hpack_fail(40135);
            return;
        }
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40136))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40180);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, c, steam_httppeer);
        dynamic_lists.push_front({name_key, std::move(value)});

        begin -= 1;
        if (dynamic_lists.size() > 255)
        {
            dynamic_lists.pop_back();
        }
    }
}
void http2parse::headertype3(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;

    a = c & 0x0F;
    if (a == 0)
    {
        begin += 1;
        if (begin >= header_data.size())
        {
            hpack_fail(40138);
            return;
        }
        item_length = header_data[begin] & 0x7F;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40139))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40183);
            return;
        }
        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin           = begin + item_length;
        ishuffman_value = false;
        if (begin >= header_data.size())
        {
            hpack_fail(40140);
            return;
        }
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40141))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40184);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }
        begin = begin + item_length;

        header_process(name_key, value, 0, steam_httppeer);
        begin -= 1;
    }
    else
    {
        a = header_data[begin] & 0x0F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, a, 4, a, error, h2_error_code, 40143))
        {
            return;
        }
        if (a < 62)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {
            c = 0;
            a -= 62;
            if (a >= dynamic_lists.size())
            {
                hpack_fail(40197);
                return;
            }
            unsigned int j = 0;
            for (const auto &hinfo : dynamic_lists)
            {
                if (j == a)
                {
                    name_key = hinfo.first;
                    break;
                }
                j++;
            }
        }

        if (begin >= header_data.size())
        {
            hpack_fail(40144);
            return;
        }
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }
        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40145))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40185);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        header_process(name_key, value, c, steam_httppeer);
        begin -= 1;
    }
}
void http2parse::headertype4(unsigned char c, std::string_view header_data, unsigned int &begin, std::shared_ptr<httppeer> steam_httppeer)
{
    unsigned int a;
    unsigned int item_length;
    unsigned char field_state = 0;
    bool ishuffman_value      = false;
    std::string name_key;
    std::string value;
    a = c & 0x0F;
    if (a == 0)
    {

        begin += 1;
        if (begin >= header_data.size())
        {
            hpack_fail(40147);
            return;
        }

        item_length = header_data[begin] & 0x7F;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40148))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40190);
            return;
        }

        if (ishuffman_value)
        {
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, name_key, 1);
        }
        else
        {
            name_key.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        if (begin >= header_data.size())
        {
            hpack_fail(40149);
            return;
        }
        ishuffman_value = false;
        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40150))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40191);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;
        header_process(name_key, value, 0, steam_httppeer);
        begin -= 1;
    }
    else
    {
        a = header_data[begin] & 0x0F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, a, 4, a, error, h2_error_code, 40152))
        {
            return;
        }
        begin -= 1;
        if (a < 62)
        {
            name_key = http2_header_static_table[a].key;
            c        = a;
        }
        else
        {
            c = 0;
            a -= 62;
            if (a >= dynamic_lists.size())
            {
                hpack_fail(40197);
                return;
            }
            unsigned int j = 0;
            for (const auto &hinfo : dynamic_lists)
            {
                if (j == a)
                {
                    name_key = hinfo.first;
                    break;
                }
                j++;
            }
        }

        begin += 1;
        ishuffman_value = false;
        if (begin >= header_data.size())
        {
            hpack_fail(40153);
            return;
        }

        if (header_data[begin] & 0x80)
        {
            ishuffman_value = true;
        }

        item_length = header_data[begin] & 0x7F;
        begin += 1;
        if (!hpack_int_decode(header_data, begin, item_length, 7, item_length, error, h2_error_code, 40154))
        {
            return;
        }
        if (item_length > header_data.size() - begin)
        {
            hpack_fail(40156);
            return;
        }

        if (ishuffman_value)
        {
            field_state = 0;
            http_huffman_decode(&field_state, (unsigned char *)&header_data[begin], item_length, value, 1);
        }
        else
        {
            value.append((char *)&header_data[begin], item_length);
        }

        begin = begin + item_length;

        header_process(name_key, value, c, steam_httppeer);
        begin -= 1;
    }
}

void http2parse::readsetting(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    // unsigned int j = readoffset;
    if (temp_pack_data.flags == 0x01)
    {
        DEBUG_LOG("readsetting fackback %zu ", temp_pack_data.payload.size());
        return;
    }

    unsigned short ident_type;
    unsigned int ident_value;

    // 发送窗口只在「本连接还没初始化」时取 RFC 默认值 65535。
    // 若每收到一个 SETTINGS 帧都无条件重置成 65535：客户端中途补发一个 SETTINGS
    // （哪怕不含 INITIAL_WINDOW_SIZE）就会把之前 WINDOW_UPDATE 发放的发送额度全部抹掉，
    // 正在发送的响应会莫名其妙卡住。
    if (!conn_send_window_inited)
    {
        conn_send_window_inited         = true;
        peer_session->window_update_num = CONST_HTTP2_DEFAULT_WINDOW;
    }

    for (size_t n = 0; n < temp_pack_data.payload.size(); n += 6)
    {
        if ((n + 5) >= temp_pack_data.payload.size())
        {
            break;
        }
        ident_type = temp_pack_data.payload[n];
        ident_type = ident_type << 8 | (unsigned char)temp_pack_data.payload[n + 1];

        ident_value = temp_pack_data.payload[n + 2];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 3];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 4];
        ident_value = ident_value << 8 | (unsigned char)temp_pack_data.payload[n + 5];

        switch (ident_type)
        {
        case HTTP2_SETTINGS_HEADER_TABLE_SIZE:
            setting_data.header_table_size = ident_value;
            break;

        case HTTP2_SETTINGS_ENABLE_PUSH:
            if (ident_value > 1)
            {
                // RFC 9113 §6.5.2: 只允许 0/1，其他值 MUST 视为 PROTOCOL_ERROR
                error = 40037;
                return;
            }
            setting_data.enable_push = ident_value;
            break;

        case HTTP2_SETTINGS_MAX_CONCURRENT_STREAM:
            setting_data.max_concurrent_streams = ident_value;
            break;

        case HTTP2_SETTINGS_INITIAL_WINDOW_SIZE:

            if (ident_value > CONST_HTTP2_MAX_WINDOW)
            {
                // RFC 9113 §6.5.2: 超过 2^31-1 必须按 FLOW_CONTROL_ERROR 处理。
                // 不校验的话客户端报 0xFFFFFFFF 就能把发送窗口直接顶到 4G。
                error = 40018;
                return;
            }
            setting_data.initial_window_size = ident_value;
            // INITIAL_WINDOW_SIZE 是「每一条流」的发送窗口初值，
            // 绝不能赋给连接级窗口 window_update_num。RFC 9113 §6.9.2 还要求：
            // 该值变化时，所有已存在流的发送窗口按 (new - old) 等额调整。
            {
                // SETTINGS_INITIAL_WINDOW_SIZE 变更：每条已存在流按 (new - old) 等额调整。
                // unsigned cur + signed delta 在 delta 负时会 wrap，所以先过 int64 中间态
                // 统一 clamp 到 [0, MAX_WINDOW]。
                // Each existing stream adjusts by (new - old). cur + delta can underflow
                // unsigned cur when delta is negative — route through int64 then clamp to [0, MAX].
                // 已知限制（2026-10-09 定案：维持 unsigned，不改成 signed 追负值, 不然不好维护窗口）：地板到 0 抹掉的那段
                // 负额度不再记账，对端随后对该流发 WINDOW_UPDATE 等于把它白退回去，本端允许的在途字节
                // 比 RFC 9113 §6.9.1 的口径多出一截。追负值要改发送侧三处闸门（reserve/refund/gate_open）
                // 再加 readwinupdate 里那处额度累加，并逐处防下溢；收益只是抹平这条限制，所以不换。
                std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
                signed long long old_ll =
                    static_cast<signed long long>(peer_session->remote_initial_window_size.load());
                signed long long new_ll =
                    static_cast<signed long long>(ident_value);
                signed long long delta  = new_ll - old_ll;
                for (auto &kv : peer_session->stream_send_window)
                {
                    signed long long raw =
                        static_cast<signed long long>(kv.second) + delta;
                    unsigned int cur;
                    if (raw <= 0)
                    {
                        cur = 0;  // SETTINGS IWS 下调把窗口压到 0 以下 → 地板到 0（h2spec 6.9.2 #2 依赖 track negative，此处放弃）
                    }
                    else if (raw > static_cast<signed long long>(CONST_HTTP2_MAX_WINDOW))
                    {
                        cur = static_cast<unsigned int>(CONST_HTTP2_MAX_WINDOW);
                    }
                    else
                    {
                        cur = static_cast<unsigned int>(raw);
                    }
                    kv.second = cur;
                }
            }
            peer_session->remote_initial_window_size.store(ident_value);
            break;

        case HTTP2_SETTINGS_MAX_FRAME_SIZE:
            if (ident_value < 16384 || ident_value > 16777215)
            {
                // RFC 9113 §6.5.2: 合法范围 [2^14, 2^24-1]
                error = 40216;
                return;
            }
            setting_data.max_frame_size = ident_value;
            // 发送分片要按对端上限封顶，所以这个值必须镜像到连接上（发送侧读得到）。
            peer_session->remote_max_frame_size.store(ident_value);
            break;

        case HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
            setting_data.max_heaer_list_size = ident_value;
            break;

        default:
            break;
        }
    }

    peer_session->send_recv_setting();
    // 客户端的 INITIAL_WINDOW_SIZE 描述的是「它自己愿意接收多少」，约束的是本端的发送额度，
    // 与本端的接收窗口无关。把它直接写进本端接收窗口计数器，客户端报个超大值就能让
    // 本端长期不回 WINDOW_UPDATE。这里固定用本端自己声明的窗口值。
    //
    // 连接级窗口另有 RFC 9113 §6.9.2 固定的初值 65535，SETTINGS 改不了它，只能用
    // stream id = 0 的 WINDOW_UPDATE 抬升。抬升必须「只做一次」：每收到一个 SETTINGS
    // 就重发一次会让对端连接窗口线性增长并顶穿 2^31-1（对端 FLOW_CONTROL_ERROR）。
    if (!conn_recv_window_raised)
    {
        conn_recv_window_raised = true;
        peer_session->send_window_update_conn(CONST_HTTP2_WINDOW_UPDATE_STEP);
        conn_recv_window_num = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }
    need_wakeup_send_threads = true;
}
void http2parse::readpriority(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    if (temp_pack_data.payload.size() > 16384)
    {
        error = 40017;
    }
}

void http2parse::readgoaway(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readgoaway %zu", temp_pack_data.payload.size());

    if (temp_pack_data.stream_id != 0)
    {
        error = 40044;
        return;
    }
    if (temp_pack_data.flags != 0)
    {
        error = 40203;
        return;
    }

    peer_session->isgoway = true;
}

void http2parse::readwinupdate(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readwinupdate %zu", temp_pack_data.payload.size());

    // RFC 9113 §6.9：WINDOW_UPDATE 的载荷恰好 4 字节，其他长度是 FRAME_SIZE_ERROR。
    // 只判 < 4 时，多出来的字节会被静默忽略（分帧按帧头声明的 length 精确消费，
    // 不会造成后续帧错位，但该报错的不报）。
    if (temp_pack_data.payload.size() != 4)
    {
        error = 40012;
        return;
    }

    unsigned char temp_n  = 0;
    temp_n                = temp_pack_data.payload[0] & 0x7F;
    uint32_t ident_stream = temp_n;
    ident_stream          = ident_stream << 8;

    temp_n       = temp_pack_data.payload[1];
    ident_stream = ident_stream + temp_n;
    ident_stream = ident_stream << 8;

    temp_n       = temp_pack_data.payload[2];
    ident_stream = ident_stream + temp_n;
    ident_stream = ident_stream << 8;

    temp_n       = temp_pack_data.payload[3];
    ident_stream = ident_stream + temp_n;

    if (ident_stream == 0)
    {
        // RFC 9113 §6.9: WINDOW_UPDATE 的增量必须大于 0，为 0 属于 PROTOCOL_ERROR
        error = 40218;
        return;
    }

    // ident_stream 实际上是 window_size_increment（4 字节载荷），帧头里的
    // 流 id 在 temp_pack_data.stream_id。按 stream_id 路由到连接级或流级发送窗口，
    // 两套记账各自独立做上溢校验。
    DEBUG_LOG("window_update stream_id=%u inc=%u", temp_pack_data.stream_id, ident_stream);

    if (temp_pack_data.stream_id == 0)
    {
        // 连接级发送窗口：只受 stream id = 0 的 WINDOW_UPDATE 影响。
        //
        // window_update_num 是「对端累计授予额」，不是剩余额：发送侧扣的是
        // has_send_update_num（累计已发），剩余额 = 两者之差（见
        // http2_loop_send_sequence 的连接级窗口判定）。2^31-1 上限约束的是
        // 「未确认的剩余额度」，所以溢出校验必须作用在差值上；作用在累计值上会
        // 在长连接上必然误杀：累计授予额只增不减，发满 2^31-1 字节之后，
        // 每一个合法的 WINDOW_UPDATE 都会被拒成连接级错误。
        const unsigned long long granted = peer_session->window_update_num.load();
        if (!http2_wu_conn_avail_ok(granted, peer_session->has_send_update_num.load(), ident_stream))
        {
            // RFC 9113 §6.9.1: 发送窗口超过 2^31-1 必须按 FLOW_CONTROL_ERROR 处理。
            // 连接级的额度错在连接上，只能整条断开（error 通道回 GOAWAY）。
            // 走 set_conn_error 确保 GOAWAY 带出正确的 FLOW_CONTROL_ERROR(0x3)，
            // 而不是默认 PROTOCOL_ERROR(0x1)：对端据此把本端判定为流控违规而非协议违例。
            set_conn_error(40036, 0x3);
            return;
        }
        peer_session->window_update_num.store(granted + ident_stream);
        // 连接级发送窗口抬升：可能有流因为「累计授予 - 累计已发 = 0」挂在那里
        need_wakeup_send_threads = true;
    }
    else
    {
        // 流级发送窗口：只受对应流的 WINDOW_UPDATE 影响。
        const unsigned int wu_sid = temp_pack_data.stream_id;

        // 三道门，任一不过就丢弃这一帧：
        //  ① 偶数流 id 留给服务端发起的流，本实现不开 push，收到即非法；
        //  ② 大于本连接已见过的最大客户端流 id，说明这条流还没 OPEN；
        //  ③ 本端根本没见过这条流，或它已经结束/被撤销。
        // 只丢弃、不报错，也**绝不 emplace**：emplace 会把一个陌生流 id 写进
        // stream_send_window，而该表只按 RST 和流结束清理，等于给远程对端留了一个
        // 无界增长的入口。检查流是否存活，只看本端真正认下的流：
        // http_data（readheaders 收 HEADERS 时 emplace，分发时 extract 走）与
        // http_data_weak（分发末尾登记，响应结束后随 shared_ptr 释放而失效）并集，
        // 恰好覆盖「HEADERS 已收下」到「响应发完」整段。单用后者会在
        // 「同一批字节里 HEADERS 之后紧跟 WINDOW_UPDATE」处漏掉——那时 weak 还没写。
        // 也不能只靠 ②：对端发一个 HEADERS 把 max_client_stream_id 抬到 99 万，
        // 就能凭 ② 放行 50 万个陌生流 id。
        // 也不能对上面没通过检查、被丢弃的流回 RST_STREAM：本端没有流状态机，分不清 idle 与 closed，
        // 对 closed 流发 RST 是我们自己新造出来的协议违规（RFC 9113 §5.1 只允许
        // 对 half-closed 之外的流发 RST，且对 closed 流的一切帧都该忽略）。
        if (!http2_wu_stream_id_ok(wu_sid, max_client_stream_id))
        {
            // RFC 9113 §5.1：idle 流（从未 OPEN 过，即 id 大于本连接见过的最大客户端流 id，
            // 或偶数/id=0 这类本端不应收到的流 id）上的 WINDOW_UPDATE 是连接级错误 PROTOCOL_ERROR。
            // 已 OPEN 但已关闭的流（sid <= max_client_stream_id 且本端不再认得）走下面 ③ 的静默丢弃，
            // 那是 RFC 允许的"对关闭流的所有帧都忽略"。
            set_conn_error(40213, 0x1);
            return;
        }
        {
            auto weak_iter = http_data_weak.find(wu_sid);
            bool stream_known =
                http_data.contains(wu_sid) ||
                (weak_iter != http_data_weak.end() && weak_iter->second.lock() != nullptr);
            if (!stream_known)
            {
                winupdate_dropped++;
                DEBUG_LOG("http2 window_update on unknown or closed stream %u", wu_sid);
                return;
            }
        }

        std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
        auto it = peer_session->stream_send_window.find(wu_sid);
        if (it == peer_session->stream_send_window.end())
        {
            // 上面三道门已确认流是活的：服务端建流时并不预填本表（只在真正发 DATA
            // 前懒初始化），所以这里合法的空位表示「流在，只是还没发过 body」。
            // 这是全函数唯一允许的懒初始化入口。
            it = peer_session->stream_send_window
                     .emplace(wu_sid, peer_session->remote_initial_window_size.load())
                     .first;
        }
        // stream_send_window 存的是「剩余额度」（发送时直接扣减），所以溢出校验
        // 作用在本值上就是对的，与连接级那套「累计授予额」不同。
        // 全 unsigned 域：ident_stream 是 uint32，先判自身 ≤ MAX（否则增量超界）；
        // cur + ident_stream 用 unsigned long long 中间态防 32-bit wrap。
        // Guard increment ≤ MAX first, then compute sum in unsigned long long to dodge wrap.
        unsigned int cur = it->second;
        unsigned long long sum =
            static_cast<unsigned long long>(cur) +
            static_cast<unsigned long long>(ident_stream);
        if (ident_stream > CONST_HTTP2_MAX_WINDOW ||
            sum > static_cast<unsigned long long>(CONST_HTTP2_MAX_WINDOW))
        {
            // 流级发送窗口溢出：额度错在流上，按 RFC 9113 §5.1 走流错误处理，
            // 不必带走整条连接上其它在途的流。
            // 不再在这里裸发 RST——裸发会漏掉 cleanup_stream，导致 http_data /
            // http_post_data / 发送窗口条目泄漏到连接关闭才清，状态随请求数
            // 无限增长（与 Rapid Reset 类攻击同理，给对端留无界入口）。
            // 交给主循环统一分流：http2_handle_parse_error 会发
            // RST_STREAM(FLOW_CONTROL_ERROR=0x3)、调 cleanup_stream 回收该流
            // 全部状态、clear_error 后继续复用连接。h2_error_code 沿用 0x3，
            // 与裸发时完全一致；内部码 40221 仅作日志标识，不参与线上判定。
            // 刻意不共用连接级那一处的 40036：两级判定用的是两套不同算式（那里判累计获准额，
            // 这里判本条流的剩余额度），同号会让日志里分不出是哪一路触发的。
            set_stream_error(40221, wu_sid, CONST_HTTP2_STREAM_ERROR_FLOW_CONTROL);
            return;
        }
        it->second = static_cast<unsigned int>(sum);
        // 流级窗口抬升：这条流可能就是挂在 parked_list 里等它的那一个
        need_wakeup_send_threads = true;
    }
    // 注意：本函数只影响「发送」窗口。本端接收侧另有 conn_recv_window_num /
    // stream_recv_window 两套记账，客户端的 WINDOW_UPDATE 绝不能改动它们。
}
//
void http2parse::readping(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    // RFC 9113 §6.7：PING 的 ACK 标志表示对端对我们 PING 请求的回应。本实现从不主动
    // 发 PING 请求，因此对端任何带 ACK 的 PING 都属于「未 solicited」的协议违规：
    // §6.7 明文 "MUST NOT send a PING frame with the ACK flag set unless it has received
    // a PING frame from the peer with the ACK flag not set"。收到即按连接级 PROTOCOL_ERROR
    // 断连（h2spec 6.7.1），不能像之前那样静默 return 留着连接。
    if ((temp_pack_data.flags & 0x01) > 0)
    {
        set_conn_error(40052, 0x1);
        return;
    }

    unsigned char _recvack[] =
        {0x00, 0x00, 0x08, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int i = 0;
    for (; i < temp_pack_data.payload.size(); i++)
    {
        if (i < 8)
        {
            _recvack[i + 9] = temp_pack_data.payload[i];
        }
    }
    peer_session->http2_ring_queue->push(_recvack, 17);
    need_wakeup_send_threads = true;
    DEBUG_LOG("need ack ping");
}
void http2parse::readrst_stream(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    DEBUG_LOG("readrst_stream %u ", temp_pack_data.stream_id);
    // RFC 9113 §5.1：idle 流（id 为 0、偶数、或大于本连接见过的最大客户端流 id）上的
    // RST_STREAM 是连接级错误 PROTOCOL_ERROR。已 OPEN 但之后关闭的流收到 RST_STREAM 是允许的，
    // 走下面的正常清理（无操作）即可，不要误判成连接错误。
    if (temp_pack_data.stream_id == 0 || (temp_pack_data.stream_id & 0x1) == 0 ||
        temp_pack_data.stream_id > max_client_stream_id)
    {
        set_conn_error(40214, 0x1);
        return;
    }
    // connection means the peer is opening then immediately cancelling streams
    // to exhaust CPU. Once the count exceeds 250, flag an error so the caller
    // sends GOAWAY and closes the connection.
    if (rst_stream_count < 255)
    {
        rst_stream_count++;
    }
    if (rst_stream_count > 250)
    {
        // Rapid Reset 防护触发：连接级错误，GOAWAY 断连。显式 set_conn_error 保证
        // error_is_conn=true 且 GOAWAY 带出 PROTOCOL_ERROR(0x1)，不依赖默认值。
        set_conn_error(40220, 0x1);
    }
    // 撤销一条流 = 这条流的全部状态都不再需要，回收口径只有一处（cleanup_stream）。
    // 自己写一遍子集会漏掉 http_data / http_post_data / stream_list：业务控制器照样被
    // spawn（对端已经取消的请求仍然落库），而收 body 中途被撤销的流会一直占着临时文件和 FILE 句柄。
    cleanup_stream(temp_pack_data.stream_id);
}

// RST_STREAM / 流级错误后回收该流的全部状态：
// 清掉 http_data / http_post_data / 接收窗口 / 发送窗口 / 弱引用 / 未解完的头部块，
// 并从待派发队列移除，避免状态随请求数无限增长，也防止流还在 stream_list
// 里被 spawn 成孤儿。readrst_stream 也走这里，两处回收口径是同一份代码。
void http2parse::cleanup_stream(unsigned int sid)
{
    http_data.erase(sid);
    http_post_data.erase(sid);
    stream_recv_window.erase(sid);
    // 头部块解到一半就被撤销的流：这一块永远不会再完成，留着条目就是留着预算。
    // 正常解完的路径已经在 readheaders 末尾 erase 过，这里是空操作。
    http2_header_recvs.erase(sid);

    if (!stream_list.empty())
    {
        std::queue<unsigned int> tmp;
        while (!stream_list.empty())
        {
            unsigned int s = stream_list.front();
            stream_list.pop();
            if (s != sid)
            {
                tmp.push(s);
            }
        }
        stream_list.swap(tmp);
    }

    auto wit = http_data_weak.find(sid);
    if (wit != http_data_weak.end())
    {
        std::shared_ptr<httppeer> temp_peer = wit->second.lock();
        if (temp_peer)
        {
            temp_peer->isclose = true;
        }
        http_data_weak.erase(wit);
    }
    {
        std::lock_guard<std::mutex> lk(peer_session->stream_send_window_mutex);
        peer_session->stream_send_window.erase(sid);
    }
}

void http2parse::clear_error()
{
    error           = 0;
    error_is_conn   = true;
    error_stream_id = 0;
    h2_error_code   = 0x1;
}

void http2parse::readrawfileformdata(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack)
{
    if (!temp_post_data.fp)
    {
        server_loaclvar &localvar = get_server_global_var();

        temp_post_data.filename   = "rawcontent";
        temp_post_data.field_name = "rawcontent";

        // 落盘名统一由 make_http_temp_raw_name() 生成：文件名带 pzraw_ 前缀，
        // 该前缀是 httpwatch 周期清理时识别「框架自己的临时文件」的唯一依据，
        // 不要在此自行拼接文件名，否则文件会长期留在 temp_path 里。
        temp_post_data.temp_filename = localvar.temp_path;// + "temp/";
        temp_post_data.temp_filename.append(http::make_http_temp_raw_name());

        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data.temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data.fp = std::move(fpa);
        }
        else
        {
            error = 40167;
            return;
        }
        temp_post_data.peer->files["rawcontent"].set_object();
        temp_post_data.peer->files["rawcontent"]["filename"] = temp_post_data.filename;
        temp_post_data.peer->files["rawcontent"]["name"]     = temp_post_data.field_name;
        temp_post_data.peer->files["rawcontent"]["tempfile"] = temp_post_data.temp_filename;
        temp_post_data.peer->files["rawcontent"]["type"]     = temp_post_data.mimetype;
        temp_post_data.peer->files["rawcontent"]["size"]     = temp_post_data.cur_length;
        temp_post_data.peer->files["rawcontent"]["error"]    = 0;
    }

    if (temp_post_data.fp == nullptr)
    {
        return;
    }
    if (temp_post_data.fp)
    {
        temp_post_data.cur_length += temp_post_data.content.size();
        size_t n = fwrite(temp_post_data.content.data(), 1, temp_post_data.content.size(), temp_post_data.fp.get());
        if (n != temp_post_data.content.size())
        {
            temp_post_data.fp.reset();
            error = 40168;
        }
    }

    if (islast_pack > 0)
    {
        temp_post_data.fp.reset();
        temp_post_data.peer->files["rawcontent"]["size"] = temp_post_data.cur_length;
    }
}

void http2parse::post_form_to_postfield(std::string_view form_post_name, std::string_view form_post_value, std::shared_ptr<httppeer> steam_httppeer)
{
    bool isgroup = true;
    if (form_post_name.length() > 72)
    {
        error = 40207;
        return;
    }

    for (size_t j = 0; j < form_post_name.length(); j++)
    {
        if (form_post_name[j] == '[')
        {
            isgroup = false;
        }
    }

    if (isgroup)
    {
        steam_httppeer->post[form_post_name] = form_post_value;
        return;
    }

    isgroup = true;
    std::string objname;
    for (size_t j = 0; j < form_post_name.length(); j++)
    {
        if (form_post_name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < form_post_name.length(); n++)
            {
                if (form_post_name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (form_post_name[n] == '[')
                {

                    break;
                }
                if (form_post_name[n] != 0x22)
                {
                    key1name.push_back(form_post_name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < form_post_name.length())
                {
                    if (form_post_name[m] == '[')
                    {
                        m += 1;
                        for (; m < form_post_name.length(); m++)
                        {
                            if (form_post_name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (form_post_name[m] == '[')
                            {

                                break;
                            }
                            if (form_post_name[m] != 0x22)
                            {
                                key2name.push_back(form_post_name[m]);
                            }
                        }

                        if (ishaskey2 && m == form_post_name.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40106;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->post[objname][key1name].set_object();

                            iii      = steam_httppeer->post[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40107;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40108;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                            key1name         = std::to_string(iii);
                            steam_httppeer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40109;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40110;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            steam_httppeer->post[objname][key1name].set_object();

                            unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname][key1name].size());
                            key2name         = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40111;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40112;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40113;
                                return;
                            }

                            steam_httppeer->post[objname].set_object();
                            steam_httppeer->post[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp = form_post_value;

                            steam_httppeer->post[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == form_post_name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40114;
                            return;
                        }

                        steam_httppeer->post[objname].set_object();
                        unsigned int iii = static_cast<unsigned int>(steam_httppeer->post[objname].size());
                        key1name         = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        steam_httppeer->post[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40115;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40116;
                            return;
                        }
                        steam_httppeer->post[objname].set_object();

                        http::obj_val objtemp;
                        objtemp = form_post_value;

                        steam_httppeer->post[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // 没有数组
                }
            }
            if (isgroup)
            {
                objname.push_back(form_post_name[j]);
            }
        }
        else
        {
            objname.push_back(form_post_name[j]);
        }
    }
    if (isgroup)
    {
        steam_httppeer->post[form_post_name] = form_post_value;
    }
}

void http2parse::multipart_post_file_field(HTTP2_POST_DATA_T &temp_post_data)
{
    std::string objname;
    bool isgroup = true;

    for (size_t j = 0; j < temp_post_data.field_name.length(); j++)
    {
        if (temp_post_data.field_name[j] == '[')
        {
            std::string key1name;
            unsigned int n = j;
            n++;
            bool ishaskey  = false;
            bool ishaskey2 = false;
            for (; n < temp_post_data.field_name.length(); n++)
            {
                if (temp_post_data.field_name[n] == ']')
                {
                    ishaskey = true;
                    n++;
                    break;
                }
                else if (temp_post_data.field_name[n] == '[')
                {

                    break;
                }
                if (temp_post_data.field_name[n] != 0x22)
                {
                    key1name.push_back(temp_post_data.field_name[n]);
                }
            }

            std::string key2name;
            if (ishaskey)
            {

                unsigned int m = n;
                if (n < temp_post_data.field_name.length())
                {
                    if (temp_post_data.field_name[m] == '[')
                    {
                        m += 1;
                        for (; m < temp_post_data.field_name.length(); m++)
                        {
                            if (temp_post_data.field_name[m] == ']')
                            {
                                ishaskey2 = true;
                                m++;
                                break;
                            }
                            else if (temp_post_data.field_name[m] == '[')
                            {

                                break;
                            }
                            if (temp_post_data.field_name[m] != 0x22)
                            {
                                key2name.push_back(temp_post_data.field_name[m]);
                            }
                        }

                        if (ishaskey2 && m == temp_post_data.field_name.length())
                        {
                        }
                        else
                        {
                            ishaskey2 = false;
                        }
                    }
                }

                if (ishaskey2)
                {
                    // 双数组
                    if (key1name.empty())
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40117;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(temp_post_data.peer->files[objname].size());
                            key1name         = std::to_string(iii);
                            temp_post_data.peer->files[objname][key1name].set_object();

                            iii      = temp_post_data.peer->files[objname][key1name].size();
                            key2name = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {
                            if (objname.size() > 48)
                            {
                                error = 40118;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40119;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            unsigned int iii = static_cast<unsigned int>(temp_post_data.peer->files[objname].size());
                            key1name         = std::to_string(iii);
                            temp_post_data.peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    else
                    {
                        if (key2name.empty())
                        {
                            if (objname.size() > 48)
                            {
                                error = 40120;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40121;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            temp_post_data.peer->files[objname][key1name].set_object();

                            unsigned iii = temp_post_data.peer->files[objname][key1name].size();
                            key2name     = std::to_string(iii);

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                        else
                        {

                            if (objname.size() > 48)
                            {
                                error = 40122;
                                return;
                            }
                            if (key1name.size() > 48)
                            {
                                error = 40123;
                                return;
                            }
                            if (key2name.size() > 48)
                            {
                                error = 40124;
                                return;
                            }

                            temp_post_data.peer->files[objname].set_object();
                            temp_post_data.peer->files[objname][key1name].set_object();

                            http::obj_val objtemp;
                            objtemp.set_object();
                            objtemp["filename"] = temp_post_data.filename;
                            objtemp["name"]     = temp_post_data.field_name;
                            objtemp["tempfile"] = temp_post_data.temp_filename;
                            objtemp["type"]     = temp_post_data.mimetype;
                            objtemp["size"]     = temp_post_data.cur_length;
                            objtemp["error"]    = 0;

                            temp_post_data.peer->files[objname][key1name].push(key2name, std::move(objtemp));
                        }
                    }
                    j       = m;
                    isgroup = false;
                }
                else if (n == temp_post_data.field_name.length())
                {
                    // 只有一个
                    if (key1name.empty())
                    {
                        if (objname.size() > 48)
                        {
                            error = 40125;
                            return;
                        }

                        temp_post_data.peer->files[objname].set_object();
                        unsigned iii = temp_post_data.peer->files[objname].size();
                        key1name     = std::to_string(iii);

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data.filename;
                        objtemp["name"]     = temp_post_data.field_name;
                        objtemp["tempfile"] = temp_post_data.temp_filename;
                        objtemp["type"]     = temp_post_data.mimetype;
                        objtemp["size"]     = temp_post_data.cur_length;
                        objtemp["error"]    = 0;

                        temp_post_data.peer->files[objname].push(key1name, std::move(objtemp));
                    }
                    else
                    {
                        if (objname.size() > 48)
                        {
                            error = 40126;
                            return;
                        }
                        if (key1name.size() > 48)
                        {
                            error = 40127;
                            return;
                        }
                        temp_post_data.peer->files[objname].set_object();

                        http::obj_val objtemp;
                        objtemp.set_object();
                        objtemp["filename"] = temp_post_data.filename;
                        objtemp["name"]     = temp_post_data.field_name;
                        objtemp["tempfile"] = temp_post_data.temp_filename;
                        objtemp["type"]     = temp_post_data.mimetype;
                        objtemp["size"]     = temp_post_data.cur_length;
                        objtemp["error"]    = 0;

                        temp_post_data.peer->files[objname].push(key1name, std::move(objtemp));
                    }
                    j       = n;
                    isgroup = false;
                }
                else
                {

                    // not array
                }
            }
            if (isgroup)
            {
                objname.push_back(temp_post_data.field_name[j]);
            }
        }
        else
        {
            objname.push_back(temp_post_data.field_name[j]);
        }
    }
    if (isgroup)
    {
        temp_post_data.peer->files[temp_post_data.field_name].set_object();
        temp_post_data.peer->files[temp_post_data.field_name]["filename"] = temp_post_data.filename;
        temp_post_data.peer->files[temp_post_data.field_name]["name"]     = temp_post_data.field_name;
        temp_post_data.peer->files[temp_post_data.field_name]["tempfile"] = temp_post_data.temp_filename;
        temp_post_data.peer->files[temp_post_data.field_name]["type"]     = temp_post_data.mimetype;
        temp_post_data.peer->files[temp_post_data.field_name]["size"]     = temp_post_data.cur_length;
        temp_post_data.peer->files[temp_post_data.field_name]["error"]    = 0;
    }
}

void http2parse::post_www_form_urlencoded(HTTP2_POST_DATA_T &temp_post_data)
{
    std::string temp_value;
    std::string buffer_key;
    std::string field_value;
    unsigned int qsize    = static_cast<unsigned int>(temp_post_data.peer->rawcontent.size());
    unsigned char partype = 0;
    unsigned int j        = 0;
    unsigned int jj       = 0;
    for (j = 0; j < qsize; j++)
    {
        if (temp_post_data.peer->rawcontent[j] == 0x3D)
        {
            buffer_key = http::url_decode(temp_value.data(), temp_value.length());
            temp_value.clear();
            partype = 1;
            jj      = 0;
            continue;
        }
        else if (temp_post_data.peer->rawcontent[j] == 0x26)
        {
            field_value = http::url_decode(temp_value.data(), temp_value.length());
            if (buffer_key.size() > 72)
            {
                error = 40196;
                return;
            }
            post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
            temp_value.clear();
            field_value.clear();
            partype = 2;
            jj      = 0;
            continue;
        }
        temp_value.push_back(temp_post_data.peer->rawcontent[j]);

        if (partype == 0 || partype == 2)
        {
            //key name too long
            if (jj > 72)
            {
                error = 40197;
                return;
            }
        }
        jj++;
    }
    if (partype == 1)
    {
        field_value = http::url_decode(temp_value.data(), temp_value.length());
        if (buffer_key.size() > 72)
        {
            error = 40198;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
    else if (partype == 2)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40199;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
    else if (temp_value.size() > 0)
    {
        buffer_key = http::url_decode(temp_value.data(), temp_value.length());
        field_value.clear();
        if (buffer_key.size() > 72)
        {
            error = 40200;
            return;
        }
        post_form_to_postfield(buffer_key, field_value, temp_post_data.peer);
    }
}

void http2parse::post_multipart_itemcontent_append(HTTP2_POST_DATA_T &temp_post_data)
{
    //only process upload file
    if (temp_post_data.temp_filename.size() > 0)
    {
        if (temp_post_data.fp)
        {
            if (temp_post_data.field_item.size() == 0)
            {
                return;
            }
            temp_post_data.cur_length += temp_post_data.field_item.size();
            size_t n = fwrite(&temp_post_data.field_item[0], 1, temp_post_data.field_item.size(), temp_post_data.fp.get());
            if (n != temp_post_data.field_item.size())
            {
                error = 40038;
                temp_post_data.fp.reset();
                std::remove(temp_post_data.temp_filename.c_str());// 再删除孤儿文件
                temp_post_data.field_item.clear();
                return;
            }
            temp_post_data.field_item.clear();
            return;
        }
    }
}
void http2parse::post_multipart_itemcontent(HTTP2_POST_DATA_T &temp_post_data, bool isfilefull)
{
    if (temp_post_data.field_item.size() == 0)
    {
        return;
    }

    unsigned int pos_m = 0;
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '\r' || temp_post_data.field_item[pos_m] == '\n' || temp_post_data.field_item[pos_m] == '-')
        {
            continue;
        }
        break;
    }
    //Content-Disposition
    std::string keyname_field;
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ':')
        {
            break;
        }
        keyname_field.push_back(temp_post_data.field_item[pos_m]);
        if (keyname_field.size() > 64)
        {
            error = 40045;
            return;
        }
    }

    if (pos_m >= temp_post_data.field_item.size())
    {
        return;
    }

    if (!str_casecmp(keyname_field, "Content-Disposition"))
    {
        error = 40026;
        return;
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ';')
        {
            pos_m++;
            break;
        }
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] != ' ')
        {
            break;
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '=')
        {
            pos_m++;
            break;
        }
        keyname_field.push_back(temp_post_data.field_item[pos_m]);
        if (keyname_field.size() > 72)
        {
            error = 40027;
            return;
        }
    }
    if (!str_casecmp(keyname_field, "name"))
    {
        error = 40204;
        return;
    }
    //fieldname
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == ' ' || temp_post_data.field_item[pos_m] == '"')
        {
            continue;
        }
        else if (temp_post_data.field_item[pos_m] == '=')
        {
            continue;
        }
        break;
    }

    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '"')
        {
            pos_m++;
            break;
        }
        temp_post_data.field_name.push_back(temp_post_data.field_item[pos_m]);
        if (temp_post_data.field_name.size() > 512)
        {
            error = 40028;
            return;
        }
    }

    //filename
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] != ' ')
        {
            break;
        }
    }
    if (pos_m >= temp_post_data.field_item.size())
    {
        error = 40029;
        return;
    }

    if (temp_post_data.field_item[pos_m] == ';')
    {
        temp_post_data.isfile = true;
        pos_m++;
        keyname_field.clear();
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '=')
            {
                pos_m++;
                break;
            }
            keyname_field.push_back(temp_post_data.field_item[pos_m]);
            if (keyname_field.size() > 64)
            {
                error = 40030;
                return;
            }
        }

        if (!str_casecmp(keyname_field, "filename"))
        {
            error = 40031;
            return;
        }

        if (pos_m >= temp_post_data.field_item.size())
        {
            error = 40032;
            return;
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] != ' ')
            {
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
        }

        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '"')
            {
                pos_m++;
                break;
            }
            temp_post_data.filename.push_back(temp_post_data.field_item[pos_m]);
            if (temp_post_data.filename.size() > 512)
            {
                error = 40217;
                return;
            }
        }
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data.field_item.size())
                {
                    if (temp_post_data.field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
        }
        keyname_field.clear();
        for (; pos_m < temp_post_data.field_item.size(); pos_m++)
        {
            if (temp_post_data.field_item[pos_m] == '\r')
            {
                pos_m++;
                if (pos_m < temp_post_data.field_item.size())
                {
                    if (temp_post_data.field_item[pos_m] == '\n')
                    {
                        pos_m++;
                    }
                }
                break;
            }
            keyname_field.push_back(temp_post_data.field_item[pos_m]);
            if (keyname_field.size() > 128)
            {
                error = 40035;
                return;
            }
        }
        if (!keyname_field.empty())
        {
            unsigned int i = 0;
            for (; i < keyname_field.size(); i++)
            {
                if (keyname_field[i] == ':')
                {
                    i++;
                    for (; i < keyname_field.size(); i++)
                    {
                        if (keyname_field[i] != ' ')
                        {
                            break;
                        }
                    }
                    break;
                }
            }
            for (; i < keyname_field.size(); i++)
            {
                temp_post_data.mimetype.push_back(keyname_field[i]);
            }
        }
    }
    else
    {
        if (temp_post_data.field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data.field_item.size())
            {
                if (temp_post_data.field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
        }
    }
    keyname_field.clear();
    for (; pos_m < temp_post_data.field_item.size(); pos_m++)
    {
        if (temp_post_data.field_item[pos_m] == '\r')
        {
            pos_m++;
            if (pos_m < temp_post_data.field_item.size())
            {
                if (temp_post_data.field_item[pos_m] == '\n')
                {
                    pos_m++;
                }
            }
            break;
        }
    }

    std::string_view keyvalue_field(&temp_post_data.field_item[pos_m], temp_post_data.field_item.size() - pos_m);
    if (temp_post_data.isfile)
    {
        server_loaclvar &localvar    = get_server_global_var();
        temp_post_data.temp_filename = localvar.temp_path;
        if (temp_post_data.temp_filename.size() > 0 && temp_post_data.temp_filename.back() != '/')
        {
            temp_post_data.temp_filename.push_back('/');
        }

        temp_post_data.temp_filename = temp_post_data.temp_filename + http::make_http_temp_upload_name(temp_post_data.peer->content_length);
        std::unique_ptr<std::FILE, int (*)(FILE *)> fpa(std::fopen(temp_post_data.temp_filename.c_str(), "wb"), std::fclose);
        if (fpa)
        {
            temp_post_data.fp         = std::move(fpa);
            temp_post_data.cur_length = keyvalue_field.size();
            size_t n                  = fwrite(&keyvalue_field[0], 1, keyvalue_field.size(), temp_post_data.fp.get());
            if (n != keyvalue_field.size())
            {
                error = 40219;
                temp_post_data.fp.reset();
                std::remove(temp_post_data.temp_filename.c_str());// 再删除孤儿文件
                return;
            }
        }
        if (isfilefull)
        {
            multipart_post_file_field(temp_post_data);
            temp_post_data.isfile = false;
            temp_post_data.temp_filename.clear();
            temp_post_data.field_name.clear();
            temp_post_data.filename.clear();
            temp_post_data.fp.reset();
        }
    }
    else
    {
        post_form_to_postfield(temp_post_data.field_name, keyvalue_field, temp_post_data.peer);
    }
}
void http2parse::reset_uploadfile(HTTP2_POST_DATA_T &temp_post_data)
{
    if (temp_post_data.temp_filename.size() > 0)
    {
        if (temp_post_data.fp)
        {
            post_multipart_itemcontent_append(temp_post_data);
            multipart_post_file_field(temp_post_data);
        }
    }
    temp_post_data.isfile = false;
    temp_post_data.temp_filename.clear();
    temp_post_data.field_name.clear();
    temp_post_data.filename.clear();
    temp_post_data.fp.reset();
}
void http2parse::post_multipart_formdata(HTTP2_POST_DATA_T &temp_post_data, [[maybe_unused]] unsigned char islast_pack)
{
    unsigned int pos_m = temp_post_data.field_offset;

    //处理上次剩余的
    if (temp_post_data.pre_content.size() > 0)
    {
        if (temp_post_data.pre_content.size() == 1)
        {
            bool isnotmatch = true;
            if (temp_post_data.content[pos_m] == '-')
            {
                //说明已经命中两个，接着直接匹配boundary
                pos_m++;
                if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
                {
                    //两次数量小于boundary.size()
                    temp_post_data.field_offset = temp_post_data.content.size();
                    return;
                }
                unsigned int i = 0;
                for (; i < temp_post_data.boundary.size(); i++)
                {
                    if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                    {
                        pos_m = temp_post_data.field_offset;
                        break;
                    }
                    pos_m++;
                }
                if (i == temp_post_data.boundary.size())
                {
                    isnotmatch = false;
                }
            }
            //说明是假匹配
            temp_post_data.pre_content.clear();
            if (isnotmatch)
            {
                temp_post_data.field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
            }
            else
            {
                temp_post_data.field_offset = pos_m;
                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
        else if (temp_post_data.pre_content.size() == 2)
        {
            bool isnotmatch = true;
            if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
            {
                //两次数量小于boundary.size()
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
            unsigned int i = 0;
            for (; i < temp_post_data.boundary.size(); i++)
            {
                if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                {
                    pos_m = temp_post_data.field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data.boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            temp_post_data.pre_content.clear();
            if (isnotmatch)
            {
                //需要恢复2个
                temp_post_data.field_item.push_back('-');
                temp_post_data.field_item.push_back('-');
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
            }
            else
            {
                temp_post_data.field_offset = pos_m;
                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
        else
        {
            //3个字符，进入boundary 匹配
            bool isnotmatch = true;

            if ((pos_m + temp_post_data.boundary.size()) > temp_post_data.content.size())
            {
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
            unsigned int i = static_cast<unsigned int>(temp_post_data.pre_content.size() - 2);
            for (; i < temp_post_data.boundary.size(); i++)
            {
                if (temp_post_data.content[pos_m] != temp_post_data.boundary[i])
                {
                    pos_m = temp_post_data.field_offset;
                    break;
                }
                pos_m++;
            }
            if (i == temp_post_data.boundary.size())
            {
                isnotmatch = false;
            }

            //说明是假匹配
            if (isnotmatch)
            {
                //恢复所有temp_post_data.pre_content
                temp_post_data.field_item.append(temp_post_data.pre_content);
                //重新开始当作没有发生过
                pos_m = temp_post_data.field_offset;
                temp_post_data.pre_content.clear();
            }
            else
            {
                temp_post_data.pre_content.clear();
                temp_post_data.field_offset = pos_m;

                if (temp_post_data.field_item.size() > 1)
                {
                    temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                }
                reset_uploadfile(temp_post_data);
                post_multipart_itemcontent(temp_post_data, true);
                temp_post_data.field_item.clear();
                if ((pos_m + 1) < temp_post_data.content.size())
                {
                    if (temp_post_data.content[pos_m] == '-')
                    {
                        if (temp_post_data.content[pos_m + 1] == '-')
                        {
                            temp_post_data.peer->isfinish = true;
                            //post body end
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                    else if (temp_post_data.content[pos_m] == '\r')
                    {
                        if (temp_post_data.content[pos_m + 1] == '\n')
                        {
                            pos_m = pos_m + 1;
                            //next char
                            temp_post_data.field_offset = pos_m + 1;
                        }
                    }
                }
                return;
            }
        }
    }

    temp_post_data.pre_content.clear();
    for (; pos_m < temp_post_data.content.size(); pos_m++)
    {
        if (temp_post_data.content[pos_m] == '-')
        {
            unsigned int j = pos_m;
            j++;
            if (j < temp_post_data.content.size())
            {
                if (temp_post_data.content[j] == '-')
                {
                    j++;
                    if (j < temp_post_data.content.size())
                    {
                        //主要是检查是否匹配或最后
                        unsigned int i = 0;
                        for (; i < temp_post_data.boundary.size();)
                        {
                            if (temp_post_data.content[j] != temp_post_data.boundary[i])
                            {
                                break;
                            }
                            j++;
                            i++;
                            if (j >= temp_post_data.content.size())
                            {
                                break;
                            }
                        }
                        if (i == temp_post_data.boundary.size())
                        {
                            temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);

                            if (temp_post_data.field_item.size() > 1)
                            {
                                temp_post_data.field_item.resize(temp_post_data.field_item.size() - 2);
                            }
                            reset_uploadfile(temp_post_data);
                            post_multipart_itemcontent(temp_post_data, true);
                            temp_post_data.field_offset = j;
                            temp_post_data.field_item.clear();

                            if (j + 1 < temp_post_data.content.size())
                            {
                                if (temp_post_data.content[j] == '-' && temp_post_data.content[j + 1] == '-')
                                {
                                    temp_post_data.field_offset   = j + 1;
                                    temp_post_data.peer->isfinish = true;
                                }
                                else if (temp_post_data.content[j] == '\r' && temp_post_data.content[j + 1] == '\n')
                                {
                                    temp_post_data.field_offset = j + 1;
                                }
                            }

                            return;
                        }
                        if (j >= temp_post_data.content.size())
                        {
                            //例外已经到头了
                            //让进入下一轮
                            temp_post_data.pre_content.append(&temp_post_data.content[pos_m], temp_post_data.content.size() - pos_m);
                            temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                            temp_post_data.field_offset = temp_post_data.content.size();
                            return;
                        }
                    }
                    else
                    {
                        temp_post_data.pre_content.push_back('-');
                        temp_post_data.pre_content.push_back('-');
                        temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                        temp_post_data.field_offset = temp_post_data.content.size();
                        return;
                    }
                }
            }
            else
            {
                temp_post_data.pre_content.push_back('-');
                temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
                temp_post_data.field_offset = temp_post_data.content.size();
                return;
            }
        }
    }
    temp_post_data.field_item.append(&temp_post_data.content[temp_post_data.field_offset], pos_m - temp_post_data.field_offset);
    temp_post_data.field_offset = pos_m;
}

void http2parse::post_data_process(HTTP2_POST_DATA_T &temp_post_data, unsigned char islast_pack)
{
#ifdef ENABLE_FASTCGI
    if (temp_post_data.peer->compress == 10)
    {
        //ready output to php
        if (temp_post_data.peer->content_length < CONST_PHP_BODY_POST_SIZE)
        {
            if (temp_post_data.peer->output.size() < temp_post_data.peer->content_length)
            {
                temp_post_data.peer->output.append(temp_post_data.content);
            }
        }
        else
        {
            error = 40172;
            return;
        }
    }
    else
#endif
    {
        if (temp_post_data.posttype == 0)
        {
            if (temp_post_data.content.size() > 0)
            {
                char first = temp_post_data.content[0];
                if (first == '-')
                {
                    temp_post_data.posttype = 2;// multipart
                }
                else if (first == '{' || first == '[')
                {
                    temp_post_data.posttype = 3;// json
                }
                else if (first == '<')
                {
                    temp_post_data.posttype = 4;// xml
                }
                else
                {
                    temp_post_data.posttype = 5;// fallback: raw
                }
            }
        }

        switch (temp_post_data.posttype)
        {
        case 1:
            // x-www-form-urlencoded
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40166;
                return;
            }
            if (islast_pack)
            {
                post_www_form_urlencoded(temp_post_data);
            }
            if (error > 0)
            {
                return;
            }
            break;
        case 2:
            // multipart/form-data
            temp_post_data.field_offset = 0;
            for (; temp_post_data.field_offset < temp_post_data.content.size();)
            {
                post_multipart_formdata(temp_post_data, islast_pack);
            }

            if (islast_pack)
            {
                reset_uploadfile(temp_post_data);
            }
            else
            {
                if (temp_post_data.isfile && temp_post_data.field_item.size() > 2097152)
                {
                    //及时消化掉大文件中间内容，尽快保存到文件
                    post_multipart_itemcontent_append(temp_post_data);
                    temp_post_data.field_item.clear();
                }
                else if (temp_post_data.field_item.size() > 2097152)
                {
                    if (!temp_post_data.isfile)
                    {

                        std::string_view check_filename = std::string_view(temp_post_data.field_item.data(), 80);
                        size_t pos                      = check_filename.find("filename");
                        if (pos != std::string::npos)
                        {
                            reset_uploadfile(temp_post_data);
                            post_multipart_itemcontent(temp_post_data, false);
                            temp_post_data.field_item.clear();
                        }
                    }
                }
            }

            if (error > 0)
            {
                return;
            }
            break;
        case 3:
            // json
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40170;
                return;
            }
            if (islast_pack)
            {
                temp_post_data.peer->json.from_json(temp_post_data.peer->rawcontent);
            }
            break;
        case 4:
            //xml
            temp_post_data.peer->rawcontent.append(temp_post_data.content);
            if (temp_post_data.peer->rawcontent.size() > CONST_HTTP_JSON_POST_SIZE)
            {
                error = 40171;
                return;
            }
            break;
        case 5:
            // octet-stream
            readrawfileformdata(temp_post_data, islast_pack);
            break;
        }
    }
}
void http2parse::readpostdata(const HTTP2_PACK_DATA_T &temp_pack_data)
{
    if ((temp_pack_data.flags & 0xF6) != 0)
    {
        //非数据部分
        error = 40042;
        return;
    }

    unsigned char last_pack    = temp_pack_data.flags & HTTP2_DATA_END_STREAM;
    unsigned int header_offset = 0, padded_length = 0;
    if ((temp_pack_data.flags & HTTP2_DATA_PADDED) > 0)
    {
        if (temp_pack_data.payload.size() == 0)
        {
            error = 40041;
            return;
        }
        padded_length = temp_pack_data.payload[header_offset];
        header_offset++;
    }

    unsigned int new_size_num = header_offset + padded_length;
    auto iter                 = http_data.find(temp_pack_data.stream_id);
    if (iter == http_data.end())
    {
        // 流不在 http_data 里，分两种身份，不能一律按"陌生流"处理：
        // ① 本连接从未见过的流（流 0、偶数流、或 id 大于已开过的最大流 id）：
        //    RFC 9113 §5.1 规定 idle 上收到 DATA 是连接错误 PROTOCOL_ERROR，保持 GOAWAY 行为。
        // ② 见过但已派发/结束的流：http_data 在派发时已被 extract 掉，等响应发完这条流对
        //    客户端就是 closed。此时再来 DATA 是流错误 STREAM_CLOSED，按 ① 把它 GOAWAY 掉整条
        //    连接属于误伤（h2spec 6.1.2 / 5.1.x 期望 RST_STREAM(STREAM_CLOSED)，连接关闭只是勉强放过）；
        //    而且单纯 GOAWAY 还容易被当成"连接级错误"，与 RFC 5.4.2 的"流级错误"要求不符。
        if (temp_pack_data.stream_id == 0 || (temp_pack_data.stream_id & 0x1) == 0 ||
            temp_pack_data.stream_id > max_client_stream_id)
        {
            set_conn_error(40215, 0x1);  // 0x1 = PROTOCOL_ERROR
        }
        else
        {
            set_stream_error(40211, temp_pack_data.stream_id, 0x5);  // 0x5 = STREAM_CLOSED
        }
        return;
    }

    // 这条流在头块阶段就被站点限额拒掉了：正文一律丢弃，不落盘、也不再派发第二份响应。
    // 丢弃不等于可以不记账：连接级窗口的扣减与补量照正常路径走一遍，只跳过越限判定。
    // 「直接原额发 WINDOW_UPDATE」是错的——本端计数没扣就补量，等于每丢一帧就把对端的
    // 连接窗口往上顶一截，攒久了超 2^31-1，对端必须按 FLOW_CONTROL_ERROR 断连。
    // 流级窗口不碰：这条流已经答完，额度没有后续复用者。
    if (iter->second->reject_status > 0)
    {
        unsigned int drop_bytes = temp_pack_data.length;
        if (drop_bytes > conn_recv_window_num)
        {
            drop_bytes = conn_recv_window_num;
        }
        conn_recv_window_num -= drop_bytes;
        if (conn_recv_window_num < CONST_HTTP2_WINDOW_UPDATE_THRESHOLD)
        {
            peer_session->send_window_update_conn(CONST_HTTP2_LOCAL_INITIAL_WINDOW - conn_recv_window_num);
            need_wakeup_send_threads = true;
            conn_recv_window_num     = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
        }
        return;
    }

    auto post_iter = http_post_data.find(temp_pack_data.stream_id);
    if (post_iter == http_post_data.end())
    {
        // 该流还没建过 http_post_data：
        //  * 正常 POST/PUT（HEADERS 未带 END_STREAM，isfinish=false）：首个 body DATA 到达，
        //    按 body 收集逻辑建 http_post_data。
        //  * 但 HEADERS 已带 END_STREAM（isfinish=true）说明请求体为空、客户端此侧已发完，
        //    流已进入 half-closed(remote)/closed。此时再来 DATA 是 RFC 9113 §5.1 规定的
        //    流错误 STREAM_CLOSED，绝不能当成 body 收下——否则会和已派发的请求重复响应
        //    （h2spec 6.1.2 / 5.1.x 失败），也违背了"流级错误"的语义。
        if (iter->second->isfinish)
        {
            set_stream_error(40211, temp_pack_data.stream_id, 0x5);  // 0x5 = STREAM_CLOSED
            return;
        }
        HTTP2_POST_DATA_T temp_p;
        temp_p.peer = iter->second;

        temp_p.stream_id = temp_pack_data.stream_id;
        http_post_data.insert_or_assign(temp_pack_data.stream_id, std::move(temp_p));
        post_iter = http_post_data.find(temp_pack_data.stream_id);
        if (post_iter == http_post_data.end())
        {
            error = 40024;
            return;
        }
    }

    // DATA 帧的 padding（1 字节 pad length + N 字节填充）要占 flow-control 额度，
    // 但不属于正文。若用整个 payload.size() 累加 exp_length，
    // 客户端一旦使用 PADDED，exp_length 必然大于 content-length，收尾时误报 40202。
    unsigned int content_bytes = static_cast<unsigned int>(temp_pack_data.payload.size());
    if (new_size_num > 0)
    {
        // RFC 9113 §6.1：Pad Length 字段为 0（即"带 PADDED 标志但零填充"或"只有填充、
        // 正文为空"）是合法帧，payload 恰好等于 header_offset + padded_length。
        // 用 >= 会误杀这类合法空 DATA 帧，必须严格大于才算填充越界。
        if (new_size_num > temp_pack_data.payload.size())
        {
            error = 40206;
            return;
        }
        new_size_num = temp_pack_data.payload.size() - new_size_num;
        if (header_offset > temp_pack_data.payload.size() || new_size_num > (temp_pack_data.payload.size() - header_offset))
        {
            error = 40023;
            return;
        }
        content_bytes             = new_size_num;
        post_iter->second.content = std::string_view(temp_pack_data.payload.data() + header_offset, new_size_num);
    }
    else
    {
        post_iter->second.content = std::string_view(temp_pack_data.payload.data(), temp_pack_data.payload.size());
    }

    post_iter->second.exp_length += content_bytes;
    if (post_iter->second.exp_length > CONST_HTTP_BODY_POST_SIZE)
    {
        error = 40009;
        return;
    }

    // RFC 9113 §6.1（DATA）："The entire DATA frame payload is included in flow control,
    // including the Pad Length and Padding fields if present." temp_pack_data.length 正是这个口径。
    // 连接级与流级必须各自独立扣减、各自独立补量，
    // 不能用「全局计数越线」去决定该补给哪一条流。
    unsigned int fc_bytes = temp_pack_data.length;

    // RFC 9113 §6.9："A receiver MAY respond with a stream error (Section 5.4.2) or
    // connection error (Section 5.4.1) of type FLOW_CONTROL_ERROR if it is unable to
    // accept a frame." 两级都合法，这里取**流级**那一支：流控的存在理由（§5.2.2）就是
    // 「这条流收不下还要能继续收别的流」，为一枚超窗 DATA 帧发 GOAWAY 会把整条连接上
    // 所有在途流一起带走。
    // 窗口不够时不能截断继续——对端可以发一个超大 PADDED 帧一次穿透本端背压（窗口被砍到 0 再补回，但正文已被正常处理）。
    // Guard both windows before accepting; overflow ⇒ RST_STREAM(FLOW_CONTROL_ERROR)
    // and drop the frame + all subsequent on this stream.
    auto s_win_iter = stream_recv_window.find(temp_pack_data.stream_id);
    if (s_win_iter == stream_recv_window.end())
    {
        s_win_iter = stream_recv_window.emplace(temp_pack_data.stream_id, CONST_HTTP2_LOCAL_INITIAL_WINDOW).first;
    }
    if (fc_bytes > conn_recv_window_num || fc_bytes > s_win_iter->second)
    {
        // 流级 RST_STREAM 后这条流后续 DATA 都走 http_data 已派发/已结束分支
        // （isfinish 置 true），不会再被当成 body 收下。
        set_stream_error(40210, temp_pack_data.stream_id, 0x3);  // 0x3 = FLOW_CONTROL_ERROR
        return;
    }

    conn_recv_window_num -= fc_bytes;
    s_win_iter->second   -= fc_bytes;

    if (conn_recv_window_num < CONST_HTTP2_WINDOW_UPDATE_THRESHOLD)
    {
        // 只补连接级：增量正好等于本端已消费掉的量
        peer_session->send_window_update_conn(CONST_HTTP2_LOCAL_INITIAL_WINDOW - conn_recv_window_num);
        need_wakeup_send_threads = true;
        conn_recv_window_num     = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }

    if (s_win_iter->second < CONST_HTTP2_WINDOW_UPDATE_THRESHOLD)
    {
        // 只补当前这条流，不要波及别的流
        peer_session->send_window_update_stream(temp_pack_data.stream_id,
                                                CONST_HTTP2_LOCAL_INITIAL_WINDOW - s_win_iter->second);
        need_wakeup_send_threads = true;
        s_win_iter->second       = CONST_HTTP2_LOCAL_INITIAL_WINDOW;
    }

    bool is_last_pack = last_pack;

    post_data_process(post_iter->second, is_last_pack);

    if (is_last_pack)
    {
        if (iter->second->ishas_content_length && post_iter->second.exp_length != iter->second->content_length)
        {
            error = 40202;
            return;
        }

        http_post_data.erase(post_iter);
        // 流已结束，释放该流的窗口记账，避免 map 随请求数无限增长
        stream_recv_window.erase(temp_pack_data.stream_id);
        iter->second->isfinish = true;
        stream_list.emplace(temp_pack_data.stream_id);
    }
}

void http2parse::process_pack()
{
    // // RFC 9113 §6.10：头部块进行中（header_block_pending）收到非 CONTINUATION(0x09) 帧，
    // // 属连接级 PROTOCOL_ERROR，必须 GOAWAY。否则未完成的请求会被当完整请求继续处理、回 DATA。
    // // 单标志位 O(1) 判断，不遍历任何容器，正常分发路径零开销。
    // if (header_block_pending && pack_data.frame_type != 0x09)
    // {
    //     set_conn_error(40036, 0x1);
    //     return;
    // }

    // RFC 9113 §4.2 帧定长：长度与帧类型不匹配属于连接级错误 FRAME_SIZE_ERROR(0x6)，
    // 必须 GOAWAY；合并到分发 switch，省一次跳转表查找。
    switch (pack_data.frame_type)
    {
    case 0x00:
        // DATA（长度可变）
        // RFC 9113 §4.2.2 / §4.2.3：帧 payload 不得超过本端愿意接受的最大帧大小。
        // 本实现不主动下发 SETTINGS_MAX_FRAME_SIZE，按 RFC 默认上限 2^14 = 16384 执行
        // （不能用 setting_data.max_frame_size——它会被对端 SETTINGS 覆盖成对端上限，
        // 那样对端就能把我们的接受上限抬高成 DoS 入口）。超界属连接级 FRAME_SIZE_ERROR。
        if (pack_data.length > 16384)
        {
            set_conn_error(40050, 0x6);
            return;
        }
        readpostdata(pack_data);
        break;
    case 0x01:
        // HEADERS（长度可变）
        // 同上：帧 payload 不得超过本端接受上限（默认 16384），否则连接级 FRAME_SIZE_ERROR。
        if (pack_data.length > 16384)
        {
            set_conn_error(40051, 0x6);
            return;
        }
        readheaders(pack_data);
        break;
    case 0x02: // PRIORITY 固定 5 字节
        // RFC 9113 §6.3：PRIORITY 必须关联一条流，stream_id 为 0 是连接级 PROTOCOL_ERROR。
        if (pack_data.stream_id == 0)
        {
            set_conn_error(40047, 0x1);
            return;
        }
        if (pack_data.length != 5)
        {
            set_conn_error(40030, 0x6);
            return;
        }
        readpriority(pack_data);
        break;
    case 0x03: // RST_STREAM 固定 4 字节
        if (pack_data.length != 4)
        {
            set_conn_error(40031, 0x6);
            return;
        }
        readrst_stream(pack_data);
        break;
    case 0x04: // SETTINGS 长度须为 6 的倍数
        // RFC 9113 §6.5.1：SETTINGS 只作用于连接，stream_id 非 0 是连接级 PROTOCOL_ERROR。
        if (pack_data.stream_id != 0)
        {
            set_conn_error(40048, 0x1);
            return;
        }
        if (pack_data.length % 6 != 0)
        {
            set_conn_error(40032, 0x6);
            return;
        }
        readsetting(pack_data);
        break;
    case 0x05:
        error = 40016;
        return;
    case 0x06: // PING 固定 8 字节
        // RFC 9113 §6.7：PING 不关联任何流，stream_id 非 0 是连接级 PROTOCOL_ERROR。
        if (pack_data.stream_id != 0)
        {
            set_conn_error(40049, 0x1);
            return;
        }
        if (pack_data.length != 8)
        {
            set_conn_error(40033, 0x6);
            return;
        }
        readping(pack_data);
        break;
    case 0x07:
        // GOAWAY（长度可变）
        readgoaway(pack_data);
        break;
    case 0x08: // WINDOW_UPDATE 固定 4 字节
        if (pack_data.length != 4)
        {
            set_conn_error(40034, 0x6);
            return;
        }
        readwinupdate(pack_data);
        break;
    case 0x09:
        // CONTINUATION（长度可变）
        readheaders(pack_data);
        break;
    default:
        error = 40169;
    }
}

void http2parse::read_pack_data(const unsigned char *buffer, unsigned int buffersize)
{
    if (pack_data.length == 0)
    {
        // 帧头不足 9 字节：把「还缺的字节」攒进 subpad。
        // 不能把本包剩余字节无条件塞进 subpad 而不看它已有多少：
        // 客户端按 1 字节/包发送时 subpad 会无上限增长；一旦 subpad > 9，
        // 下面只解析前 9 字节、多出来的字节被静默丢弃，后续帧头/载荷整体错位。
        if ((readoffset + 9) > buffersize && pack_data.subpad.size() < 9)
        {
            unsigned int need = static_cast<unsigned int>(9 - pack_data.subpad.size());
            for (; readoffset < buffersize && need > 0; readoffset++, need--)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
            }
            if (pack_data.subpad.size() < 9)
            {
                // 帧头还没凑齐，等下一次读
                return;
            }
        }

        unsigned int j = 0;

        if (pack_data.subpad.size() > 0)
        {
            j = pack_data.subpad.size();
            for (; j < 9; j++)
            {
                pack_data.subpad.push_back(buffer[readoffset]);
                readoffset++;
                // j 已经走到 8 说明第 9 个字节刚补齐，帧头是完整的，不能报错
                // （只要 readoffset 触底就报 40005，会在 TCP 恰好切在帧头最后一个字节时误杀连接）
                if (j < 8 && readoffset >= buffersize)
                {
                    error = 40005;
                    return;
                }
            }
            j                = 0;
            pack_data.length = (unsigned char)pack_data.subpad[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)pack_data.subpad[j++];

            pack_data.frame_type = pack_data.subpad[j++];
            pack_data.flags      = pack_data.subpad[j++];

            if ((pack_data.subpad[j] & 0x7F) > 0)
            {
                error = 40002;
                return;
            }

            pack_data.stream_id = pack_data.subpad[j++] & 0x7F;
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)pack_data.subpad[j++];
            j                   = readoffset;
        }
        else
        {
            j = readoffset;
            if ((j + 5) > buffersize)
            {
                error = 40003;
                return;
            }

            pack_data.length = buffer[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)buffer[j++];
            pack_data.length = (pack_data.length << 8) | (unsigned char)buffer[j++];

            pack_data.frame_type = buffer[j++];
            pack_data.flags      = buffer[j++];

            if (j < buffersize && (buffer[j] & 0x7F) > 0)
            {
                error = 40208;
                return;
            }

            if ((j + 4) > buffersize)
            {
                error = 40004;
                return;
            }

            pack_data.stream_id = buffer[j++] & 0x7F;
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
            pack_data.stream_id = (pack_data.stream_id << 8) | (unsigned char)buffer[j++];
        }

        // 非 DATA 帧按帧头声明的 length 提前拒绝：本函数末尾那个 16K 检查是在
        // DATA 帧不在这里卡（它由 CONST_HTTP_BODY_POST_SIZE 与 flow-control 窗口限制）。
        if (pack_data.frame_type != 0x00 && pack_data.length > CONST_HTTP_HEADER_BODY_SIZE)
        {
            error      = 40001;
            readoffset = buffersize;
            return;
        }

        for (; j < buffersize; j++)
        {
            if (pack_data.payload.size() < pack_data.length)
            {
                pack_data.payload.push_back(buffer[j]);
            }
            else
            {
                break;
            }
        }
        readoffset = j;
    }
    else
    {
        unsigned int j = readoffset;
        for (; j < buffersize; j++)
        {
            if (pack_data.payload.size() < pack_data.length)
            {
                pack_data.payload.push_back(buffer[j]);
            }
            else
            {
                break;
            }
        }
        readoffset = j;
    }

    if (pack_data.frame_type != 0x00 && pack_data.payload.size() > CONST_HTTP_HEADER_BODY_SIZE)
    {
        error      = 40001;
        readoffset = buffersize;
        return;
    }

    if (pack_data.payload.size() >= pack_data.length)
    {
        //每次收完整的包
        process_pack();
        pack_data.length     = 0;
        pack_data.frame_type = 0;
        pack_data.flags      = 0;
        pack_data.stream_id  = 0;
        pack_data.payload.clear();
        pack_data.subpad.clear();
    }
}

unsigned int http2parse::process(const unsigned char *buffer, unsigned int buffersize)
{
    if (error > 0)
    {
        return 0;
    }
    readoffset = 0;
    for (; readoffset < buffersize;)
    {
        read_pack_data(buffer, buffersize);
        if (error > 0)
        {
            break;
        }
    }
    // 出错时停在坏帧末尾：其后已读进来的帧要靠这个返回值交给调用方续着解
    return readoffset;
}
}// namespace http
