#include <string>
#include <array>
#include "https_brotli.h"

namespace http
{
    bool brotli_encode(const std::string &data, std::string &out_data)
    {
        std::array<unsigned char, 2048> buffer;
        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return false;

        size_t available_in = data.length(), available_out = buffer.size();
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer.data();
        bool ok = true;

        do
        {
            if (!BrotliEncoderCompressStream(
                    instance, BROTLI_OPERATION_FINISH,
                    &available_in, &next_in, &available_out, &next_out, nullptr))
            {
                ok = false;
                break;
            }
            out_data.append(reinterpret_cast<const char *>(buffer.data()), buffer.size() - available_out);
            available_out = buffer.size();
            next_out = buffer.data();
        } while (!(available_in == 0 && BrotliEncoderIsFinished(instance)));

        BrotliEncoderDestroyInstance(instance);
        if (!ok)
        {
            out_data.clear();
            return false;
        }
        return true;
    }

    bool brotli_encode(const std::string &data, std::string &out_data, unsigned char *buffer, unsigned int buffer_size)
    {
        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return false;

        size_t available_in = data.length(), available_out = buffer_size;
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer;
        bool ok = true;

        do
        {
            if (!BrotliEncoderCompressStream(
                    instance, BROTLI_OPERATION_FINISH,
                    &available_in, &next_in, &available_out, &next_out, nullptr))
            {
                ok = false;
                break;
            }
            out_data.append(reinterpret_cast<const char *>(buffer), buffer_size - available_out);
            available_out = buffer_size;
            next_out = buffer;
        } while (!(available_in == 0 && BrotliEncoderIsFinished(instance)));

        BrotliEncoderDestroyInstance(instance);
        if (!ok)
        {
            out_data.clear();
            return false;
        }
        return true;
    }

    bool brotli_encode_file(std::FILE *fp, long long file_size, std::string &out_data)
    {
        if (!fp || file_size <= 0)
            return false;

        constexpr unsigned int CHUNK = 16384;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return false;

        long long remaining = file_size;
        BrotliEncoderOperation op = BROTLI_OPERATION_PROCESS;
        bool ok = true;

        do
        {
            size_t chunk = (remaining > (long long)CHUNK) ? CHUNK : (size_t)remaining;
            size_t nread = fread(in, 1, chunk, fp);
            // 短读＝文件在 stat 之后被截断或读错误。绝不能把不完整的输入当成功收尾，
            // 否则会得到一个"能解压但内容缺失"的 br 流并写进缓存长期命中。
            // Short read = file truncated after stat, or a read error. Never finish a stream
            // from incomplete input: it yields a decodable but truncated br payload that then
            // gets cached and served forever.
            if (nread != chunk)
            {
                ok = false;
                break;
            }
            remaining -= (long long)nread;

            size_t available_in  = nread;
            const unsigned char *next_in = in;
            if (remaining == 0)
                op = BROTLI_OPERATION_FINISH;

            do
            {
                size_t available_out = CHUNK;
                unsigned char *next_out = out;
                if (!BrotliEncoderCompressStream(instance, op,
                                                 &available_in, &next_in, &available_out, &next_out, nullptr))
                {
                    ok = false;
                    break;
                }
                out_data.append(reinterpret_cast<const char *>(out), CHUNK - available_out);
            } while (available_in > 0 || (op == BROTLI_OPERATION_FINISH && !BrotliEncoderIsFinished(instance)));

            if (!ok)
                break;
        } while (op != BROTLI_OPERATION_FINISH || !BrotliEncoderIsFinished(instance));

        if (!BrotliEncoderIsFinished(instance))
            ok = false;

        BrotliEncoderDestroyInstance(instance);
        return ok && !out_data.empty();
    }

    bool brotli_decode(const std::string &data, std::string &out_data)
    {
        std::array<unsigned char, 2048> buffer;
        auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return false;

        size_t available_in = data.length(), available_out = buffer.size();
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer.data();
        bool ok = true;

        for (;;)
        {
            size_t before_in  = available_in;
            size_t before_out = available_out;
            BrotliDecoderResult per_result = BrotliDecoderDecompressStream(
                instance,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer.data()), buffer.size() - available_out);
            available_out = buffer.size();
            next_out = buffer.data();

            if (per_result == BROTLI_DECODER_RESULT_SUCCESS)
                break;
            // 截断的输入会让解码器既不报错也不再消费/产出，再转下去就是空转
            // A truncated input leaves the decoder neither erroring nor making progress;
            // looping again would just spin.
            if (per_result == BROTLI_DECODER_RESULT_ERROR ||
                (before_in == available_in && before_out == available_out))
            {
                ok = false;
                break;
            }
        }

        BrotliDecoderDestroyInstance(instance);
        if (!ok)
        {
            out_data.clear();
            return false;
        }
        return true;
    }
    bool brotli_decode(const std::string &data, std::string &out_data, unsigned char *buffer, unsigned int buffer_size)
    {
        auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return false;

        size_t available_in = data.length(), available_out = buffer_size;
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer;
        bool ok = true;

        for (;;)
        {
            size_t before_in  = available_in;
            size_t before_out = available_out;
            BrotliDecoderResult per_result = BrotliDecoderDecompressStream(
                instance,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer), buffer_size - available_out);
            available_out = buffer_size;
            next_out = buffer;

            if (per_result == BROTLI_DECODER_RESULT_SUCCESS)
                break;
            if (per_result == BROTLI_DECODER_RESULT_ERROR ||
                (before_in == available_in && before_out == available_out))
            {
                ok = false;
                break;
            }
        }

        BrotliDecoderDestroyInstance(instance);
        if (!ok)
        {
            out_data.clear();
            return false;
        }
        return true;
    }
}