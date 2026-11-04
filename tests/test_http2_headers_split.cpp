/*
 * test_http2_headers_split.cpp — 响应 HEADERS 的帧级切片契约
 * 契约来源：RFC 9113 §4.2（单帧长度不得越过对端 SETTINGS_MAX_FRAME_SIZE）、
 *          §6.2（END_STREAM/END_HEADERS 落在哪一枚帧上）、
 *          §6.10（CONTINUATION 的合法 flag 只有 END_HEADERS）、§6.5.2（该设置项的合法区间）。
 * 只调用 http2_frame.cpp 里的纯函数，不建 peer/session：
 * 断言的是「切出来的字节形状」，与 HPACK 正文内容无关，所以正文用可自检的填充字节。
 * 输出：每条 REQUIRE 计数，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 */
#include <cstdint>
#include <string>
#include <vector>
#include "http2_frame.h"
#include "test_framework.hpp"

namespace
{
constexpr unsigned char kHeaders = 0x01;
constexpr unsigned char kContinuation = 0x09;

struct frame_view
{
    unsigned int length = 0;
    unsigned char type = 0;
    unsigned char flags = 0;
    unsigned int stream_id = 0;
    std::string payload;
    bool complete = false;
};

// 从一串自成一体的帧里按 9 字节帧头逐枚解出；剩余字节不足一帧时停止
std::vector<frame_view> walk(const std::string &bytes)
{
    std::vector<frame_view> out;
    size_t off = 0;
    while (off + 9 <= bytes.size())
    {
        frame_view f;
        f.length = ((unsigned char)bytes[off] << 16) | ((unsigned char)bytes[off + 1] << 8) |
                   (unsigned char)bytes[off + 2];
        f.type = (unsigned char)bytes[off + 3];
        f.flags = (unsigned char)bytes[off + 4];
        f.stream_id = ((unsigned char)bytes[off + 5] << 24) | ((unsigned char)bytes[off + 6] << 16) |
                      ((unsigned char)bytes[off + 7] << 8) | (unsigned char)bytes[off + 8];
        if (off + 9 + f.length > bytes.size())
        {
            out.push_back(f);
            return out;
        }
        f.payload = bytes.substr(off + 9, f.length);
        f.complete = true;
        out.push_back(f);
        off += 9 + f.length;
    }
    return out;
}

// 造一枚单帧：9 字节帧头 + 恰好 block_size 字节的正文
std::string make_frame(unsigned char flags, unsigned int stream_id, size_t block_size)
{
    std::string out;
    out.resize(9);
    out[0] = (char)((block_size >> 16) & 0xFF);
    out[1] = (char)((block_size >> 8) & 0xFF);
    out[2] = (char)(block_size & 0xFF);
    out[3] = (char)kHeaders;
    out[4] = (char)flags;
    out[5] = (char)((stream_id >> 24) & 0xFF);
    out[6] = (char)((stream_id >> 16) & 0xFF);
    out[7] = (char)((stream_id >> 8) & 0xFF);
    out[8] = (char)(stream_id & 0xFF);
    for (size_t i = 0; i < block_size; i++)
    {
        out.push_back((char)('a' + (i % 26)));
    }
    return out;
}

std::string join_payloads(const std::vector<frame_view> &frames)
{
    std::string all;
    for (const auto &f : frames)
    {
        all.append(f.payload);
    }
    return all;
}

std::string flags_text(const std::vector<frame_view> &frames)
{
    std::string text;
    for (const auto &f : frames)
    {
        if (!text.empty())
        {
            text.append(",");
        }
        text.append(std::to_string((unsigned int)f.flags));
    }
    return text;
}

std::string lens_text(const std::vector<frame_view> &frames)
{
    std::string text;
    for (const auto &f : frames)
    {
        if (!text.empty())
        {
            text.append(",");
        }
        text.append(std::to_string(f.length));
    }
    return text;
}

std::string types_text(const std::vector<frame_view> &frames)
{
    std::string text;
    for (const auto &f : frames)
    {
        if (!text.empty())
        {
            text.append(",");
        }
        text.append(std::to_string((unsigned int)f.type));
    }
    return text;
}
}// namespace

// 头块正好等于上限：仍走单帧，一个字节都不动
TEST_CASE(split_exact_cap_stays_single_frame)
{
    const unsigned int cap = 16384;
    std::string one = make_frame(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM, 7, cap);
    std::string before = one;
    REQUIRE(http::http2_split_headers(one, cap) == false);
    REQUIRE(one == before);
    auto frames = walk(one);
    REQUIRE_EQ(frames.size(), (size_t)1);
    REQUIRE_EQ(frames[0].length, cap);
    REQUIRE_EQ((unsigned int)frames[0].flags, (unsigned int)(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM));
}

// 上限 + 1：切成两帧，首帧装满，尾帧 1 字节
TEST_CASE(split_cap_plus_one_makes_two_frames)
{
    const unsigned int cap = 16384;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, 3, cap + 1);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE_EQ(frames.size(), (size_t)2);
    REQUIRE_EQ(lens_text(frames), std::string("16384,1"));
    REQUIRE_EQ(types_text(frames), std::string("1,9"));
    REQUIRE_EQ(flags_text(frames), std::string("0,4"));
    REQUIRE_EQ(join_payloads(frames).size(), (size_t)(cap + 1));
}

// 跨多帧：每片都写自己的长度，末片才带 END_HEADERS
TEST_CASE(split_four_frames_with_tail)
{
    const unsigned int cap = 16384;
    const size_t block = 50000;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM, 11, block);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE_EQ(frames.size(), (size_t)4);
    REQUIRE_EQ(lens_text(frames), std::string("16384,16384,16384,848"));
    REQUIRE_EQ(types_text(frames), std::string("1,9,9,9"));
    REQUIRE_EQ(flags_text(frames), std::string("1,0,0,4"));
    REQUIRE_EQ(frames[0].payload.size(), (size_t)cap);
    REQUIRE_EQ(frames[3].payload.size(), (size_t)848);
}

// 块长是上限的整数倍：不产生 0 长度续帧
TEST_CASE(split_multiple_of_cap_has_no_empty_frame)
{
    const unsigned int cap = 16384;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, 5, (size_t)cap * 3);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE_EQ(frames.size(), (size_t)3);
    REQUIRE_EQ(lens_text(frames), std::string("16384,16384,16384"));
    REQUIRE_EQ(flags_text(frames), std::string("0,0,4"));
    for (const auto &f : frames)
    {
        REQUIRE(f.length > 0);
        REQUIRE(f.complete);
    }
}

// END_STREAM 只落首帧：续帧的合法 flag 只有 END_HEADERS
TEST_CASE(end_stream_lands_only_on_first_frame)
{
    const unsigned int cap = 1024;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM, 9, 2500);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE_EQ(frames.size(), (size_t)3);
    REQUIRE_EQ(flags_text(frames), std::string("1,0,4"));
    REQUIRE((frames[0].flags & HTTP2_HEADER_END_HEADERS) == 0);
    REQUIRE((frames[1].flags & (HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM)) == 0);
    REQUIRE((frames[2].flags & HTTP2_HEADER_END_HEADERS) != 0);
}

// 每枚帧都带原 stream id，包括 ≥ 2^24 的合法 id
TEST_CASE(stream_id_kept_in_every_frame)
{
    const unsigned int cap = 1000;
    const unsigned int big_id = 0x01000001;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, big_id, 2400);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE(frames.size() >= 3);
    for (const auto &f : frames)
    {
        REQUIRE_EQ(f.stream_id, big_id);
    }
}

// 正文不被改写：各片拼回去与原块逐字节相同
TEST_CASE(header_block_bytes_are_not_touched)
{
    const unsigned int cap = 4096;
    const size_t block = 10000;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, 13, block);
    std::string original = chain.substr(9);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE_EQ(frames.size(), (size_t)3);
    REQUIRE(join_payloads(frames) == original);
}

// cap=0 与不足 9 字节的输入：原样返回 false，不动字节
TEST_CASE(zero_cap_and_short_input_leave_bytes_untouched)
{
    std::string one = make_frame(HTTP2_HEADER_END_HEADERS, 1, 100);
    std::string before = one;
    REQUIRE(http::http2_split_headers(one, 0) == false);
    REQUIRE(one == before);

    std::string tiny = "abc";
    std::string tiny_before = tiny;
    REQUIRE(http::http2_split_headers(tiny, 16) == false);
    REQUIRE(tiny == tiny_before);

    std::string nine(9, 'z');
    REQUIRE(http::http2_split_headers(nine, 1) == false);
    REQUIRE_EQ(nine.size(), (size_t)9);
}

// 广告值超过 24 位：块没越过夹到 0xFFFFFF 的上限，仍是单帧
TEST_CASE(cap_above_24bit_leaves_small_block_single)
{
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, 15, 4096);
    std::string before = chain;
    REQUIRE(http::http2_split_headers(chain, 0x7FFFFFF) == false);
    REQUIRE(chain == before);
}

// 收尾两条写法的实际字节差：赋值会把 END_HEADERS 加回首帧，OR 不会
TEST_CASE(end_stream_patch_or_semantics_keeps_block_open)
{
    const unsigned int cap = 1000;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS, 17, 2400);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    REQUIRE_EQ(flags_text(walk(chain)), std::string("0,0,4"));

    std::string assigned = chain;
    REQUIRE(http::set_http2_headers_flag(assigned, HTTP2_HEADER_END_STREAM | HTTP2_HEADER_END_HEADERS) == true);
    REQUIRE_EQ(flags_text(walk(assigned)), std::string("5,0,4"));

    std::string ored = chain;
    REQUIRE(http::or_http2_headers_flag(ored, HTTP2_HEADER_END_STREAM) == true);
    REQUIRE_EQ(flags_text(walk(ored)), std::string("1,0,4"));
    // 首帧之外的字节必须完全不动
    REQUIRE_EQ(ored.substr(9), chain.substr(9));
}

// OR 语义只并指定位，已有的位保持原样；单帧（未切）也照常工作
TEST_CASE(or_flag_only_sets_given_bits)
{
    std::string single = make_frame(HTTP2_HEADER_END_HEADERS, 19, 40);
    REQUIRE(http::http2_split_headers(single, 16384) == false);
    REQUIRE(http::or_http2_headers_flag(single, HTTP2_HEADER_END_STREAM) == true);
    REQUIRE_EQ((unsigned int)walk(single)[0].flags, (unsigned int)(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM));

    std::string short_input = "abc";
    REQUIRE(http::or_http2_headers_flag(short_input, HTTP2_HEADER_END_STREAM) == false);
}

// 中间片必须全 flag 为 0：任何非零 flag 都会让续帧非法
TEST_CASE(middle_frames_carry_no_flags)
{
    const unsigned int cap = 1024;
    std::string chain = make_frame(HTTP2_HEADER_END_HEADERS | HTTP2_HEADER_END_STREAM, 21, 5000);
    REQUIRE(http::http2_split_headers(chain, cap) == true);
    auto frames = walk(chain);
    REQUIRE(frames.size() >= 5);
    for (size_t i = 1; i + 1 < frames.size(); i++)
    {
        REQUIRE_EQ((unsigned int)frames[i].flags, 0u);
        REQUIRE_EQ((unsigned int)frames[i].type, (unsigned int)kContinuation);
    }
    REQUIRE_EQ((unsigned int)frames[0].type, (unsigned int)kHeaders);
    REQUIRE(frames.back().flags & HTTP2_HEADER_END_HEADERS);
}

TEST_MAIN()
