#include <string>
#include <array>
#include "https_brotli.h"

namespace http
{
    void brotli_encode(const std::string &data, std::string &out_data)
    {
        std::array<unsigned char, 2048> buffer;
        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        size_t  available_in = data.length(), available_out = buffer.size();
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer.data();

        do
        {
            BrotliEncoderCompressStream(
                instance, BROTLI_OPERATION_FINISH,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer.data()), buffer.size() - available_out);
            available_out = buffer.size();
            next_out = buffer.data();
        } while (!(available_in == 0 && BrotliEncoderIsFinished(instance)));

        BrotliEncoderDestroyInstance(instance);
    }

    void brotli_encode(const std::string &data, std::string &out_data, unsigned char *buffer, unsigned int buffer_size)
    {
        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);

        size_t available_in = data.length(), available_out = buffer_size;
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer;

        do
        {
            BrotliEncoderCompressStream(
                instance, BROTLI_OPERATION_FINISH,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer), buffer_size - available_out);
            available_out = buffer_size;
            next_out = buffer;
        } while (!(available_in == 0 && BrotliEncoderIsFinished(instance)));

        BrotliEncoderDestroyInstance(instance);
    }

    void brotli_encode_file(std::FILE *fp, long long file_size, std::string &out_data)
    {
        if (!fp || file_size <= 0)
            return;

        constexpr unsigned int CHUNK = 16384;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!instance)
            return;

        long long remaining = file_size;
        BrotliEncoderOperation op = BROTLI_OPERATION_PROCESS;

        do
        {
            size_t chunk = (remaining > (long long)CHUNK) ? CHUNK : (size_t)remaining;
            size_t nread = fread(in, 1, chunk, fp);
            if (nread == 0)
                break;
            remaining -= (long long)nread;

            size_t available_in  = nread;
            const unsigned char *next_in = in;
            if (remaining == 0)
                op = BROTLI_OPERATION_FINISH;

            do
            {
                size_t available_out = CHUNK;
                unsigned char *next_out = out;
                BrotliEncoderCompressStream(
                    instance, op,
                    &available_in, &next_in, &available_out, &next_out, nullptr);
                out_data.append(reinterpret_cast<const char *>(out), CHUNK - available_out);
            } while (available_in > 0 || (op == BROTLI_OPERATION_FINISH && !BrotliEncoderIsFinished(instance)));

        } while (op != BROTLI_OPERATION_FINISH || !BrotliEncoderIsFinished(instance));

        BrotliEncoderDestroyInstance(instance);
    }

    void brotli_decode(const std::string &data, std::string &out_data)
    {
        std::array<unsigned char, 2048> buffer;
        BrotliDecoderResult per_result;
        auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        size_t available_in = data.length(), available_out = buffer.size();
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer.data();

        do
        {
            per_result = BrotliDecoderDecompressStream(
                instance,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer.data()), buffer.size() - available_out);
            available_out = buffer.size();
            next_out = buffer.data();
        } while (!(available_in == 0 && per_result == BROTLI_DECODER_RESULT_SUCCESS));

        BrotliDecoderDestroyInstance(instance);
    }
    void brotli_decode(const std::string &data, std::string &out_data, unsigned char *buffer, unsigned int buffer_size)
    {
        BrotliDecoderResult per_result;
        auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        size_t available_in = data.length(), available_out = buffer_size;
        const unsigned char *next_in = reinterpret_cast<const unsigned char *>(data.c_str());
        unsigned char *next_out = buffer;
        do
        {
            per_result = BrotliDecoderDecompressStream(
                instance,
                &available_in, &next_in, &available_out, &next_out, nullptr);
            out_data.append(reinterpret_cast<const char *>(buffer), buffer_size - available_out);
            available_out = buffer_size;
            next_out = buffer;
        } while (!(available_in == 0 && per_result == BROTLI_DECODER_RESULT_SUCCESS));

        BrotliDecoderDestroyInstance(instance);
    }
}