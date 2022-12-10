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
}

#endif