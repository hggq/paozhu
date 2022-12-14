//
// Description: WebSocket RFC6544 codec, written in C++.
//

#ifndef PROJECT_WEBSOCKET_PARSE_H
#define PROJECT_WEBSOCKET_PARSE_H
#include <memory>
#include <cstddef>
#include <string>
#include <string_view>
namespace http
{
    enum FRAMETYPE
    {
        CONT = 0x00,
        CONT_TEXT = 0x01,
        CONT_BINARY = 0x02,

        WS_TEXT = 0x81,
        WS_BINARY = 0x82,

        WS_CLOSE = 0x88,

        WS_PING = 0x89,
        WS_PONG = 0x8A
    };

    class websocketparse
    {
    public:
        websocketparse();
        ~websocketparse();
        // 解析 WebSocket 的握手数据
        bool parseHandshake(const std::string &request);

        // 应答 WebSocket 的握手
        std::string respondHandshake();
        // 应答 WebSocket 的握手
        void setWebsocketkey(const std::string &request);

        // 取得数据包长度
        unsigned long long getprocssdata(unsigned char *inputdata, unsigned int buffersize);

        void parsedata(unsigned char *inputdata, unsigned int buffersize);
        // 封装 WebSocket 协议的数据，服务器-->客户端
        int makeWSData(char *msg, int msgLen, std::string &outBuf);
        int makeWSText(char *msg, int msgLen, std::string &outBuf);
        int makeWSData(std::string_view, std::string &outBuf);
        int makeWSText(std::string_view, std::string &outBuf);
        // 封装 WebSocket 协议的数据头（二进制数据）
        static int makeWSHeader(int len, std::string &header, unsigned char);
        std::string getWebsocketkey();

        std::string makeClose();
        std::string makePong();
        std::string makePing();

        void closefile();

    private:
        std::string websocketKey_; // 握手中客户端发来的key
        FRAMETYPE state;

    public:
        bool isfinish = false;
        std::string indata;
        bool islock = false;
        bool isopen = false; // 是否还活着
        bool isfile = false;
        bool ispack = false;
        unsigned char fin = 0x0B;
        unsigned char opcode;
        unsigned int pos = 0;
        unsigned long long contentlength=0;
        unsigned long long readlength=0;
        unsigned long long contentoffset=0;
        // FILE *rawfile; // 临时文件
        std::unique_ptr<std::FILE, decltype(&std::fclose)> rawfile;
        std::string filename;
        unsigned char mask;
        unsigned char mask_key[4];
    };
}
#endif // PROJECT_WEBSOCKET_H
