#ifndef HTTPS_BROTLI_H
#define HTTPS_BROTLI_H

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <array>
 
namespace http
{
    bool brotli_encode(const std::string &data, std::string &out_data);
    bool brotli_decode(const std::string &data, std::string &out_data);

    bool brotli_encode(const std::string &data, std::string &out_data,unsigned char *buffer,unsigned int buffer_size);
    bool brotli_decode(const std::string &data, std::string &out_data,unsigned char *buffer,unsigned int buffer_size);

    // 流式压缩：从 FILE* 分块读取并压缩，避免将整个文件内容加载到内存。
    // 返回 false 表示失败（实例创建失败 / 短读 / 压缩流未完成 / 空结果），调用方必须回退原文。
    // Returns false on failure (no instance / short read / unfinished stream / empty result);
    // the caller MUST fall back to sending the uncompressed original.
    bool brotli_encode_file(std::FILE *fp, long long file_size, std::string &out_data);
}

#endif