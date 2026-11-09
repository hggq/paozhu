/**
 *  @copyright copyright 2026, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file rpc_parse.cpp
 *  @date 2026-05-06
 *
 *  http rpc parse file
 *
 *
 */
#include "server_localvar.h"
#include "rpc_parse.h"
#include "func.h"
#include "cost_define.h"
#include <cstdio>

namespace http
{

namespace
{
// KV 行以 0x0A 分隔：键或值里带 CR/LF 就是在响应帧里多写一行头
bool is_one_line(const std::string &s)
{
    for (unsigned char c : s)
    {
        if (c == 0x0A || c == 0x0D)
        {
            return false;
        }
    }
    return true;
}

// 请求行/查询串不允许 NUL 与控制字符（日志、SQL、文件路径注入面）。
// 不做 %XX 解码：保住现有 RPC 客户端把 %XX 原样传参的语义。
bool is_clean_kv(const std::string &s)
{
    for (unsigned char c : s)
    {
        if (c < 0x20 || c == 0x7f)
        {
            return false;
        }
    }
    return true;
}

// 临时观测（N4-b 评估用）：只认「% + 两位合法 hex」，不把 100% / %zz 算进去。
// 确认业务流量里有没有真正的 percent-encoding 之后，连同调用处的日志一起删除。
int hexval(unsigned char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool is_pct_encoded(const std::string &s)
{
    for (size_t i = 0; i + 2 < s.size(); ++i)
    {
        if (s[i] == '%' &&
            hexval(static_cast<unsigned char>(s[i + 1])) >= 0 &&
            hexval(static_cast<unsigned char>(s[i + 2])) >= 0)
        {
            return true;
        }
    }
    return false;
}
}// namespace

rpc_parse::rpc_parse() : uprawfile(nullptr, std::fclose) { iserror = false; }
void rpc_parse::set_chunk(bool c)
{
    ischunked = c;
}
void rpc_parse::reset()
{
    isfinish         = false;
    isbegin          = false;
    isbody           = false;
    iserror          = false;
    cur_process_type = 0;
    offsetnum        = 0;
    val_size         = 0;
    body_size        = 0;
    content_size     = 0;
    body_sub         = 0;
    line_bytes       = 0;
    kv_num           = 0;
    kv_bytes         = 0;
    has_host         = false;
    has_size         = false;
    ischunked        = false;
    is_send          = false;

    read_key.clear();//read temp var in the header
    read_value.clear();
    read_temp_file.clear();
    status_msg.clear();
    send_content.clear();
    // 保留容量给 keep-alive 的下一次响应；只在超大响应后按阈值回收
    if (send_content.capacity() > CONST_PEER_BUFFER_SHRINK_SIZE)
    {
        send_content.shrink_to_fit();
    }
    uprawfile = nullptr;
}
void rpc_parse::process_headkv()
{
    kv_num++;
    kv_bytes += read_key.size() + read_value.size();
    if (kv_num > CONST_RPC_KV_MAX_ITEMS || kv_bytes > CONST_RPC_KV_MAX_BYTES)
    {
        // KV 段条数/总字节超限：按协议错误处理
        iserror = true;
        return;
    }

    // 除 size 外（其值是 body 长度的大端二进制），其余头的 value 必须是文本
    if (!(read_key.size() == 4 && read_key == "size"))
    {
        for (unsigned int i = 0; i < read_value.size(); i++)
        {
            unsigned char c = read_value[i];
            if (c != 0x09 && (c < 0x20 || c > 0x7E))
            {
                iserror = true;
                return;
            }
        }
    }

    if (read_key.size() == 4 && read_key == "HOST")
    {
        if (has_host)
        {
            // 保留字重复：按协议错误处理
            iserror = true;
            return;
        }
        // 严格域名白名单：host 值后续可能被拼进文件路径，
        // 跟 HTTP/1 getheaderhost / HTTP/2 header_host_process 保持同一口径，
        // 只允许 [0-9a-zA-Z.-]，非法字符直接按协议错误断连。
        for (unsigned int i = 0; i < read_value.size(); i++)
        {
            unsigned char c = read_value[i];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') || c == '.' || c == '-'))
            {
                iserror = true;
                return;
            }
        }
        has_host   = true;
        peer->host = read_value;
    }
    else if (read_key.size() == 6 && read_key == "cookie")
    {
        read_key.clear();
        unsigned int i = 0;
        for (; i < read_value.size(); i++)
        {
            if (read_value[i] == '=')
            {
                break;
            }
            read_key.push_back(read_value[i]);
        }

        if (read_key.empty())
        {
            //ignore
            return;
        }

        if (i < read_value.size() && read_value[i] == '=')
        {
            i++;
            if (i < read_value.size())
            {
                peer->cookie[read_key] = read_value.substr(i);
            }
            else
            {
                //empty
                peer->cookie[read_key] = "";
            }
        }
    }
    else if (read_key.size() == 4 && read_key == "size")
    {
        if (has_size)
        {
            // 保留字重复：按协议错误处理
            iserror = true;
            return;
        }
        has_size = true;
        if (read_value.size() > 8)
        {
            // 长度字段最多 8 字节（long long）
            iserror = true;
            return;
        }
        unsigned long long v = 0;
        for (unsigned int i = 0; i < read_value.size(); i++)
        {
            v = (v << 8) + static_cast<unsigned char>(read_value[i]);
        }
        if (v > 68719476735ULL)
        {
            // 上限 64GiB-1，再大就超出 long long 累加的安全范围
            iserror = true;
            return;
        }
        body_size    = static_cast<long long>(v);
        content_size = body_size;
        body_sub     = body_size;
    }
    else
    {
        peer->post[read_key] = read_value;
    }
}
void rpc_parse::process_value(const unsigned char *buffer, unsigned int readnum)
{
    if (val_size == 0)
    {
        // 空值：不消费净荷字节，直接去读行尾 0x0A
        cur_process_type = 11;
        return;
    }
    for (; offsetnum < readnum; offsetnum++)
    {
        read_value.push_back(buffer[offsetnum]);
        val_size--;
        if (val_size == 0)
        {
            cur_process_type = 11;
            offsetnum++;
            return;
        }
    }
}

void rpc_parse::process_parameter(const unsigned char *buffer, unsigned int readnum)
{
    if (cur_process_type == 0)
    {
        if (read_key.empty() && offsetnum < readnum && buffer[offsetnum] == 0x0A)
        {
            // 空行（KV 段结束）作为新段首字节：直接进 body
            offsetnum++;
            isbody = true;
            return;
        }
        for (unsigned int i = 0; offsetnum < readnum; offsetnum++)
        {
            if (buffer[offsetnum] == 0x20)
            {
                break;
            }
            // header key 必须是可打印 ASCII（不含空格与控制字符）
            if (buffer[offsetnum] < 0x21 || buffer[offsetnum] > 0x7E)
            {
                iserror = true;
                return;
            }
            read_key.push_back(buffer[offsetnum]);
            i++;
            if (i > 128)
            {
                iserror = true;
                return;
            }
        }

        if (read_key.size() > 128)
        {
            iserror = true;
            return;
        }
        if (offsetnum >= readnum)
        {
            // 本段耗尽（key 未读完）：状态保留，下一段继续
            return;
        }
        offsetnum++;// 循环不变式：能到此处 buffer[offsetnum] 必为分隔空格

        if (offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if (offsetnum < readnum)
            {
                val_size         = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer, readnum);
            }
            else
            {
                // not finished
                cur_process_type = 1;
                return;
            }
        }
        else
        {
            // not finished
            cur_process_type = 2;
            return;
        }
    }
    else if (cur_process_type == 1)
    {
        if (offsetnum < readnum)
        {
            val_size         = (val_size << 8) + buffer[offsetnum];
            cur_process_type = 10;
            offsetnum++;
            process_value(buffer, readnum);
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if (cur_process_type == 2)
    {
        if (offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if (offsetnum < readnum)
            {
                val_size         = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer, readnum);
            }
            else
            {
                // 本段只送到值长的高字节：留作 1 态，下一段续低字节
                cur_process_type = 1;
                return;
            }
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if (cur_process_type == 10)
    {
        process_value(buffer, readnum);
    }
    else if (cur_process_type == 12)
    {
        if (offsetnum >= readnum)
        {
            return;// 空段：保持 12 态等下一段，不越界、不误断连
        }
        if (buffer[offsetnum] != 0x0A)
        {
            iserror = true;
            return;
        }
        offsetnum++;
        cur_process_type = 0;
        //重新开始kv read kv
    }

    if (cur_process_type == 11)
    {
        if (offsetnum < readnum)
        {
            // 行尾 0x0A 必须存在：跨段与否不改变一条 KV 的合法性判定
            if (buffer[offsetnum] != 0x0A)
            {
                iserror = true;
                return;
            }
            offsetnum++;

            process_headkv();
            cur_process_type = 0;
            read_key.clear();
            read_value.clear();
        }
        else if (offsetnum == readnum)
        {
            process_headkv();
            cur_process_type = 12;
            read_key.clear();
            read_value.clear();
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }

    if (iserror)
    {
        return;
    }

    if (offsetnum < readnum)
    {
        //test the end
        if (buffer[offsetnum] == 0x0A)
        {
            offsetnum++;
            isbody = true;
            return;
        }
    }
    else
    {
        //the end
        return;
    }

    // next
    process_parameter(buffer, readnum);
}
void rpc_parse::process(const unsigned char *buffer, unsigned int readnum)
{
    isbegin   = true;
    offsetnum = 0;

    if (readnum > 3 && buffer[0] == 'r' && buffer[1] == 'p' && buffer[2] == 'c' && buffer[3] == 0x20)
    {
        offsetnum = 4;
    }
    else
    {
        iserror = true;
        return;
    }

    peer->url.clear();
    peer->urlpath.clear();
    peer->querystring.clear();
    peer->header.erase("urlpath");
    peer->pathinfos.clear();
    peer->sendfilename.clear();
    read_key.clear();
    read_value.clear();
    line_bytes       = 0;
    kv_num           = 0;
    kv_bytes         = 0;
    has_host         = false;
    has_size         = false;
    cur_process_type = 20;

    process_line(buffer, readnum);
}

// 请求行跨段解析：'rpc ' 之后、0x0A 之前的字节逐字节处理，跨段时状态与临时串保留，
// 下一段由 process_append 分发回来续传。
// 20=路径段中，21=查询 key 中，22=查询 value 中；查询串按 & 分字段，字段没有 '=' 就整段丢弃。
void rpc_parse::process_line(const unsigned char *buffer, unsigned int readnum)
{
    for (; offsetnum < readnum; offsetnum++)
    {
        unsigned char c = buffer[offsetnum];

        if (c == 0x0A)
        {
            if (cur_process_type == 22 && !read_key.empty())
            {
                // "?x=" 空值也是合法参数（RFC 3986 §3.4），与 '&' 分隔处的落法一致
                if (!is_clean_kv(read_key) || !is_clean_kv(read_value))
                {
                    iserror = true;
                    return;
                }
                peer->get[read_key] = read_value;
                read_key.clear();
            }
            else if (cur_process_type == 20 && !read_key.empty())
            {
                // 20 态行尾 = 路径最后一段；与 h1/h2 同口径，交给统一归一化
                if (!url_segments_normalize(peer->pathinfos, read_key))
                {
                    iserror = true;
                    return;
                }
                read_key.clear();
            }
            else if (!read_key.empty())
            {
                // 21 态行尾悬空 key（"?x" 无 '=' 形态）：keyless 字段，直接丢弃；
                // 不再落进 pathinfos，避免污染 sendfilename 与路由前缀匹配
                read_key.clear();
            }
            read_value.clear();

            // 与 h1/h2 同口径：由归一化后的段表组装路由键
            peer->urlpath.clear();
            if (peer->pathinfos.empty())
            {
                peer->urlpath = "/";
            }
            else
            {
                for (const auto &seg : peer->pathinfos)
                {
                    peer->urlpath.push_back('/');
                    peer->urlpath.append(seg);
                }
            }
            peer->header["urlpath"] = peer->urlpath;
            peer->url               = peer->urlpath;
            if (!peer->querystring.empty())
            {
                peer->url.push_back('?');
                peer->url.append(peer->querystring);
            }
            peer->isfile = !peer->pathinfos.empty() &&
                           peer->pathinfos.back().find('.') != std::string::npos;

            // 临时观测：统计业务流量里是否出现合法 %XX，确认完删除。
            // 会打印原始 query，仅用于测试环境（含敏感参数时注意脱敏）。
            if (is_pct_encoded(peer->querystring))
            {
                std::fprintf(stderr, "rpc pct query: %s?%.*s\n",
                             peer->urlpath.c_str(),
                             (int)peer->querystring.size(), peer->querystring.data());
            }

            offsetnum++;
            cur_process_type = 0;
            after_line(buffer, readnum);
            return;
        }

        if (++line_bytes > CONST_RPC_LINE_MAX)
        {
            iserror = true;
            return;
        }

        if (cur_process_type == 22)
        {
            if (c == '&')
            {
                if (!read_key.empty())
                {
                    if (!is_clean_kv(read_key) || !is_clean_kv(read_value))
                    {
                        iserror = true;
                        return;
                    }
                    peer->get[read_key] = read_value;
                }
                read_value.clear();
                read_key.clear();
                peer->querystring.push_back('&');
                cur_process_type = 21;
                continue;
            }
            read_value.push_back(c);
            peer->querystring.push_back(c);
            continue;
        }

        if (cur_process_type == 20)
        {
            if (c == '?')
            {
                if (!read_key.empty() && !url_segments_normalize(peer->pathinfos, read_key))
                {
                    iserror = true;
                    return;
                }
                read_key.clear();
                cur_process_type = 21;
                continue;
            }
            if (c == '/')
            {
                if (!read_key.empty() && !url_segments_normalize(peer->pathinfos, read_key))
                {
                    iserror = true;
                    return;
                }
                read_key.clear();
                continue;// 不再逐字节 push peer->url，行尾统一组装
            }
            read_key.push_back(c);
            continue;
        }

        // 21：查询 key（'&' 是字段分隔符：此刻还没 '='，本段是 keyless 字段，丢弃；
        // '=' 切到 value）
        if (c == '&')
        {
            read_key.clear();
            peer->querystring.push_back('&');
            continue;
        }
        if (c == '=')
        {
            read_value.clear();
            peer->querystring.push_back('=');
            cur_process_type = 22;
            continue;
        }
        read_key.push_back(c);
        peer->querystring.push_back(c);
    }
    // 段耗尽：直接返回，状态保留
}

// 请求行收尾：拼 sendfilename，然后区分「空行（进 body）」与「KV 段」。
void rpc_parse::after_line(const unsigned char *buffer, unsigned int readnum)
{
    if (peer->pathinfos.size() > 0)
    {
        for (unsigned int j = 0; j < peer->pathinfos.size(); j++)
        {
            if (j > 0)
            {
                peer->sendfilename.push_back('/');
            }
            peer->sendfilename.append(peer->pathinfos[j]);
        }
    }

    if (offsetnum >= readnum)
    {
        return;// 段恰在请求行后结束：KV 段留给下一段
    }

    if (buffer[offsetnum] == 0x0A)
    {
        //test the end
        offsetnum++;
        isbody = true;
    }
    else
    {
        process_parameter(buffer, readnum);
    }
    //in parameter
    if (iserror)
    {
        return;
    }

    if (isbody)
    {
        process_body(buffer, readnum);
    }
}

void rpc_parse::process_append(const unsigned char *buffer, unsigned int readnum)
{
    if (isfinish)
    {
        return;
    }
    if (iserror)
    {
        return;
    }

    // 每个新缓冲区从偏移 0 开始；解析状态由 cur_process_type / isbody 保存
    offsetnum = 0;

    if (isbody)
    {
        process_body(buffer, readnum);
        return;
    }
    if (cur_process_type == 20 || cur_process_type == 21 || cur_process_type == 22)
    {
        process_line(buffer, readnum);
        return;
    }

    process_parameter(buffer, readnum);
    // 对齐 process()：若 process_parameter 读到了空行进入 body 阶段，
    // 需要继续消费当前缓冲区中剩余的 body 字节。
    if (!iserror && isbody)
    {
        process_body(buffer, readnum);
    }
}

void rpc_parse::process_body(const unsigned char *buffer, unsigned int readnum)
{
    if (isfinish)
    {
        return;
    }
    if (iserror)
    {
        return;
    }

    if (body_size == 0)
    {
        isfinish             = true;
        peer->content_length = 0;
        return;
    }

    // 本段可提供的 body 字节数，按剩余量截断
    unsigned int take = 0;
    if (offsetnum < readnum)
    {
        take = readnum - offsetnum;
        if (static_cast<unsigned long long>(take) > static_cast<unsigned long long>(body_sub))
        {
            take = static_cast<unsigned int>(body_sub);
        }
    }

    if (body_size > 16384)
    {
        if (uprawfile == nullptr)
        {
            server_loaclvar &localvar = get_server_global_var();

            // 落盘名统一由 make_http_temp_raw_name() 生成：文件名带 pzraw_ 前缀，
            // 该前缀是 httpwatch 周期清理时识别「框架自己的临时文件」的唯一依据，
            // 不要在此自行拼接文件名，否则文件会长期留在 temp_path 里。
            read_value = localvar.temp_path;
            read_value.append(make_http_temp_raw_name());

            // uprawfile = fopen(upfile.tempfile.c_str(), "wb");
            uprawfile.reset(fopen(read_value.c_str(), "wb"));

            peer->files["tempraw"].set_object();
            peer->files["tempraw"]["name"]     = "tempraw";
            peer->files["tempraw"]["filename"] = "";
            peer->files["tempraw"]["tempfile"] = read_value;
            peer->files["tempraw"]["type"]     = "application/octet-stream";
            peer->files["tempraw"]["size"]     = body_size;
            peer->files["tempraw"]["error"]    = 0;
        }

        if (uprawfile == nullptr)
        {
            iserror = true;
            return;
        }
        if (take > 0)
        {
            if (fwrite(&buffer[offsetnum], take, 1, uprawfile.get()) != 1)
            {
                // 磁盘满/写失败：落盘文件已截断，不能按已收完继续分发
                iserror = true;
                return;
            }
            offsetnum += take;
        }
        body_sub -= take;
        if (body_sub == 0)
        {
            uprawfile.reset(nullptr);
            isfinish             = true;
            peer->content_length = content_size;
        }
    }
    else
    {
        if (take > 0)
        {
            peer->rawcontent.append((char *)&buffer[offsetnum], take);
            offsetnum += take;
        }
        body_sub -= take;
        if (body_sub == 0)
        {
            isfinish             = true;
            peer->content_length = content_size;
        }
    }

    // isfinish 之后本段剩余字节不在此判错：由 client_rpc_loop 前移缓冲头，
    // 下一轮作为下一条请求继续解析（同段/跨段的流水线行为一致）。
    // offsetnum 保留为本次消费到的位置，loop 在 reset() 之前读取它。
}

void rpc_parse::async_send_error()
{
    send_content.clear();
    send_content.append("rpc");
    send_content.push_back(0x20);
    send_content.append("000");
    send_content.push_back(0x20);
    // 错误帧状态文本固定为空：不拼 status_msg，恒空字段将来被赋值也不会成为注入口
    send_content.push_back(0x0A);
    send_content.append("type ");
    send_content.push_back(0x00);
    send_content.push_back(0x09);
    send_content.append("text/html");
    send_content.push_back(0x0A);
    send_content.append("size ");
    send_content.push_back(0x00);
    send_content.push_back(0x01);
    send_content.push_back(0x11);
    send_content.push_back(0x0A);
    send_content.push_back(0x0A);
    send_content.append("rpc server error!");
}

void rpc_parse::build_header()
{
    send_content.clear();
    if (!is_one_line(status_msg))
    {
        iserror = true;
        return;
    }
    send_content.append("rpc");
    send_content.push_back(0x20);

    if (peer)
    {
        if (peer->status_code == 0)
        {
            send_content.append("200");
        }
        else if (peer->status_code < 1000 && peer->status_code > 99)
        {
            send_content.append(std::to_string(peer->status_code));
        }
        else if (peer->status_code < 100 && peer->status_code > 9)
        {
            send_content.push_back('0');
            send_content.append(std::to_string(peer->status_code));
        }
        else if (peer->status_code < 10 && peer->status_code > 0)
        {
            send_content.append("00");
            send_content.append(std::to_string(peer->status_code));
        }
        else
        {
            send_content.append("000");
        }
    }
    else
    {
        send_content.append("000");
    }

    send_content.push_back(0x20);
    send_content.append(status_msg);
    send_content.push_back(0x0A);
    if (!peer)
    {
        iserror = true;
        send_content.push_back(0x0A);
        return;
    }

    if (ischunked)
    {
        send_content.append("chunk ");
        send_content.push_back(0x00);
        send_content.push_back(0x04);
        send_content.append("plan");
    }
    else
    {
        send_content.append("size ");

        if (peer->output.size() <= 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x02);
            unsigned short value_size = peer->output.size();
            unsigned char a           = value_size & 0xFF;
            value_size                = value_size & 0xFF00;
            unsigned char b           = value_size >> 8;
            send_content.push_back(b);
            send_content.push_back(a);
        }
        else if (peer->output.size() <= 0xFFFFFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x04);

            unsigned int value_size = peer->output.size();
            unsigned char a;
            a = (value_size & 0xFF000000) >> 24;
            send_content.push_back(a);
            a = (value_size & 0xFF0000) >> 16;
            send_content.push_back(a);
            a = (value_size & 0xFF00) >> 8;
            send_content.push_back(a);
            a = value_size & 0xFF;
            send_content.push_back(a);
        }
        else if (peer->output.size() <= 0xFFFFFFFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x05);

            unsigned long long value_size = peer->output.size();
            unsigned char a;
            a = (value_size & 0xFF00000000) >> 32;
            send_content.push_back(a);
            a = (value_size & 0xFF000000) >> 24;
            send_content.push_back(a);
            a = (value_size & 0xFF0000) >> 16;
            send_content.push_back(a);
            a = (value_size & 0xFF00) >> 8;
            send_content.push_back(a);
            a = value_size & 0xFF;
            send_content.push_back(a);
        }
        else
        {
            iserror = true;
            return;
        }
    }

    send_content.push_back(0x0A);
    send_content.append("type ");

    if (!is_one_line(peer->content_type))
    {
        iserror = true;
        return;
    }

    if (peer->content_type.size() == 0)
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if (peer->content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");
            iserror = true;
            return;
        }

        unsigned short value_size = peer->content_type.size();
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(peer->content_type);
    }
    send_content.push_back(0x0A);

    std::string temp_h;
    for (const auto &[map_k, map_value] : peer->send_header)
    {
        if (map_value.size() > 0xFFFF || !is_one_line(map_k) || !is_one_line(map_value))
        {
            // 长度前缀只有 2 字节，超出会声明长度与实写字节不一致；CR/LF 会注入 KV 行
            iserror = true;
            return;
        }

        temp_h = map_k;

        for (unsigned int i = 0; i < temp_h.size(); i++)
        {
            if (temp_h[i] >= 'A' && temp_h[i] <= 'Z')
            {
                temp_h[i] += 32;
            }
            else if (temp_h[i] == 0x20)
            {
                temp_h[i] = 0x5F;
            }
        }

        send_content.append(temp_h);
        send_content.push_back(0x20);
        unsigned short value_size = map_value.size();
        unsigned char a           = value_size & 0xFF;
        value_size                = value_size & 0xFF00;
        unsigned char b           = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(map_value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
}

}// namespace http