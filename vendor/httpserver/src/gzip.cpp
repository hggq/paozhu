

#ifndef PROJECT_GZIP_H
#define PROJECT_GZIP_H

#include <string_view>
#include <zlib.h>
#include <memory>
#include <string>
#include <vector>
#include "gzip.h"

namespace http
{
#define CHUNK 16384
  /* Compress from file source to file dest until EOF on source.
     def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
     allocated for processing, Z_STREAM_ERROR if an invalid compression
     level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
     version of the library linked do not match, or Z_ERRNO if there is
     an error reading or writing the files. */
  int compress(const char *in_str, size_t in_len,
               std::string &out_str, int level)
  {
    if (!in_str)
      return Z_DATA_ERROR;

    int ret, flush;
    unsigned have;
    z_stream strm;
    int windowBits = 15;
    int GZIP_ENCODING = 16;

    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.next_in = Z_NULL; // inflateInit和inflateInit2都必须初始化next_in和avail_in
    strm.avail_in = 0;     // deflateInit和deflateInit2则不用

    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
      return ret;

    std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream *strm)
                                      { (void)deflateEnd(strm); });
    const char *end = in_str + in_len;

    //size_t pos_index = 0;
    size_t distance = 0;
    /* compress until end of file */
    do
    {
      distance = end - in_str;
      strm.avail_in = (distance >= CHUNK) ? CHUNK : distance;
      strm.next_in = (Bytef *)in_str;

      // next pos
      in_str += strm.avail_in;
      flush = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

      /* run deflate() on input until output buffer not full, finish
         compression if all of source has been read in */
      do
      {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = deflate(&strm, flush); /* no bad return value */
        if (ret == Z_STREAM_ERROR)
          break;
        have = CHUNK - strm.avail_out;
        out_str.append((const char *)out, have);
      } while (strm.avail_out == 0);
      if (strm.avail_in != 0) /* all input will be used */
        break;

      /* done when last data in file processed */
    } while (flush != Z_FINISH);
    if (ret != Z_STREAM_END) /* stream will be complete */
      return Z_STREAM_ERROR;

    /* clean up and return */
    return Z_OK;
  }

  // GZip Decompression
  // @param compressed_data - the gzip compressed data
  // @param data - the resulting uncompressed data (may contain binary data)
  // @return - true on success, false on failure
  bool uncompress(std::string_view compressed_data, std::string &data)
  {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
      return false;
    }
    strm.avail_in = (uInt)compressed_data.length();
    strm.next_in = (unsigned char *)compressed_data.data();
    do
    {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret)
      {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:

        inflateEnd(&strm);
        return false;
      }
      have = CHUNK - strm.avail_out;
      data.append((char *)out, have);
    } while (strm.avail_out == 0);

    if (inflateEnd(&strm) != Z_OK)
    {
      return false;
    }

    return true;
  }
  bool uncompress(unsigned char *compressed_data, unsigned int length, std::vector<unsigned char> &data)
  {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
      return false;
    }
    strm.avail_in = length;
    strm.next_in = compressed_data;
    do
    {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret)
      {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:

        inflateEnd(&strm);
        return false;
      }
      have = CHUNK - strm.avail_out;
      data.assign((char *)out, (char *)out + have);
    } while (strm.avail_out == 0);

    if (inflateEnd(&strm) != Z_OK)
    {
      return false;
    }

    return true;
  }
}
#endif