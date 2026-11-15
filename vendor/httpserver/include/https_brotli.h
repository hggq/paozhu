#ifndef HTTPS_BROTLI_H
#define HTTPS_BROTLI_H

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <array>
 
namespace http
{
    // 每请求生成、无法缓存的动态响应（server.cpp 的 peer->output）专用。
    // 用低 quality / 小窗口换可预测的每请求延迟：默认 q11/lgwin22 对 4-8KB
    // 的 body 要 9-19ms，会把 io 线程打满；q1 约 30us。
    // 静态资源仍用 brotli_encode() / brotli_encode_file() 的库默认高档，
    // 它们压一次可长期复用，压缩率比单请求延迟更重要。
    // Per-request dynamic output only. Cacheable assets keep the library
    // defaults (quality 11), where the one-off cost buys a better ratio.
    // Level is set by PAOZHU_BROTLI_DYNAMIC_QUALITY / _LGWIN in common/cost_define.h (override with -D).
    bool brotli_encode_dynamic(const std::string &data, std::string &out_data);

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