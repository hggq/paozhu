/**
 *  @copyright copyright 2022, huang ziquan  All rights reserved.
 *  @author huang ziquan
 *  @author 黄自权
 *  @file http2_frame.cpp
 *  @date 2022-10-15
 *
 *  http2 protocol make header file
 *
 *
 */
#include "http2_frame.h"
#include "http2_huffman.h"

namespace http
{
    std::map<std::string, unsigned char> http2_header_codes_table = {
        {"empty", 0},
        {"authority", 1},
        {"GET", 2},
        {"POST", 3},
        {"path", 4},
        {"index", 5},
        {"http", 6},
        {"https", 7},
        {"200", 8},
        {"204", 9},
        {"206", 10},
        {"304", 11},
        {"400", 12},
        {"404", 13},
        {"500", 14},
        {"accept-charset", 15},
        {"accept-encoding", 16},
        {"accept-language", 17},
        {"accept-ranges", 18},
        {"accept", 19},
        {"access-control-allow-origin", 20},
        {"age", 21},
        {"allow", 22},
        {"authorization", 23},
        {"cache-control", 24},
        {"content-disposition", 25},
        {"content-encoding", 26},
        {"content-language", 27},
        {"content-length", 28},
        {"content-location", 29},
        {"content-range", 30},
        {"content-type", 31},
        {"cookie", 32},
        {"date", 33},
        {"etag", 34},
        {"expect", 35},
        {"expires", 36},
        {"from", 37},
        {"host", 38},
        {"if-match", 39},
        {"if-modified-since", 40},
        {"if-none-match", 41},
        {"if-range", 42},
        {"if-unmodified-since", 43},
        {"last-modified", 44},
        {"link", 45},
        {"location", 46},
        {"max-forwards", 47},
        {"proxy-authenticate", 48},
        {"proxy-authorization", 49},
        {"range", 50},
        {"referer", 51},
        {"refresh", 52},
        {"retry-after", 53},
        {"server", 54},
        {"set-cookie", 55},
        {"strict-transport-security", 56},
        {"transfer-encoding", 57},
        {"user-agent", 58},
        {"vary", 59},
        {"via", 60},
        {"www-authenticate", 61},

    };

    bool make_http2_headers(std::string &hh_data, unsigned int streamid)
    {

        unsigned char frame_header_data[] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01};

        for (int i = 0; i < 9; i++)
        {
            hh_data.push_back(frame_header_data[i]);
        }
        frame_header_data[8] = streamid & 0xFF;
        streamid = streamid >> 8;
        frame_header_data[7] = streamid & 0xFF;
        streamid = streamid >> 8;
        frame_header_data[6] = streamid & 0xFF;
        streamid = streamid >> 8;
        frame_header_data[5] = streamid & 0xFF;
        return true;
    }
    bool set_http2_frame_streamid(std::string &hh_data, unsigned int streamid)
    {
        if (hh_data.size() < 9)
        {
            return false;
        }
        hh_data[8] = streamid & 0xFF;
        streamid = streamid >> 8;
        hh_data[7] = streamid & 0xFF;
        streamid = streamid >> 8;
        hh_data[6] = streamid & 0xFF;
        streamid = streamid >> 8;
        hh_data[5] = streamid & 0xFF;

        return true;
    }
    bool set_http2_headers_size(std::string &hh_data, unsigned int sizenum)
    {
        if (hh_data.size() < 3)
        {
            return false;
        }
        hh_data[2] = sizenum & 0xFF;
        sizenum = sizenum >> 8;
        hh_data[1] = sizenum & 0xFF;
        sizenum = sizenum >> 8;
        hh_data[0] = sizenum & 0xFF;

        return true;
    }

    bool set_http2_headers_flag(std::string &hh_data, unsigned char flag)
    {
        if (hh_data.size() < 9)
        {
            return false;
        }
        hh_data[4] = flag;
        return true;
    }
    bool set_http2_headers_static(unsigned char *hh_data, unsigned char hh_code)
    {
        *hh_data = hh_code | 0x80;
        return true;
    }
    bool make_http2_headers_static(std::string &hh_data, unsigned int hh_code)
    {   
    
        switch (hh_code)
        {
        case 200:
            hh_data.push_back(0x88);
            break;
        case 204:
            hh_data.push_back(0x89);
            break;
        case 206:
            hh_data.push_back(0x8A);
            break;
        case 304:
            hh_data.push_back(0x8B);
            break;  
        case 400:
            hh_data.push_back(0x8C);
            break;
        case 404:
            hh_data.push_back(0x8D);
            break;
        case 500:
            hh_data.push_back(0x8E);
            break;                       
        default:
            hh_data.push_back(0x88);
            return false;

        }
 
        return true;
    }
    bool make_http2_headers_item(std::string &hh_data, unsigned char hh_code)
    {
        hh_data.push_back(hh_code | 0x80);
        return true;
    }

    bool make_http2_headers_item(std::string &hh_data, unsigned char hh_code, const std::string &value)
    {
        std::string en_value;
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
        hh_data.push_back(hh_code | 0x40);
        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item2(std::string &hh_data, const std::string &key, const std::string &value)
    {

        hh_data.push_back(0x40);

        std::string en_value;
        http_huffman_encode((unsigned char *)&key[0], key.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }

        hh_data.append((char *)&en_value[0], en_value.size());

        en_value.clear();
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item2(std::string &hh_data, unsigned char hh_code, const std::string &value)
    {
        std::string en_value;
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (hh_code > 63)
        {
            hh_data.push_back(0x7F);
            hh_data.push_back(hh_code - 63);
        }
        else
        {
            hh_data.push_back(hh_code | 0x40);
        }

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }

        hh_data.push_back((unsigned char)en_value.size() | 0x80);
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item2(std::string &hh_data, unsigned char hh_code, unsigned long long num)
    {

        if (hh_code > 63)
        {
            hh_data.push_back(0x7F);
            hh_data.push_back(hh_code - 63);
        }
        else
        {
            hh_data.push_back(hh_code | 0x40);
        }

        std::string value, en_value;
        value = std::to_string(num);
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item3(std::string &hh_data, const std::string &key, const std::string &value)
    {

        hh_data.push_back(0x00);

        std::string en_value;
        http_huffman_encode((unsigned char *)&key[0], key.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.append((char *)&en_value[0], en_value.size());

        en_value.clear();
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item3(std::string &hh_data, unsigned char hh_code, const std::string &value)
    {
        std::string en_value;
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (hh_code > 15)
        {
            hh_data.push_back(0x0F);
            hh_data.push_back(hh_code - 15);
        }
        else
        {
            hh_data.push_back(hh_code | 0x00);
        }

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }

        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item3(std::string &hh_data, unsigned char hh_code, unsigned long long num)
    {

        if (hh_code > 15)
        {
            hh_data.push_back(0x0F);
            hh_data.push_back(hh_code - 15);
        }
        else
        {
            hh_data.push_back(hh_code | 0x00);
        }

        std::string value, en_value;
        value = std::to_string(num);
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item4(std::string &hh_data, unsigned char hh_code, const std::string &value)
    {
        std::string en_value;
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (hh_code > 15)
        {
            hh_data.push_back(0x1F);
            hh_data.push_back(hh_code - 15);
        }
        else
        {
            hh_data.push_back(hh_code | 0x10);
        }

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }

        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item4(std::string &hh_data, unsigned char hh_code, unsigned long long num)
    {

        if (hh_code > 15)
        {
            hh_data.push_back(0x1F);
            hh_data.push_back(hh_code - 15);
        }
        else
        {
            hh_data.push_back(hh_code | 0x10);
        }

        std::string value, en_value;
        value = std::to_string(num);
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);
        hh_data.push_back((unsigned char)en_value.size() | 0x80);
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }

    bool make_http2_headers_item4(std::string &hh_data, const std::string &key, const std::string &value)
    {

        hh_data.push_back(0x10);

        std::string en_value;
        http_huffman_encode((unsigned char *)&key[0], key.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.append((char *)&en_value[0], en_value.size());

        en_value.clear();
        http_huffman_encode((unsigned char *)&value[0], value.size(), en_value);

        if (en_value.size() > 0x7F)
        {
            unsigned int totalsize = en_value.size() - 0x7F;
            hh_data.push_back(0xFF);
            if (totalsize > 127)
            {
                unsigned char temp = totalsize / 128;
                unsigned char tempsub = totalsize % 128;
                hh_data.push_back((unsigned char)tempsub | 0x80);
                hh_data.push_back((unsigned char)temp);
            }
            else
            {
                hh_data.push_back((unsigned char)totalsize);
            }
        }
        else
        {
            hh_data.push_back((unsigned char)en_value.size() | 0x80);
        }
        hh_data.append((char *)&en_value[0], en_value.size());
        return true;
    }
}