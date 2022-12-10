
#include <string>
#include "http2_huffman.h"

/*
 * this table form nghttp2 and Nginx
 */
namespace http
{
    int http_huffman_decode(unsigned char *state, unsigned char *src, size_t len, std::string &outstr, unsigned int last)
    {
        unsigned char *end, ch, ending;

        ch = 0;
        ending = 1;

        end = src + len;

        while (src != end)
        {
            ch = *src++;

            if (http_huffman_decode_bits(state, &ending, ch >> 4, outstr) != HUFFMAN_OK)
            {
                return HUFFMAN_ERROR;
            }

            if (http_huffman_decode_bits(state, &ending, ch & 0xf, outstr) != HUFFMAN_OK)
            {
                return HUFFMAN_ERROR;
            }
        }

        if (last)
        {
            if (!ending)
            {
                return HUFFMAN_ERROR;
            }

            *state = 0;
        }

        return HUFFMAN_OK;
    }

    int http_huffman_decode_bits(unsigned char *state, unsigned char *ending, unsigned int bits, std::string &outstr)
    {
        http_huffman_decode_code_t code;

        code = http_huffman_decode_tables[*state][bits];

        if (code.next == *state)
        {
            return HUFFMAN_ERROR;
        }

        if (code.emit)
        {
            outstr.push_back(code.sym);
        }

        *ending = code.ending;
        *state = code.next;

        return HUFFMAN_OK;
    }

    int http_huffman_encode(unsigned char *src, unsigned int len, std::string &outstr)
    {

        huffman_static_encode_t temp;
        unsigned int subchar_int, new_char;
        unsigned char temp_char = 0x00, pre_char_length = 0, cur_char_length = 0;

        //上一个 预留多少位 每满8位 送一个字符到outstr
        for (int i = 0; i < len; i++)
        {

            temp = huffman_encode_table[src[i]];
            ;
            subchar_int = temp.code; //转为int
            cur_char_length = temp.nbits;
            if (pre_char_length > 0)
            {
                if ((pre_char_length + cur_char_length) < 8)
                {
                    new_char = subchar_int >> pre_char_length | new_char;
                    pre_char_length = pre_char_length + cur_char_length;
                    continue;
                }

                unsigned char subchar_length = 8 - pre_char_length;
                temp_char = (unsigned char)((new_char >> 24) & 0xFF);
                unsigned char a = 0x00;
                for (int j = 0; j < subchar_length; j++)
                {

                    if ((subchar_int & 0x80000000) == 0x80000000)
                    {
                        a = (a << 1) | 0x01;
                    }
                    else
                    {
                        a = a << 1;
                    }
                    subchar_int = subchar_int << 1;
                }
                temp_char = temp_char | a;
                outstr.push_back(temp_char);
                cur_char_length -= subchar_length;
            }

            if (cur_char_length >= 8)
            {
                for (; cur_char_length >= 8; cur_char_length -= 8)
                {
                    temp_char = (unsigned char)((subchar_int >> 24) & 0xFF);
                    outstr.push_back(temp_char);
                    subchar_int = subchar_int << 8;
                }
            }
            if (cur_char_length > 0)
            {
                new_char = subchar_int;
                pre_char_length = cur_char_length;
            }
            else
            {
                new_char = 0x00;
                pre_char_length = 0x00;
            }
        }

        if (pre_char_length > 0)
        {
            unsigned char codes = 0xFF;
            temp_char = (unsigned char)((new_char >> 24) & 0xFF);
            temp_char |= (unsigned char)(codes >> pre_char_length);
            outstr.push_back(temp_char);
        }
        return 0;
    }
}