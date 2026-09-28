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

namespace http
{
    
rpc_parse::rpc_parse(): uprawfile(nullptr, std::fclose) { iserror = false; }
void rpc_parse::set_chunk(bool c)
{
    ischunked = c;
} 
void rpc_parse::reset()
{
    isfinish=false;
    isbegin =false;
    isbody =false;
    iserror = false;
    cur_process_type = 0;
    offsetnum = 0;
    val_size = 0;
    body_size = 0;
    content_size = 0;

    read_key.clear();   //read temp var in the header
    read_value.clear();
    read_temp_file.clear();
    send_content.shrink_to_fit();
    uprawfile=nullptr;
}
void rpc_parse::process_headkv()
{
    // 除 size 外（其值是 body 长度的大端二进制），其余头的 value 必须是文本
    if(!(read_key.size()==4 && read_key == "size"))
    {
        for(unsigned int i = 0; i < read_value.size(); i++)
        {
            unsigned char c = read_value[i];
            if(c != 0x09 && (c < 0x20 || c > 0x7E))
            {
                iserror = true;
                return;
            }
        }
    }

    if(read_key.size()==4 && read_key == "HOST")
    {
        peer->host = read_value;
    }
    else if(read_key.size()==6 && read_key == "cookie")
    {
        read_key.clear();
        unsigned int i = 0;
        for(; i < read_value.size(); i++)
        {
            if(read_value[i]=='=')
            {
                break;
            }
            read_key.push_back(read_value[i]);  
        }

        if(read_key.empty())
        {
            //ignore
            return;
        }

        if(read_value[i]=='=')
        {
            i++;
            if(i < read_value.size())
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
    else if(read_key.size()==4 && read_key == "size")
    {
        unsigned int i = 0;
        body_size = 0;
        for(; i < read_value.size(); i++)
        {
           body_size = (body_size <<8) + read_value[i];
        }
        content_size = body_size;
        body_sub = body_size;
        if(content_size > 68719476735)
        {
            //to long long(>15G)
            iserror = true;
            return;
        }
    }
    else
    {
        peer->post[read_key] = read_value;
    }
}
void rpc_parse::process_value(const unsigned char *buffer, unsigned int readnum)
{
    for(; offsetnum < readnum; offsetnum++)
    {
        read_value.push_back(buffer[offsetnum]);
        val_size--; 
        if(val_size == 0)
        {
            cur_process_type = 11;
            offsetnum++;
            return;
        }
    }
}   

void rpc_parse::process_parameter(const unsigned char *buffer, unsigned int readnum)
{
    if(cur_process_type == 0)
    {
        for(unsigned int i = 0; offsetnum < readnum; offsetnum++)
        {
            if(buffer[offsetnum]==0x20)
            {
                break;
            }
            // header key 必须是可打印 ASCII（不含空格与控制字符）
            if(buffer[offsetnum] < 0x21 || buffer[offsetnum] > 0x7E)
            {
                iserror = true;
                return;
            }
            read_key.push_back(buffer[offsetnum]);
            i++;
            if(i > 128)
            {
                iserror = true;
                return;
            }
        }
        
        if(read_key.size() > 128)
        {
            iserror = true;
            return;
        }
        if(buffer[offsetnum]==0x20)
        {
           offsetnum++;  
        }
        else
        {
            // not finished
            cur_process_type = 0;
            return;
        }

        
        if(offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if(offsetnum < readnum)
            {
                val_size =(val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer,readnum);
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
    else if(cur_process_type == 1)
    {
        if(offsetnum < readnum)
        {
            val_size = (val_size << 8) + buffer[offsetnum];
            cur_process_type = 10;
            offsetnum++;
            process_value(buffer,readnum);
        }
        else
        {
            // not finished
            iserror = true;
            return;
        }
    }
    else if(cur_process_type == 2)
    {
        if(offsetnum < readnum)
        {
            val_size = buffer[offsetnum];
            offsetnum++;
            if(offsetnum < readnum)
            {
                val_size = (val_size << 8) + buffer[offsetnum];
                cur_process_type = 10;
                offsetnum++;
                process_value(buffer,readnum);
            }
            else
            {
                // not finished
                iserror = true;
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
    else if(cur_process_type == 10)
    {
        process_value(buffer,readnum);
    }
    else if(cur_process_type == 12)
    {
        //值还没有读到空格 The value has not yet reached the space
        if(buffer[offsetnum]==0x0A)
        {
            offsetnum++;  
        }
        else
        {
            iserror = true;
            return;
        }
        cur_process_type = 0;
        //重新开始kv read kv 
    }

    if(cur_process_type == 11)
    {
        if(offsetnum < readnum)
        {
            if(buffer[offsetnum]==0x0A)
            {
                offsetnum++;  
            }

            process_headkv();
            cur_process_type = 0;
            read_key.clear();
            read_value.clear();
        }
        else if(offsetnum == readnum)
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

    if(offsetnum < readnum)
    {
        //test the end
        if(buffer[offsetnum]==0x0A)
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
    process_parameter(buffer,readnum);

}
void rpc_parse::process(const unsigned char *buffer, unsigned int readnum)
{
    isbegin = true;
    offsetnum = 0;
     
    if(readnum > 3 && buffer[0]=='r' && buffer[1]=='p' && buffer[2]=='c' && buffer[3]==0x20)
    {
        offsetnum = 4;
    }
    else
    {
        iserror = true;
        return;
    }

    peer->url.clear();
    read_key.clear();
    read_value.clear();
    
    for(unsigned int i = 0; offsetnum < readnum; offsetnum++)
    {
        if(buffer[offsetnum]==0x0A)
        {
            break;
        }
        if(buffer[offsetnum]=='/')
        {
            if(read_key.size() > 0)
            {
                peer->pathinfos.push_back(read_key);
            }
            read_key.clear();
        }
        else
        {
            if(buffer[offsetnum]=='?')
            {
                if(read_key.size() > 0)
                {
                    peer->pathinfos.push_back(read_key);
                }
                read_key.clear();
                offsetnum++; // 跳过 '?'，避免其进入第一个 key
                for(; offsetnum < readnum; offsetnum++)
                {
                    if(buffer[offsetnum]==0x0A)
                    {
                        break;
                    }
                    if(buffer[offsetnum]=='=')
                    {
                        read_value.clear();
                        offsetnum++; // 跳过 '='，避免其进入 value
                        for(; offsetnum < readnum; offsetnum++)
                        {
                            if(buffer[offsetnum]==0x0A)
                            {
                                break;
                            }
                            if(buffer[offsetnum]=='&')
                            {
                                if(read_key.size() > 0)
                                {
                                    peer->get[read_key] = read_value;
                                }
                                read_value.clear();
                                read_key.clear();
                                break;
                            }
                            read_value.push_back(buffer[offsetnum]); 
                        }
                        if(read_value.size() > 0)
                        {
                            if(read_key.size() > 0)
                            {
                                peer->get[read_key] = read_value;
                            }
                            read_value.clear();
                            read_key.clear();
                        }

                        if(buffer[offsetnum]==0x0A)
                        {
                            break;
                        }

                        continue;
                    }
                    read_key.push_back(buffer[offsetnum]); 
                }
                break;
            }
            read_key.push_back(buffer[offsetnum]); 
        }

        peer->url.push_back(buffer[offsetnum]);
        i++;
        if(i > 256)
        {
            iserror = true;
            return;
        }
    }

    if(read_key.size() > 0)
    {
        peer->pathinfos.push_back(read_key);
    }
    read_key.clear();
    
    if(peer->pathinfos.size() > 0)
    {
        for(unsigned int j=0;j< peer->pathinfos.size(); j++)
        {
            if(j > 0)
            {
                peer->sendfilename.push_back('/');
            }
            peer->sendfilename.append(peer->pathinfos[j]);
        }
    }

    if(buffer[offsetnum]!=0x0A)
    {
        iserror = true;
        return;
    }
    offsetnum++;
    if(offsetnum >= readnum)
    {
        //is not begin
        iserror = true;
        return;
    }

    if(buffer[offsetnum]==0x0A)
    {
        //test the end
        offsetnum++;
        isbody = true;
    }
    else
    {
        process_parameter(buffer,readnum);    
    }
    //in parameter
    if(iserror)
    {
        return;
    }

    if(isbody)
    {
        process_body(buffer,readnum);   
    }
}

void rpc_parse::process_append(const unsigned char *buffer, unsigned int readnum)
{
    if(isfinish)
    {
        return;
    }
    if(iserror)
    {
        return;
    }

    // 每个新缓冲区从偏移 0 开始；解析状态由 cur_process_type / isbody 保存
    offsetnum = 0;

    if(isbody)
    {
        process_body(buffer,readnum);   
    }
    else
    {
        process_parameter(buffer,readnum);
        // 对齐 process()：若 process_parameter 读到了空行进入 body 阶段，
        // 需要继续消费当前缓冲区中剩余的 body 字节。
        if(!iserror && isbody)
        {
            process_body(buffer,readnum);
        }
    }    
}

void rpc_parse::process_body(const unsigned char *buffer, unsigned int readnum)
{
    if(isfinish)
    {
        return;
    }
    if(iserror)
    {
        return;
    }

    if(body_size == 0)
    {
        isfinish = true;
        peer->content_length = 0;
        return;
    } 
    if(body_size > 16384)
    {
        if(uprawfile == nullptr)
        {
            server_loaclvar &localvar = get_server_global_var();
            read_key = std::to_string(timeid()) + std::to_string(std::hash<std::string>{}(peer->url)) + std::to_string(rand_range(1000, 9999));

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

        if(uprawfile == nullptr)
        {
            //
            iserror = true;
            return;
        }
        if(offsetnum < readnum)
        {
            fwrite(&buffer[offsetnum], (readnum - offsetnum), 1, uprawfile.get());
            offsetnum = readnum;
        }
        else if(offsetnum == readnum)
        {
            return;
        }
        else
        {
            //
            iserror = true;
            return;
        }
        body_sub = body_sub - (readnum - offsetnum);
        if(body_sub < 0)
        {
            // too conent
            iserror = true;
            uprawfile.reset(nullptr);
            return;
        }
        if(body_sub == 0)
        {
            uprawfile.reset(nullptr);
            isfinish = true;
            return;
        }
    }
    else
    {
        if(offsetnum < readnum)
        {
            peer->rawcontent.append((char *)&buffer[offsetnum], (readnum - offsetnum));
            offsetnum = readnum;
            if(content_size >= 0 && peer->rawcontent.size() == static_cast<size_t>(content_size))
            {
                isfinish = true;
                peer->content_length = content_size;
                return;
            }
        }
        else if(offsetnum == readnum)
        {
            return;
        }
        else
        {
            //read error
            iserror = true;
            return;
        }

        body_sub = body_sub - (readnum - offsetnum);
        if(body_sub < 0)
        {
            // too conent
            iserror = true;
            return;
        }
        
        if(body_sub == 0)
        {
            isfinish = true;
            peer->content_length = content_size;
            return;
        }
    }
}

void rpc_parse::async_send_error()
{
    send_content.clear();
    send_content.append("rpc");
    send_content.append("000");
    send_content.push_back(0x20);
    send_content.append(status_msg);
    send_content.push_back(0x0A);
    send_content.append("type");
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
    send_content.append("rpc");
    send_content.push_back(0x20);

    if(peer)
    {
        if(peer->status_code == 0)
        {
            send_content.append("200");
        }
        else if(peer->status_code < 1000 && peer->status_code > 99)
        {
            send_content.append(std::to_string(peer->status_code));
        }
        else if(peer->status_code < 100 && peer->status_code > 9)
        {
            send_content.push_back('0');
            send_content.append(std::to_string(peer->status_code));
        }
        else if(peer->status_code < 10 && peer->status_code > 0)
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
    if(!peer)
    {
        iserror = true;
        send_content.push_back(0x0A);
        return;
    }

    if(ischunked)
    {
        send_content.append("chunk ");
        send_content.push_back(0x00);
        send_content.push_back(0x04);
        send_content.append("plan");
    }
    else
    {
        send_content.append("size ");
        
        if(peer->output.size() <=  0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x02);
            unsigned short value_size = peer->output.size();
            unsigned char a = value_size & 0xFF;
            value_size = value_size & 0xFF00;
            unsigned char b = value_size >> 8;
            send_content.push_back(b);
            send_content.push_back(a);
        }
        else if(peer->output.size() <=  0xFFFFFFFF)
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
        else if(peer->output.size() <=  0xFFFFFFFFFF)
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

    if(peer->content_type.size() ==0 )
    {
        send_content.push_back(0x00);
        send_content.push_back(0x09);
        send_content.append("text/html");
    }
    else
    {
        if(peer->content_type.size() > 0xFFFF)
        {
            send_content.push_back(0x00);
            send_content.push_back(0x09);
            send_content.append("text/html");               
            iserror = true;
            return;
        }

        send_content.push_back(0x00);
        send_content.push_back(0x02);
        unsigned short value_size = peer->content_type.size();
        unsigned char a = value_size & 0xFF;
        value_size = value_size & 0xFF00;
        unsigned char b = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(peer->content_type);
    }
    send_content.push_back(0x0A);

    std::string temp_h;
    for(const auto& [map_k, map_value]:peer->send_header )
    {
        temp_h = map_k;

        for (unsigned int i=0;i< temp_h.size(); i++) 
        {
            if (temp_h[i] >= 'A' && temp_h[i] <= 'Z') 
            {
                temp_h[i] += 32;
            }
            else if(temp_h[i] == 0x20)
            {
                temp_h[i] = 0x5F;
            }
        }

        send_content.append(temp_h);
        send_content.push_back(0x20);
        unsigned short value_size = map_value.size();
        unsigned char a = value_size & 0xFF;
        value_size = value_size & 0xFF00;
        unsigned char b = value_size >> 8;
        send_content.push_back(b);
        send_content.push_back(a);
        send_content.append(map_value);
        send_content.push_back(0x0A);
    }
    send_content.push_back(0x0A);
}

} //end http