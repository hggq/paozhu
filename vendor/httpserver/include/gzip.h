
#ifndef PROJECT_GZIP_H
#define PROJECT_GZIP_H
#include <string>
#include <string_view>
#include <zlib.h>
#include <memory>
#include <vector>
#include <cstddef>

namespace http
{
   constexpr size_t DEFAULT_MAX_DECOMPRESS_SIZE = 256 * 1024 * 1024;

   int compress(const char *in_str, size_t in_len, std::string &out_str, int level);

   bool uncompress(std::string_view compressed_data, std::string &data, size_t max_size = DEFAULT_MAX_DECOMPRESS_SIZE);
   bool uncompress(unsigned char *compressed_data, unsigned int length, std::vector<unsigned char> &data, size_t max_size = DEFAULT_MAX_DECOMPRESS_SIZE);
}
#endif
