

#ifndef PROJECT_GZIP_H
#define PROJECT_GZIP_H
#include <string>
#include <string_view>
#include <zlib.h>
#include <memory>

namespace http
{
   /* Compress from file source to file dest until EOF on source.
      def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
      allocated for processing, Z_STREAM_ERROR if an invalid compression
      level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
      version of the library linked do not match, or Z_ERRNO if there is
      an error reading or writing the files. */
   int compress(const char *in_str, size_t in_len, std::string &out_str, int level);

   // GZip Decompression
   // @param compressed_data - the gzip compressed data
   // @param data - the resulting uncompressed data (may contain binary data)
   // @return - true on success, false on failure
   bool uncompress(std::string_view compressed_data, std::string &data);
}
#endif