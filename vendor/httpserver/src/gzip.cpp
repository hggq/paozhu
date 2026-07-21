#include "gzip.h"
#include <cstring>

namespace http
{
#define CHUNK 16384

int compress(const char *in_str, size_t in_len, std::string &out_str, [[maybe_unused]] int level)
{
    if (!in_str)
        return Z_DATA_ERROR;

    int ret, flush;
    unsigned have;
    z_stream strm;
    int windowBits    = 15;
    int GZIP_ENCODING = 16;

    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    strm.next_in  = Z_NULL;
    strm.avail_in = 0;

    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
        return ret;

    std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream *strm) { (void)deflateEnd(strm); });
    const char *end = in_str + in_len;

    size_t distance = 0;
    do
    {
        distance      = end - in_str;
        strm.avail_in = (distance >= CHUNK) ? CHUNK : (uInt)distance;
        strm.next_in  = (Bytef *)in_str;

        in_str += strm.avail_in;
        flush = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            strm.avail_out = CHUNK;
            strm.next_out  = out;
            ret            = deflate(&strm, flush);
            if (ret == Z_STREAM_ERROR)
                break;
            have = CHUNK - strm.avail_out;
            out_str.append((const char *)out, have);
        } while (strm.avail_out == 0);
        if (strm.avail_in != 0)
            break;

    } while (flush != Z_FINISH);
    if (ret != Z_STREAM_END)
        return Z_STREAM_ERROR;

    return Z_OK;
}

bool uncompress(std::string_view compressed_data, std::string &data, size_t max_size)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
        return false;
    }
    strm.avail_in = (uInt)compressed_data.length();
    strm.next_in  = (unsigned char *)compressed_data.data();
    do
    {
        strm.avail_out = CHUNK;
        strm.next_out  = out;
        ret            = inflate(&strm, Z_NO_FLUSH);
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR: inflateEnd(&strm); return false;
        }
        have = CHUNK - strm.avail_out;
        if (data.size() + have > max_size)
        {
            inflateEnd(&strm);
            return false;
        }
        data.append((char *)out, have);
    } while (strm.avail_out == 0);

    if (inflateEnd(&strm) != Z_OK)
    {
        return false;
    }

    return true;
}

bool uncompress(unsigned char *compressed_data, unsigned int length, std::vector<unsigned char> &data, size_t max_size)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
        return false;
    }
    strm.avail_in = length;
    strm.next_in  = compressed_data;
    do
    {
        strm.avail_out = CHUNK;
        strm.next_out  = out;
        ret            = inflate(&strm, Z_NO_FLUSH);
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR: inflateEnd(&strm); return false;
        }
        have = CHUNK - strm.avail_out;
        if (data.size() + have > max_size)
        {
            inflateEnd(&strm);
            return false;
        }
        data.insert(data.end(), out, out + have);
    } while (strm.avail_out == 0);

    if (inflateEnd(&strm) != Z_OK)
    {
        return false;
    }

    return true;
}
}

