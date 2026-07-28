#ifndef HTTPS_BROTLI_H
#define HTTPS_BROTLI_H

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <array>
 
namespace http
{
    void brotli_encode(const std::string &data, std::string &out_data);
    void brotli_decode(const std::string &data, std::string &out_data);

    void brotli_encode(const std::string &data, std::string &out_data,unsigned char *buffer,unsigned int buffer_size);
    void brotli_decode(const std::string &data, std::string &out_data,unsigned char *buffer,unsigned int buffer_size);

    // 流式压缩：从 FILE* 分块读取并压缩，避免将整个文件内容加载到内存
    void brotli_encode_file(std::FILE *fp, long long file_size, std::string &out_data);
}

#endif