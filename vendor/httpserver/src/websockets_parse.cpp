#include <cmath>
#include <cstddef>
#include <openssl/sha.h>//for SHA1
//#include <string.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "websockets_parse.h"
#include "base64.h"
#include "server_localvar.h"

namespace http
{

websocketparse::websocketparse() : rawfile(nullptr, std::fclose)
{
    // rawfile = NULL;
}
websocketparse::~websocketparse()
{
    if (rawfile)
    {
        // fclose(rawfile);
        // rawfile = NULL;
        rawfile.reset(nullptr);
    }
}
unsigned long long websocketparse::getprocssdata(unsigned char *inputdata, unsigned int buffersize)
{

    fin                     = inputdata[0] >> 7 & 0x01;
    opcode                  = inputdata[0] & 0x0F;
    mask                    = inputdata[1] >> 7 & 0x01;
    unsigned char fixlength = inputdata[1] & 0x7F;
    pos                     = 2;
    isfinish                = false;
    if (fin > 0)
    {
        closefile();
        indata.clear();
        isfile = false;
        ispack = false;
    }
    // unsigned char mask_key[4];
    if (fixlength < 126)
    {
        contentlength = fixlength;
    }
    else if (fixlength == 126)
    {
        contentlength = inputdata[pos] << 8 | (unsigned char)inputdata[pos + 1];
        pos += 2;
    }
    else if (fixlength == 127)
    {
        contentlength = (unsigned char)inputdata[pos];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 1];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 2];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 3];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 4];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 5];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 6];
        contentlength = contentlength << 8 | (unsigned char)inputdata[pos + 7];
        pos += 8;
    }
    if (mask == 1)
    {
        mask_key[0] = inputdata[pos];
        mask_key[1] = inputdata[pos + 1];
        mask_key[2] = inputdata[pos + 2];
        mask_key[3] = inputdata[pos + 3];
        pos += 4;
    }
    contentoffset = buffersize - pos;
    ispack        = true;

    if (contentlength > 2097152)
    {
        http::server_loaclvar &localvar = http::get_server_global_var();
        isfile                          = true;
        filename                        = localvar.temp_path + "ws_" + std::to_string(time(NULL)) + "_" + std::to_string(contentlength);
        // rawfile = fopen(filename.c_str(), "wb");
        rawfile.reset(fopen(filename.c_str(), "wb"));
        if (rawfile)
        {

            if (mask == 1)
            {

                for (unsigned int j = 0; j < contentoffset; j++)
                {
                    unsigned char nn   = j % 4;
                    inputdata[j + pos] = inputdata[j + pos] ^ mask_key[nn];
                }
                fwrite(&inputdata[pos], contentoffset, 1, rawfile.get());
            }
            else
            {
                for (unsigned int j = 0; j < contentoffset; j++)
                {
                    fwrite(&inputdata[j + pos], 1, 1, rawfile.get());
                }
            }
        }
    }
    else
    {
        isfile = false;

        if (mask == 1)
        {

            for (unsigned int j = 0; j < contentoffset; j++)
            {
                unsigned char nn = j % 4;
                char b           = inputdata[j + pos] ^ mask_key[nn];
                indata.push_back(b);
            }
        }
        else
        {
            for (unsigned int j = 0; j < contentoffset; j++)
            {
                indata.push_back(inputdata[j + pos]);
            }
        }
    }

    if (contentoffset >= contentlength)
    {
        isfinish      = true;
        ispack        = false;
        contentlength = 0;
        contentoffset = 0;
    }

    return contentlength;
}
void websocketparse::parsedata(unsigned char *inputdata, unsigned int buffersize)
{

    contentoffset += buffersize;

    if (isfile && rawfile)
    {

        if (mask == 1)
        {
            for (unsigned int j = 0; j < buffersize; j++)
            {
                unsigned char nn = j % 4;
                inputdata[j]     = inputdata[j] ^ mask_key[nn];
            }
            fwrite(inputdata, buffersize, 1, rawfile.get());
        }
        else
        {
            fwrite(inputdata, buffersize, 1, rawfile.get());
        }
    }
    else
    {
        if (mask == 1)
        {
            for (unsigned int j = 0; j < buffersize; j++)
            {
                unsigned char nn = j % 4;
                char b           = inputdata[j] ^ mask_key[nn];
                indata.push_back(b);
                std::cout << b << std::endl;
            }
        }
        else
        {
            for (unsigned int j = 0; j < buffersize; j++)
            {
                indata.push_back(inputdata[j]);
            }
        }
    }

    if (contentoffset >= contentlength)
    {
        isfinish      = true;
        ispack        = false;
        contentlength = 0;
        contentoffset = 0;
    }
}
void websocketparse::setWebsocketkey(const std::string &key)
{
    websocketKey_ = key;
}
void websocketparse::closefile()
{
    if (isfile)
    {
        // fclose(rawfile);
        // rawfile = NULL;
        rawfile.reset(nullptr);
    }
}
std::string websocketparse::getWebsocketkey()
{
    return websocketKey_;
}
std::string websocketparse::respondHandshake()
{
    // 算出WEBSOCKET响应信息
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    response += "Connection: Upgrade\r\n";
    response += "Sec-WebSocket-Accept: ";

    // 使用请求传过来的KEY+协议字符串，先用SHA1加密然后使用base64编码算出一个应答的KEY
    const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::string serverKey = websocketKey_ + magicKey;

    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)serverKey.c_str(), serverKey.length(), (unsigned char *)&digest);

    response = response + http::base64_encode((char *)digest, SHA_DIGEST_LENGTH, false) + "\r\n\r\n";
    return response;
}

std::string websocketparse::makePong()
{
    std::string reping;
    reping.resize(6);
    reping[0] = 0x8A;
    reping[1] = 0x04;
    reping[2] = 'p';
    reping[3] = 'o';
    reping[4] = 'n';
    reping[5] = 'g';
    return reping;
}
std::string websocketparse::makePing()
{
    std::string reping;
    reping.resize(6);
    reping[0] = 0x89;
    reping[1] = 0x04;
    reping[2] = 'p';
    reping[3] = 'i';
    reping[4] = 'n';
    reping[5] = 'g';
    return reping;
}
int websocketparse::makeWSData(char *msg, int msgLen, std::string &outBuf)
{
    // std::string header;
    makeWSHeader(msgLen, outBuf, 0x02);
    // outBuf.insert(outBuf.begin(), header.begin(), header.end());
    // outBuf.insert(outBuf.end(), msg, msg+msgLen);
    for (int i = 0; i < msgLen; i++)
    {
        outBuf.push_back(msg[i]);
    }
    return 0;
}
int websocketparse::makeWSText(char *msg, int msgLen, std::string &outBuf)
{
    // std::string header;
    makeWSHeader(msgLen, outBuf, 0x01);
    // outBuf.insert(outBuf.begin(), header.begin(), header.end());
    // outBuf.insert(outBuf.end(), msg, msg+msgLen);
    for (int i = 0; i < msgLen; i++)
    {
        outBuf.push_back(msg[i]);
    }
    return 0;
}

int websocketparse::makeWSData(std::string_view msg, std::string &outBuf)
{
    makeWSHeader(msg.length(), outBuf, 0x02);
    for (unsigned int i = 0; i < msg.length(); i++)
    {
        outBuf.push_back(msg[i]);
    }
    return 0;
}
int websocketparse::makeWSText(std::string_view msg, std::string &outBuf)
{
    makeWSHeader(msg.length(), outBuf, 0x01);
    for (unsigned int i = 0; i < msg.length(); i++)
    {
        outBuf.push_back(msg[i]);
    }
    return 0;
}
int websocketparse::makeWSHeader(int len, std::string &header, unsigned char t)
{
    switch (t)
    {
    case 0x01:
        header.push_back(0x81);
        break;
    case 0x02:
        header.push_back(0x82);
        break;
    case 0x80:
        header.push_back(0x80);
        break;
    case 0x81:
        header.push_back(0x81);
        break;
    case 0x82:
        header.push_back(0x82);
        break;
    default:
        header.push_back(t);
    }

    if (len <= 125)
    {
        header.push_back((char)len);
    }
    else if (len <= 65535)
    {
        header.push_back((char)126);                // 16 bit length follows
        header.push_back((char)((len >> 8) & 0xFF));// leftmost first
        header.push_back((char)(len & 0xFF));
    }
    else// >2^16-1 (   )
    {
        header.push_back((char)127);// 64 bit length follows

        // write 8 bytes length (significant first)
        // since msg_length is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for (int i = 3; i >= 0; i--)
        {
            header.push_back((char)0);
        }
        // write the actual 32bit msg_length in the next 4 bytes
        for (int i = 3; i >= 0; i--)
        {
            header.push_back((char)((len >> 8 * i) & 0xFF));
        }
    }

    return 0;
}

}// namespace http