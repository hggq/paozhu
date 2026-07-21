#include "pzjpg.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace image {

// ==================== 常量表 ====================

static const double PI = 3.14159265358979323846;

// Zig-Zag 顺序
static const int zigzag_order[64] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// 标准亮度量化表
static const uint8_t std_lum_quant[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68,109,103, 77,
    24, 35, 55, 64, 81,104,113, 92,
    49, 64, 78, 87,103,121,120,101,
    72, 92, 95, 98,112,100,103, 99
};

// 标准色度量化表
static const uint8_t std_chrom_quant[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

// 标准 DC 亮度 Huffman 表 (Annex K)
[[maybe_unused]] static const uint8_t dc_lum_bits[16] = {0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
[[maybe_unused]] static const uint8_t dc_lum_vals[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

// 标准 DC 色度 Huffman 表
[[maybe_unused]] static const uint8_t dc_chrom_bits[16] = {0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
[[maybe_unused]] static const uint8_t dc_chrom_vals[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

// 标准 AC 亮度 Huffman 表
[[maybe_unused]] static const uint8_t ac_lum_bits[16] = {0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d};
[[maybe_unused]] static const uint8_t ac_lum_vals[162] = {
    0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
    0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,
    0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
    0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
    0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
    0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
    0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,
    0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,
    0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,
    0xf9,0xfa
};

// 标准 AC 色度 Huffman 表
[[maybe_unused]] static const uint8_t ac_chrom_bits[16] = {0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
[[maybe_unused]] static const uint8_t ac_chrom_vals[162] = {
    0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
    0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,
    0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,
    0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,
    0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,
    0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,
    0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,
    0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
    0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,
    0xf9,0xfa
};

// ==================== Huffman 编解码结构 ====================

struct HuffTable {
    uint8_t bits[16];
    uint8_t vals[256];
    int num_vals;
    // 编码用: symbol -> (code, length)
    uint16_t enc_code[256];
    uint8_t enc_len[256];
    // 解码用
    uint16_t mincode[17], maxcode[17];
    int valptr[17];

    void build() {
        // 构建编码表 (JPEG Annex C 标准 canonical code 生成)
        memset(enc_code, 0, sizeof(enc_code));
        memset(enc_len, 0, sizeof(enc_len));
        int code = 0;
        int k = 0;
        for (int len = 1; len <= 16; len++) {
            code <<= 1; // 每个码长开始前 shift
            for (int i = 0; i < bits[len - 1]; i++) {
                enc_code[vals[k]] = (uint16_t)code;
                enc_len[vals[k]] = (uint8_t)len;
                code++;
                k++;
            }
        }
        num_vals = k;
        // 构建解码表
        memset(mincode, 0, sizeof(mincode));
        memset(maxcode, 0, sizeof(maxcode));
        memset(valptr, 0, sizeof(valptr));
        code = 0;
        k = 0;
        for (int len = 1; len <= 16; len++) {
            if (bits[len - 1] == 0) {
                mincode[len] = 0xFFFF;
                maxcode[len] = 0;
                valptr[len] = 0;
            } else {
                mincode[len] = code;
                valptr[len] = k;
                code += bits[len - 1];
                maxcode[len] = code - 1;
                k += bits[len - 1];
            }
            code <<= 1;
        }
    }

    int decode(uint16_t code_val, int len) const {
        if (len >= 1 && len <= 16 && code_val >= mincode[len] && code_val <= maxcode[len]) {
            return vals[valptr[len] + (code_val - mincode[len])];
        }
        return -1;
    }
};

// ==================== 位写入器 ====================

class BitWriter {
    std::vector<uint8_t>& out;
    uint32_t buf = 0;
    int nbits = 0;
public:
    BitWriter(std::vector<uint8_t>& o) : out(o) {}

    void writeBits(uint16_t val, int len) {
        buf = (buf << len) | (val & ((1 << len) - 1));
        nbits += len;
        while (nbits >= 8) {
            nbits -= 8;
            uint8_t byte = (uint8_t)(buf >> nbits);
            out.push_back(byte);
            if (byte == 0xFF) out.push_back(0x00); // 字节填充
        }
    }

    void flush() {
        if (nbits > 0) {
            uint8_t byte = (uint8_t)(buf << (8 - nbits));
            // 剩余位填1
            byte |= (1 << (8 - nbits)) - 1;
            out.push_back(byte);
            if (byte == 0xFF) out.push_back(0x00);
            nbits = 0;
            buf = 0;
        }
    }
};

// ==================== 位读取器 ====================

class BitReader {
    const uint8_t* data;
    size_t len;
    size_t pos = 0;
    uint32_t buf = 0;
    int nbits = 0;
public:
    BitReader(const uint8_t* d, size_t l) : data(d), len(l) {}

    int readBit() {
        if (nbits == 0) {
            if (pos >= len) return -1;
            buf = data[pos++];
            if (buf == 0xFF && pos < len && data[pos] == 0x00) {
                pos++;
            }
            nbits = 8;
        }
        nbits--;
        return (buf >> nbits) & 1;
    }

    uint16_t readBits(int n) {
        uint16_t val = 0;
        for (int i = 0; i < n; i++) {
            int bit = readBit();
            if (bit < 0) return 0;
            val = (val << 1) | bit;
        }
        return val;
    }

    bool isEnd() const {
        return pos >= len && nbits == 0;
    }
};

// ==================== 辅助函数 ====================

static void rgbToYCbCr(uint8_t r, uint8_t g, uint8_t b, float& y, float& cb, float& cr) {
    y  =  0.299f * r + 0.587f * g + 0.114f * b;
    cb = -0.1687f * r - 0.3313f * g + 0.5f * b + 128.0f;
    cr =  0.5f * r - 0.4187f * g - 0.0813f * b + 128.0f;
}

static void yCbCrToRgb(float y, float cb, float cr, uint8_t& r, uint8_t& g, uint8_t& b) {
    float rf = y + 1.402f * (cr - 128.0f);
    float gf = y - 0.34414f * (cb - 128.0f) - 0.71414f * (cr - 128.0f);
    float bf = y + 1.772f * (cb - 128.0f);
    r = (uint8_t)std::max(0, std::min(255, (int)(rf + 0.5f)));
    g = (uint8_t)std::max(0, std::min(255, (int)(gf + 0.5f)));
    b = (uint8_t)std::max(0, std::min(255, (int)(bf + 0.5f)));
}

// 简易 DCT (直接计算，非快速算法，但正确)
static void fdct(float block[64]) {
    float tmp[64];
    // 行变换
    for (int i = 0; i < 8; i++) {
        for (int k = 0; k < 8; k++) {
            double sum = 0;
            for (int x = 0; x < 8; x++) {
                sum += block[i * 8 + x] * cos((2.0 * x + 1.0) * k * PI / 16.0);
            }
            double alpha = (k == 0) ? 1.0 / sqrt(2.0) : 1.0;
            tmp[i * 8 + k] = (float)(0.5 * alpha * sum);
        }
    }
    // 列变换
    for (int k = 0; k < 8; k++) {
        for (int j = 0; j < 8; j++) {
            double sum = 0;
            for (int y = 0; y < 8; y++) {
                sum += tmp[y * 8 + k] * cos((2.0 * y + 1.0) * j * PI / 16.0);
            }
            double alpha = (j == 0) ? 1.0 / sqrt(2.0) : 1.0;
            block[j * 8 + k] = (float)(0.5 * alpha * sum);
        }
    }
}

static void idct(float block[64]) {
    float tmp[64];
    // 列逆变换
    for (int k = 0; k < 8; k++) {
        for (int y = 0; y < 8; y++) {
            double sum = 0;
            for (int j = 0; j < 8; j++) {
                double alpha = (j == 0) ? 1.0 / sqrt(2.0) : 1.0;
                sum += alpha * block[j * 8 + k] * cos((2.0 * y + 1.0) * j * PI / 16.0);
            }
            tmp[y * 8 + k] = (float)(0.5 * sum);
        }
    }
    // 行逆变换
    for (int i = 0; i < 8; i++) {
        for (int x = 0; x < 8; x++) {
            double sum = 0;
            for (int k = 0; k < 8; k++) {
                double alpha = (k == 0) ? 1.0 / sqrt(2.0) : 1.0;
                sum += alpha * tmp[i * 8 + k] * cos((2.0 * x + 1.0) * k * PI / 16.0);
            }
            block[i * 8 + x] = (float)(0.5 * sum);
        }
    }
}

// 计算 value 需要的位数 (category)
static int bitCategory(int val) {
    if (val < 0) val = -val;
    int cat = 0;
    while (val > 0) { cat++; val >>= 1; }
    return cat;
}

// 编码值的附加位
static uint16_t encodeValue(int val, int cat) {
    if (val < 0) val = val + (1 << cat) - 1;
    return (uint16_t)(val & ((1 << cat) - 1));
}

// 从附加位解码值
static int decodeValue(uint16_t bits, int cat) {
    if (cat == 0) return 0;
    int val = bits;
    if (val < (1 << (cat - 1))) {
        val = val - (1 << cat) + 1;
    }
    return val;
}

// 根据质量生成量化表
static void makeQuantTable(const uint8_t base[64], uint8_t out[64], int quality) {
    quality = std::max(10, std::min(100, quality));
    int scale = (quality < 50) ? (5000 / quality) : (200 - quality * 2);
    for (int i = 0; i < 64; i++) {
        int val = ((int)base[i] * scale + 50) / 100;
        out[i] = (uint8_t)std::max(1, std::min(255, val));
    }
}

// ==================== 自适应 Huffman 表构建 ====================

struct HuffNode {
    int freq;
    int symbol; // -1 = 内部节点
    int left, right;
};

// 从频率统计构建 JPEG 格式的 Huffman 表 (bits[16] + vals[])
// 返回符号数量
// 注意: JPEG 要求任何码字不能是全1 (libjpeg 会拒绝)。
// 解决方法 (同 libjpeg jchuff.c): 加入伪符号256(频率1)参与建树，
// 它会占据全1码字，生成表时排除它，保证真实符号不会拿到全1码。
static int buildOptimalHuffTable(const int freq[256], uint8_t out_bits[16], uint8_t out_vals[256]) {
    // 收集有频率的符号
    struct SymFreq { int sym; int freq; };
    std::vector<SymFreq> syms;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) syms.push_back({i, freq[i]});
    }
    if (syms.empty()) {
        memset(out_bits, 0, 16);
        return 0;
    }

    // 加入伪符号 256 (频率1)，防止真实符号获得全1码字
    syms.push_back({256, 1});

    int n = (int)syms.size();

    // 构建 Huffman 树 (用数组模拟)
    std::vector<int> node_freq(2 * n);
    std::vector<int> node_left(2 * n, -1), node_right(2 * n, -1);
    std::vector<int> node_sym(2 * n, -1);

    for (int i = 0; i < n; i++) {
        node_freq[i] = syms[i].freq;
        node_sym[i] = syms[i].sym;
    }

    // 优先队列 (min-heap by freq)
    auto cmp = [&](int a, int b) { return node_freq[a] > node_freq[b]; };
    std::vector<int> heap;
    for (int i = 0; i < n; i++) heap.push_back(i);
    std::make_heap(heap.begin(), heap.end(), cmp);

    int next_node = n;
    while (heap.size() > 1) {
        std::pop_heap(heap.begin(), heap.end(), cmp);
        int a = heap.back(); heap.pop_back();
        std::pop_heap(heap.begin(), heap.end(), cmp);
        int b = heap.back(); heap.pop_back();
        int nn = next_node++;
        node_freq[nn] = node_freq[a] + node_freq[b];
        node_left[nn] = a;
        node_right[nn] = b;
        heap.push_back(nn);
        std::push_heap(heap.begin(), heap.end(), cmp);
    }
    int root = heap[0];

    // 计算码长 (sym 范围 0..256, 256是伪符号)
    int code_len[257];
    memset(code_len, 0, sizeof(code_len));
    struct SI { int node; int depth; };
    std::vector<SI> stk;
    stk.push_back({root, 0});
    while (!stk.empty()) {
        auto [nd, dep] = stk.back(); stk.pop_back();
        if (node_sym[nd] >= 0) {
            code_len[node_sym[nd]] = dep;
        } else {
            stk.push_back({node_left[nd], dep + 1});
            stk.push_back({node_right[nd], dep + 1});
        }
    }

    // 统计每个码长的符号数 (含伪符号256)
    int max_len = 0;
    for (int i = 0; i <= 256; i++)
        if (code_len[i] > max_len) max_len = code_len[i];
    std::vector<int> bits((size_t)std::max(max_len, 16) + 1, 0);
    for (int i = 0; i <= 256; i++)
        if (code_len[i] > 0) bits[code_len[i]]++;

    // JPEG K.2 码长限制: 将所有 >16 的码长调整到 <=16
    // (同 libjpeg jchuff.c 的标准算法, 保证生成合法表)
    for (int i = max_len; i > 16; i--) {
        while (bits[i] > 0) {
            int j = i - 2;              // 找可用作前缀的较短码长
            while (j > 0 && bits[j] == 0) j--;
            if (j < 1) break;           // 安全保护 (正常不会发生)
            bits[i] -= 2;               // 移除两个长码符号
            bits[i - 1] += 1;           // 一个进入 i-1 长度
            bits[j + 1] += 2;           // 两个新符号进入 j+1 长度
            bits[j] -= 1;               // j 长度的一个符号变为前缀
        }
    }
    // 移除伪符号256: 从最大码长中减去一个 (它占据全1码字)
    {
        int i = std::min(16, max_len);
        while (i > 0 && bits[i] == 0) i--;
        if (i > 0) bits[i]--;
    }

    // 将真实符号分配到各码长: 高频符号获得短码 (最优)
    std::vector<std::pair<int, int>> sym_freq; // (freq, sym)
    for (int i = 0; i < 256; i++)
        if (freq[i] > 0) sym_freq.push_back({freq[i], i});
    std::sort(sym_freq.begin(), sym_freq.end(), [](auto& a, auto& b) {
        if (a.first != b.first) return a.first > b.first; // 频率降序
        return a.second < b.second;                        // 同频按符号升序
    });
    memset(code_len, 0, sizeof(code_len));
    {
        int idx = 0;
        for (int len = 1; len <= 16 && idx < (int)sym_freq.size(); len++) {
            for (int j = 0; j < bits[len] && idx < (int)sym_freq.size(); j++) {
                code_len[sym_freq[idx].second] = len;
                idx++;
            }
        }
    }

    // 统计每个长度的符号数 (仅真实符号 0-255)
    memset(out_bits, 0, 16);
    for (int i = 0; i < 256; i++) {
        if (code_len[i] > 0 && code_len[i] <= 16)
            out_bits[code_len[i] - 1]++;
    }

    // 按 JPEG 规范排序: 先按码长，同长度按符号值排序
    struct SymLen { int sym; int len; };
    std::vector<SymLen> sorted;
    for (int i = 0; i < 256; i++)
        if (code_len[i] > 0) sorted.push_back({i, code_len[i]});
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
        if (a.len != b.len) return a.len < b.len;
        return a.sym < b.sym;
    });
    int nvals = (int)sorted.size();
    for (int i = 0; i < nvals; i++)
        out_vals[i] = (uint8_t)sorted[i].sym;

    return nvals;
}

// 统计 DC 符号频率
[[maybe_unused]] static void countDCFreq(const std::vector<int>& coeff, int total_blocks, int freq[256]) {
    int prev_dc = 0;
    for (int blk = 0; blk < total_blocks; blk++) {
        int dc = coeff[blk * 64];
        int diff = dc - prev_dc;
        prev_dc = dc;
        int cat = bitCategory(diff);
        freq[cat]++;
    }
}

// 统计 AC 符号频率
static void countACFreq(const std::vector<int>& coeff, int total_blocks, int freq[256]) {
    for (int blk = 0; blk < total_blocks; blk++) {
        int zero_run = 0;
        for (int k = 1; k < 64; k++) {
            int v = coeff[blk * 64 + k];
            if (v == 0) { zero_run++; continue; }
            while (zero_run > 15) { freq[0xF0]++; zero_run -= 16; }
            int cat = bitCategory(v);
            freq[(zero_run << 4) | cat]++;
            zero_run = 0;
        }
        if (zero_run > 0) freq[0x00]++; // EOB
    }
}

// ==================== 编码 ====================

static void writeByte(std::vector<uint8_t>& buf, uint8_t b) { buf.push_back(b); }
static void writeWord(std::vector<uint8_t>& buf, uint16_t w) {
    buf.push_back((uint8_t)(w >> 8));
    buf.push_back((uint8_t)(w & 0xFF));
}

static void writeDHT(std::vector<uint8_t>& buf, uint8_t id, const uint8_t bits[16],
                     const uint8_t* vals, int nvals) {
    writeWord(buf, 0xFFC4);
    writeWord(buf, (uint16_t)(2 + 1 + 16 + nvals));
    writeByte(buf, id);
    for (int i = 0; i < 16; i++) writeByte(buf, bits[i]);
    for (int i = 0; i < nvals; i++) writeByte(buf, vals[i]);
}

static void writeDQT(std::vector<uint8_t>& buf, uint8_t id, const uint8_t table[64], int bits_per_sample = 8) {
    writeWord(buf, 0xFFDB);
    if (bits_per_sample == 12) {
        writeWord(buf, 2 + 1 + 64 * 2);
        writeByte(buf, 0x10 | id); // 精度1(12bit) | 表ID
        for (int i = 0; i < 64; i++) {
            uint16_t val = static_cast<uint16_t>(table[i]) << 4;
            writeWord(buf, val);
        }
    } else {
        writeWord(buf, 2 + 1 + 64);
        writeByte(buf, id); // 精度0(8bit) | 表ID
        for (int i = 0; i < 64; i++) writeByte(buf, table[i]);
    }
}

// 编码一个 8x8 块
[[maybe_unused]] static void encodeBlock(BitWriter& bw, const float block[64],
                        const uint8_t qtable[64], int& prev_dc,
                        const HuffTable& dc_ht, const HuffTable& ac_ht) {
    // 量化 + zigzag
    int quant[64];
    for (int i = 0; i < 64; i++) {
        int zi = zigzag_order[i];
        quant[i] = (int)roundf(block[zi] / (float)qtable[i]);
    }

    // DC 差分
    int dc_diff = quant[0] - prev_dc;
    prev_dc = quant[0];
    int cat = bitCategory(dc_diff);
    bw.writeBits(dc_ht.enc_code[cat], dc_ht.enc_len[cat]);
    if (cat > 0) {
        bw.writeBits(encodeValue(dc_diff, cat), cat);
    }

    // AC RLE + Huffman
    int zero_run = 0;
    for (int i = 1; i < 64; i++) {
        if (quant[i] == 0) {
            zero_run++;
        } else {
            while (zero_run > 15) {
                bw.writeBits(ac_ht.enc_code[0xF0], ac_ht.enc_len[0xF0]); // ZRL
                zero_run -= 16;
            }
            int acat = bitCategory(quant[i]);
            uint8_t symbol = (uint8_t)((zero_run << 4) | acat);
            bw.writeBits(ac_ht.enc_code[symbol], ac_ht.enc_len[symbol]);
            bw.writeBits(encodeValue(quant[i], acat), acat);
            zero_run = 0;
        }
    }
    if (zero_run > 0) {
        bw.writeBits(ac_ht.enc_code[0x00], ac_ht.enc_len[0x00]); // EOB
    }
}

// ==================== 解码 (baseline) ====================

static bool decodeBlock(BitReader& br, float block[64],
                        const uint8_t qtable[64], int& prev_dc,
                        const HuffTable& dc_ht, const HuffTable& ac_ht) {
    memset(block, 0, sizeof(float) * 64);

    // 解码 DC
    uint16_t code = 0;
    int cat = -1;
    for (int len = 1; len <= 16; len++) {
        code = (code << 1) | br.readBit();
        int sym = dc_ht.decode(code, len);
        if (sym >= 0) { cat = sym; break; }
    }
    if (cat < 0) return false;
    int dc_val = 0;
    if (cat > 0) {
        dc_val = decodeValue(br.readBits(cat), cat);
    }
    prev_dc += dc_val;

    // 解码 AC
    int idx = 1;
    while (idx < 64) {
        code = 0;
        int symbol = -1;
        for (int len = 1; len <= 16; len++) {
            code = (code << 1) | br.readBit();
            int s = ac_ht.decode(code, len);
            if (s >= 0) { symbol = s; break; }
        }
        if (symbol < 0) return false;
        if (symbol == 0x00) break; // EOB
        if (symbol == 0xF0) { idx += 16; continue; } // ZRL
        int run = (symbol >> 4) & 0x0F;
        int acat = symbol & 0x0F;
        idx += run;
        if (idx >= 64) break;
        int ac_val = decodeValue(br.readBits(acat), acat);
        int zi = zigzag_order[idx];
        if (zi >= 0 && zi < 64)
            block[zi] = (float)(ac_val * qtable[idx]);
        idx++;
    }
    block[zigzag_order[0]] = (float)(prev_dc * qtable[0]);
    return true;
}

// ==================== Progressive 解码辅助 ====================

struct ScanInfo {
    const uint8_t* data;
    size_t len;
    int ns;              // 本次扫描分量数
    uint8_t comp_sel[4]; // 分量ID
    uint8_t td_ta[4];    // DC/AC 表选择
    int Ss, Se, Ah, Al;
    HuffTable tables[8]; // 本扫描时刻的Huffman表快照 (progressive可能在扫描间重定义表)
};

static int huffDecodeSymbol(BitReader& br, const HuffTable& ht) {
    uint16_t code = 0;
    for (int len = 1; len <= 16; len++) {
        code = (code << 1) | br.readBit();
        int s = ht.decode(code, len);
        if (s >= 0) return s;
    }
    return -1;
}

static void receiveExtend(BitReader& br, int cat, int& val) {
    if (cat == 0) { val = 0; return; }
    int bits = br.readBits(cat);
    val = decodeValue((uint16_t)bits, cat);
}

// ==================== jpg 实现 ====================

void jpg::create(int w, int h) {
    width = w;
    height = h;
    pixels.resize((size_t)w * h);
}

// 将二维码模块数据(qrcode库输出, 非零=暗模块)缩放绘制到本图
// q_width/q_height: 模块数; scale: 每模块放大为 scale×scale 像素; pos_x/pos_y: 左上角起点
bool jpg::qrdata(std::vector<uint8_t>& qr_data, int q_width, int q_height,
                 unsigned char scale, int pos_x, int pos_y) {
    if (q_width <= 0 || q_height <= 0) return false;
    if (pixels.empty() || width <= 0 || height <= 0) return false;
    if (scale == 0) scale = 1;
    if ((size_t)q_width * q_height > qr_data.size()) return false;

    for (int y = 0; y < q_height; ++y) {
        for (int x = 0; x < q_width; ++x) {
            if (!qr_data[(size_t)y * q_width + x]) continue; // 只画暗模块(非零=黑)
            int bx = pos_x + x * scale;
            int by = pos_y + y * scale;
            for (int dy = 0; dy < scale; ++dy) {
                int py = by + dy;
                if (py < 0 || py >= height) continue;
                for (int dx = 0; dx < scale; ++dx) {
                    int px = bx + dx;
                    if (px < 0 || px >= width) continue;
                    pixels[(size_t)py * width + px] = ColorRGB{0, 0, 0}; // 暗模块=黑
                }
            }
        }
    }
    return true;
}

void jpg::set(int x, int y, const ColorRGB& c) {
    if (x < 0 || x >= width || y < 0 || y >= height)
        throw std::out_of_range("坐标越界");
    pixels[y * width + x] = c;
}

// ==================== Bicubic 双三次插值缩放 ====================

// Catmull-Rom 三次插值核 (a = -0.5)
static double bicubicWeight(double x) {
    const double a = -0.5;
    double ax = std::fabs(x);
    if (ax <= 1.0) {
        return (a + 2.0) * ax * ax * ax - (a + 3.0) * ax * ax + 1.0;
    } else if (ax < 2.0) {
        return a * ax * ax * ax - 5.0 * a * ax * ax + 8.0 * a * ax - 4.0 * a;
    }
    return 0.0;
}

static uint8_t clampByte(double v) {
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return (uint8_t)(v + 0.5);
}

bool jpg::resize(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return false;
    if (width <= 0 || height <= 0 || pixels.empty()) return false;
    if (newWidth == width && newHeight == height) return true;

    std::vector<ColorRGB> dst((size_t)newWidth * newHeight);
    double xRatio = (double)width / newWidth;
    double yRatio = (double)height / newHeight;

    for (int oy = 0; oy < newHeight; oy++) {
        // 目标像素中心映射回源坐标
        double sy = (oy + 0.5) * yRatio - 0.5;
        int y0 = (int)std::floor(sy) - 1;
        for (int ox = 0; ox < newWidth; ox++) {
            double sx = (ox + 0.5) * xRatio - 0.5;
            int x0 = (int)std::floor(sx) - 1;

            double sumR = 0, sumG = 0, sumB = 0, sumW = 0;
            for (int j = 0; j < 4; j++) {
                int yy = y0 + j;
                if (yy < 0) yy = 0;
                else if (yy >= height) yy = height - 1;
                double wy = bicubicWeight(sy - (y0 + j));
                const ColorRGB* row = &pixels[(size_t)yy * width];
                for (int i = 0; i < 4; i++) {
                    int xx = x0 + i;
                    if (xx < 0) xx = 0;
                    else if (xx >= width) xx = width - 1;
                    double w = wy * bicubicWeight(sx - (x0 + i));
                    const ColorRGB& c = row[xx];
                    sumR += c.r * w;
                    sumG += c.g * w;
                    sumB += c.b * w;
                    sumW += w;
                }
            }
            ColorRGB& out = dst[(size_t)oy * newWidth + ox];
            out.r = clampByte(sumR / sumW);
            out.g = clampByte(sumG / sumW);
            out.b = clampByte(sumB / sumW);
        }
    }

    pixels = std::move(dst);
    width = newWidth;
    height = newHeight;
    return true;
}

bool jpg::resizeScale(double scale) {
    if (scale <= 0.0) return false;
    int nw = (int)std::lround(width * scale);
    int nh = (int)std::lround(height * scale);
    if (nw < 1) nw = 1;
    if (nh < 1) nh = 1;
    return resize(nw, nh);
}

// ==================== 像素访问 / 绘图 (与 PNG 保持一致) ====================

void jpg::fillColor(const ColorRGB &c) {
    std::fill(pixels.begin(), pixels.end(), c);
}

void jpg::setPixel(uint32_t x, uint32_t y, const ColorRGB &c) {
    if ((int)x < width && (int)y < height)
        at(x, y) = c;
}

ColorRGB jpg::getPixel(uint32_t x, uint32_t y) const {
    if ((int)x < width && (int)y < height)
        return at(x, y);
    return ColorRGB{0, 0, 0};
}

void jpg::setRect(int x1, int y1, int x2, int y2, const ColorRGB &c) {
    if (pixels.empty() || width == 0 || height == 0) return;
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= width) x2 = width - 1;
    if (y2 >= height) y2 = height - 1;
    if (x1 > x2 || y1 > y2) return;
    for (int y = y1; y <= y2; ++y) {
        ColorRGB *r = rowPtr((uint32_t)y);
        std::fill(r + x1, r + x2 + 1, c);
    }
}

void jpg::drawSinglePixelLine(int x1, int y1, int x2, int y2, const ColorRGB &c) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
            setPixel((uint32_t)x1, (uint32_t)y1, c);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void jpg::drawLine(int x1, int y1, int x2, int y2, const ColorRGB &c, int thickness) {
    if (pixels.empty() || width == 0 || height == 0) return;
    if (thickness <= 0) thickness = 1;
    if (y1 == y2) {
        int half = thickness / 2;
        for (int offY = -half; offY <= thickness - half - 1; ++offY)
            drawSinglePixelLine(x1, y1 + offY, x2, y1 + offY, c);
        return;
    }
    if (x1 == x2) {
        int half = thickness / 2;
        for (int offX = -half; offX <= thickness - half - 1; ++offX)
            drawSinglePixelLine(x1 + offX, y1, x1 + offX, y2, c);
        return;
    }
    if (thickness == 1) { drawSinglePixelLine(x1, y1, x2, y2, c); return; }
    double dx = (double)(x2 - x1), dy = (double)(y2 - y1);
    double len = std::sqrt(dx * dx + dy * dy);
    if (len == 0) { setPixel(x1, y1, c); return; }
    double nx = -dy / len, ny = dx / len;
    int half = thickness / 2;
    for (int offset = -half; offset <= thickness - half - 1; ++offset) {
        int ox = (int)(nx * offset + 0.5);
        int oy = (int)(ny * offset + 0.5);
        drawSinglePixelLine(x1 + ox, y1 + oy, x2 + ox, y2 + oy, c);
    }
}

void jpg::drawLine(int x, int y, int length, const ColorRGB &c) {
    if (length <= 0) return;
    drawLine(x, y, x + length - 1, y, c, 1);
}

// ---------- Xiaolin Wu 抗锯齿画线 ----------
void jpg::plotPixel(int w, int h, int x, int y, float brightness, const ColorRGB &fg) {
    if (x < 0 || x >= w || y < 0 || y >= h) return;
    float alpha = std::max(0.0f, std::min(1.0f, brightness));
    if (alpha <= 0.0f) return;
    float inv = 1.0f - alpha;
    ColorRGB &bg = pixels[(size_t)y * w + x];
    bg.r = (uint8_t)(fg.r * alpha + bg.r * inv + 0.5f);
    bg.g = (uint8_t)(fg.g * alpha + bg.g * inv + 0.5f);
    bg.b = (uint8_t)(fg.b * alpha + bg.b * inv + 0.5f);
}

void jpg::drawSoftLine(int w, int h, float x0, float y0, float x1, float y1, const ColorRGB &c) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
    float dx = x1 - x0, dy = y1 - y0;
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;
    int xpxl1 = (int)x0, ypxl1 = (int)y0;
    plotPixel(w, h, steep ? ypxl1 : xpxl1, steep ? xpxl1 : ypxl1, rfpart(y0), c);
    plotPixel(w, h, steep ? ypxl1 : xpxl1, steep ? xpxl1 : (ypxl1 + 1), fpart(y0), c);
    for (int x = xpxl1 + 1; x < (int)x1; x++) {
        float y = y0 + gradient * (x - x0);
        int ypxl = (int)y;
        plotPixel(w, h, steep ? ypxl : x, steep ? x : ypxl, rfpart(y), c);
        plotPixel(w, h, steep ? (ypxl + 1) : x, steep ? x : (ypxl + 1), fpart(y), c);
    }
}

void jpg::drawThickLine(int w, int h, float x0, float y0, float x1, float y1,
                        const ColorRGB &color, int thickness) {
    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    float nx = -dy / len, ny = dx / len;
    int half = thickness / 2;
    for (int i = -half; i <= half; ++i)
        drawSoftLine(w, h, x0 + nx * i, y0 + ny * i, x1 + nx * i, y1 + ny * i, color);
}

// ---------- 圆 / 扇形 / 圆弧 / 圆角矩形 ----------
void jpg::drawCircle(int w, int h, int cx, int cy, int radius, const ColorRGB &color) {
    int x = radius, y = 0, err = 0;
    while (x >= y) {
        plotPixel(w, h, cx + x, cy + y, 1.0f, color);
        plotPixel(w, h, cx + y, cy + x, 1.0f, color);
        plotPixel(w, h, cx - y, cy + x, 1.0f, color);
        plotPixel(w, h, cx - x, cy + y, 1.0f, color);
        plotPixel(w, h, cx - x, cy - y, 1.0f, color);
        plotPixel(w, h, cx - y, cy - x, 1.0f, color);
        plotPixel(w, h, cx + y, cy - x, 1.0f, color);
        plotPixel(w, h, cx + x, cy - y, 1.0f, color);
        if (err <= 0) { y += 1; err += 2 * y + 1; }
        if (err > 0) { x -= 1; err -= 2 * x + 1; }
    }
}

void jpg::fillCircle(int w, int h, int cx, int cy, int radius, const ColorRGB &color) {
    for (int y = -radius; y <= radius; ++y) {
        int half_width = (int)std::sqrt(radius * radius - y * y);
        for (int x = cx - half_width; x <= cx + half_width; ++x)
            plotPixel(w, h, x, cy + y, 1.0f, color);
    }
}

void jpg::fillCircleHard(int w, int h, int cx, int cy, int radius, const ColorRGB &c) {
    for (int y = -radius; y <= radius; ++y) {
        int py = cy + y;
        if (py < 0 || py >= h) continue;
        int half_width = (int)std::sqrt(radius * radius - y * y);
        int x1 = std::max(cx - half_width, 0);
        int x2 = std::min(cx + half_width, w - 1);
        for (int x = x1; x <= x2; ++x)
            pixels[(size_t)py * w + x] = c; // 直接写成员像素缓冲
    }
}

void jpg::fillSector(int w, int h, int cx, int cy, int radius,
                     float start_angle, float end_angle, const ColorRGB &color) {
    while (start_angle < 0) start_angle += 2 * (float)M_PI;
    while (end_angle < 0) end_angle += 2 * (float)M_PI;
    if (end_angle < start_angle) end_angle += 2 * (float)M_PI;
    float range = end_angle - start_angle;
    for (int y = -radius; y <= radius; ++y) {
        int half_width = (int)std::sqrt(radius * radius - y * y);
        for (int x = -half_width; x <= half_width; ++x) {
            float angle = std::atan2((float)y, (float)x);
            if (angle < 0) angle += 2 * (float)M_PI;
            bool inside = (range < 2 * (float)M_PI) ? (angle >= start_angle && angle <= end_angle) : true;
            if (inside) plotPixel(w, h, cx + x, cy + y, 1.0f, color);
        }
    }
}

void jpg::drawFilledCircleWithBorder(int w, int h, int cx, int cy, int radius,
                                     const ColorRGB &fill_color, const ColorRGB &border_color) {
    fillCircle(w, h, cx, cy, radius, fill_color);
    drawCircle(w, h, cx, cy, radius, border_color);
}

void jpg::drawArc(int w, int h, int cx, int cy, int radius,
                  float start_angle, float end_angle, const ColorRGB &color, float step) {
    if (radius <= 0) return;
    if (end_angle < start_angle) end_angle += 2.0f * (float)M_PI;
    float range = end_angle - start_angle;
    if (range <= 0) return;
    int num_steps = std::max(2, (int)(range / step));
    for (int i = 0; i <= num_steps; ++i) {
        float t = start_angle + range * (i / (float)num_steps);
        int px = cx + (int)(radius * std::cos(t) + 0.5f);
        int py = cy + (int)(radius * std::sin(t) + 0.5f);
        plotPixel(w, h, px, py, 1.0f, color);
    }
}

void jpg::drawRoundedRectOutline(int x, int y, int w, int h, int r, const ColorRGB &color) {
    int W = width, H = height; // 图像尺寸 (区别于矩形 w,h)
    if (w <= 0 || h <= 0 || r <= 0) {
        drawSoftLine(W, H, x, y, x + w, y, color);
        drawSoftLine(W, H, x + w, y, x + w, y + h, color);
        drawSoftLine(W, H, x + w, y + h, x, y + h, color);
        drawSoftLine(W, H, x, y + h, x, y, color);
        return;
    }
    r = std::min(r, std::min(w / 2, h / 2));
    drawSoftLine(W, H, x + r, y, x + w - r, y, color);
    drawSoftLine(W, H, x + w, y + r, x + w, y + h - r, color);
    drawSoftLine(W, H, x + r, y + h, x + w - r, y + h, color);
    drawSoftLine(W, H, x, y + r, x, y + h - r, color);
    drawArc(W, H, x + r, y + r, r, (float)M_PI, 1.5f * (float)M_PI, color);
    drawArc(W, H, x + w - r, y + r, r, 1.5f * (float)M_PI, 2.0f * (float)M_PI, color);
    drawArc(W, H, x + w - r, y + h - r, r, 0.0f, 0.5f * (float)M_PI, color);
    drawArc(W, H, x + r, y + h - r, r, 0.5f * (float)M_PI, (float)M_PI, color);
}

void jpg::fillRoundedRect(int x, int y, int w, int h, int r, const ColorRGB &color) {
    int W = width, H = height; // 图像尺寸 (区别于矩形 w,h)
    if (w <= 0 || h <= 0 || r <= 0) {
        for (int row = y; row < y + h; ++row)
            for (int col = x; col < x + w; ++col)
                plotPixel(W, H, col, row, 1.0f, color);
        return;
    }
    r = std::min(r, std::min(w / 2, h / 2));
    for (int row = y; row < y + h; ++row) {
        int dy_top = row - y;
        int dy_bot = y + h - 1 - row;
        int left_bound = x, right_bound = x + w - 1;
        if (dy_top < r) {
            int arc_x = (int)(std::sqrt(r * r - (r - dy_top) * (r - dy_top)) + 0.5f);
            left_bound = x + r - arc_x;
            right_bound = x + w - 1 - (r - arc_x);
        } else if (dy_bot < r) {
            int arc_x = (int)(std::sqrt(r * r - (r - dy_bot) * (r - dy_bot)) + 0.5f);
            left_bound = x + r - arc_x;
            right_bound = x + w - 1 - (r - arc_x);
        }
        for (int col = left_bound; col <= right_bound; ++col)
            plotPixel(W, H, col, row, 1.0f, color);
    }
}

// ==================== 图像拷贝 / 融合 ====================
// 计算源区域与目标区域的裁剪范围 (处理越界与负偏移)
static bool calcCopyRange(int width, int height, int x1, int y1, int x2, int y2,
                          int pos_x, int pos_y, int dst_w, int dst_h,
                          int &sx0, int &sy0, int &dx0, int &dy0, int &cw, int &ch) {
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    x1 = std::max(x1, 0); y1 = std::max(y1, 0);
    x2 = std::min(x2, width - 1); y2 = std::min(y2, height - 1);
    if (x1 > x2 || y1 > y2) return false;
    sx0 = x1; sy0 = y1; dx0 = pos_x; dy0 = pos_y;
    if (dx0 < 0) { sx0 += -dx0; dx0 = 0; }
    if (dy0 < 0) { sy0 += -dy0; dy0 = 0; }
    cw = std::min(x2 - sx0 + 1, dst_w - dx0);
    ch = std::min(y2 - sy0 + 1, dst_h - dy0);
    return (cw > 0 && ch > 0);
}

void jpg::copyFrom(const std::vector<ColorRGB> &from_data, int src_w, int src_h,
                   int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A) {
    if (from_data.empty() || src_w <= 0 || src_h <= 0) return;
    if (pixels.empty() || this->width <= 0 || this->height <= 0) return;
    int sx0, sy0, dx0, dy0, cw, ch;
    if (!calcCopyRange(src_w, src_h, x1, y1, x2, y2, pos_x, pos_y, this->width, this->height,
                       sx0, sy0, dx0, dy0, cw, ch)) return;
    int ea = A, inv = 255 - ea;
    for (int j = 0; j < ch; j++) {
        int sy = sy0 + j, dy = dy0 + j;
        for (int i = 0; i < cw; i++) {
            int sx = sx0 + i, dx = dx0 + i;
            const ColorRGB &s = from_data[(size_t)sy * src_w + sx];
            ColorRGB &d = pixels[(size_t)dy * this->width + dx];
            if (ea >= 255) { d = s; }
            else {
                d.r = (uint8_t)((s.r * ea + d.r * inv + 127) / 255);
                d.g = (uint8_t)((s.g * ea + d.g * inv + 127) / 255);
                d.b = (uint8_t)((s.b * ea + d.b * inv + 127) / 255);
            }
        }
    }
}

void jpg::copy(const std::vector<ColorRGBA> &from_data, int src_w, int src_h,
               int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A) {
    if (from_data.empty() || src_w <= 0 || src_h <= 0) return;
    if (pixels.empty() || this->width <= 0 || this->height <= 0) return;
    int sx0, sy0, dx0, dy0, cw, ch;
    if (!calcCopyRange(src_w, src_h, x1, y1, x2, y2, pos_x, pos_y, this->width, this->height,
                       sx0, sy0, dx0, dy0, cw, ch)) return;
    for (int j = 0; j < ch; j++) {
        int sy = sy0 + j, dy = dy0 + j;
        for (int i = 0; i < cw; i++) {
            int sx = sx0 + i, dx = dx0 + i;
            const ColorRGBA &s = from_data[(size_t)sy * src_w + sx];
            ColorRGB &d = pixels[(size_t)dy * this->width + dx];
            int ea = s.a * A / 255; // 全局透明度 × 源alpha
            if (ea >= 255) { d.r = s.r; d.g = s.g; d.b = s.b; }
            else if (ea > 0) {
                int inv = 255 - ea;
                d.r = (uint8_t)((s.r * ea + d.r * inv + 127) / 255);
                d.g = (uint8_t)((s.g * ea + d.g * inv + 127) / 255);
                d.b = (uint8_t)((s.b * ea + d.b * inv + 127) / 255);
            }
        }
    }
}


std::vector<uint8_t> jpg::saveAndShow(const std::string& filename, int quality,
                                      bool progressive, bool subsample420, int bits_per_sample) const {
    std::vector<uint8_t> buf = imshow(quality, progressive, subsample420, bits_per_sample);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return {};
    ofs.write((const char*)buf.data(), buf.size());
    if (ofs.good()) return buf;
    return {};
}

bool jpg::save(const std::string& filename, int quality, bool progressive, bool subsample420, int bits_per_sample) const {
    std::vector<uint8_t> buf = imshow(quality, progressive, subsample420, bits_per_sample);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return false;
    ofs.write((const char*)buf.data(), buf.size());
    return ofs.good();
}

std::vector<uint8_t> jpg::imshow(int quality, bool progressive, bool subsample420, int bits_per_sample) const {
    if (width <= 0 || height <= 0 || pixels.empty()) return {};
    quality = std::max(10, std::min(100, quality));
    bits_per_sample = (bits_per_sample == 12) ? 12 : 8;

    // 生成量化表
    uint8_t lum_qt[64], chrom_qt[64];
    makeQuantTable(std_lum_quant, lum_qt, quality);
    makeQuantTable(std_chrom_quant, chrom_qt, quality);

    // 采样因子: 4:2:0 时 Y=2x2, 色度=1x1; 4:4:4 时全部 1x1
    int h_max = subsample420 ? 2 : 1;
    int v_max = subsample420 ? 2 : 1;

    // 构建优化 Huffman 表 (基于实际符号频率)
    int freq_dc_lum[256] = {}, freq_dc_chrom[256] = {};
    int freq_ac_lum[256] = {}, freq_ac_chrom[256] = {};

    // MCU 尺寸 = 8*h_max x 8*v_max 像素
    int mcu_w = (width + 8 * h_max - 1) / (8 * h_max);
    int mcu_h = (height + 8 * v_max - 1) / (8 * v_max);
    // Y 块网格 (全分辨率)
    int y_bw = mcu_w * h_max;
    int y_bh = mcu_h * v_max;
    // 色度块网格 (下采样后)
    int c_bw = mcu_w;
    int c_bh = mcu_h;
    int y_blocks = y_bw * y_bh;
    int c_blocks = c_bw * c_bh;

    // 1) 全分辨率 RGB -> YCbCr
    std::vector<float> y_full((size_t)width * height);
    std::vector<float> cb_full((size_t)width * height);
    std::vector<float> cr_full((size_t)width * height);
    float scale = (bits_per_sample == 12) ? 16.0f : 1.0f;
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            const ColorRGB& c = pixels[py * width + px];
            float y, cb, cr;
            rgbToYCbCr(c.r, c.g, c.b, y, cb, cr);
            size_t idx = (size_t)py * width + px;
            y_full[idx] = (y - 128.0f) * scale;
            cb_full[idx] = (cb - 128.0f) * scale;
            cr_full[idx] = (cr - 128.0f) * scale;
        }
    }

    // 2) 色度下采样 (4:2:0: 2x2 平均)
    int cw = (width + h_max - 1) / h_max;
    int ch = (height + v_max - 1) / v_max;
    std::vector<float> cb_half, cr_half;
    const float* cb_src = cb_full.data();
    const float* cr_src = cr_full.data();
    int csrc_w = width, csrc_h = height;
    if (subsample420) {
        cb_half.resize((size_t)cw * ch);
        cr_half.resize((size_t)cw * ch);
        for (int cy = 0; cy < ch; cy++) {
            for (int cx = 0; cx < cw; cx++) {
                float sb = 0, sr = 0; int cnt = 0;
                for (int dy = 0; dy < v_max; dy++) {
                    for (int dx = 0; dx < h_max; dx++) {
                        int px = std::min(cx * h_max + dx, width - 1);
                        int py = std::min(cy * v_max + dy, height - 1);
                        size_t idx = (size_t)py * width + px;
                        sb += cb_full[idx];
                        sr += cr_full[idx];
                        cnt++;
                    }
                }
                cb_half[(size_t)cy * cw + cx] = sb / cnt;
                cr_half[(size_t)cy * cw + cx] = sr / cnt;
            }
        }
        cb_src = cb_half.data();
        cr_src = cr_half.data();
        csrc_w = cw;
        csrc_h = ch;
    }

    // 3) 预计算所有量化后的 DCT 系数 (zigzag顺序)
    std::vector<int> coeff_y((size_t)y_blocks * 64);
    std::vector<int> coeff_cb((size_t)c_blocks * 64);
    std::vector<int> coeff_cr((size_t)c_blocks * 64);

    // Y 块 (全分辨率)
    for (int by = 0; by < y_bh; by++) {
        for (int bx = 0; bx < y_bw; bx++) {
            float block[64];
            for (int j = 0; j < 8; j++)
                for (int i = 0; i < 8; i++) {
                    int px = std::min(bx * 8 + i, width - 1);
                    int py = std::min(by * 8 + j, height - 1);
                    block[j * 8 + i] = y_full[(size_t)py * width + px];
                }
            fdct(block);
            int blk = by * y_bw + bx;
            for (int k = 0; k < 64; k++) {
                int zi = zigzag_order[k];
                coeff_y[(size_t)blk * 64 + k] = (int)roundf(block[zi] / (float)lum_qt[k]);
            }
        }
    }
    // 色度块 (下采样后分辨率)
    for (int by = 0; by < c_bh; by++) {
        for (int bx = 0; bx < c_bw; bx++) {
            float bb[64], br2[64];
            for (int j = 0; j < 8; j++)
                for (int i = 0; i < 8; i++) {
                    int px = std::min(bx * 8 + i, csrc_w - 1);
                    int py = std::min(by * 8 + j, csrc_h - 1);
                    bb[j * 8 + i] = cb_src[(size_t)py * csrc_w + px];
                    br2[j * 8 + i] = cr_src[(size_t)py * csrc_w + px];
                }
            fdct(bb); fdct(br2);
            int blk = by * c_bw + bx;
            for (int k = 0; k < 64; k++) {
                int zi = zigzag_order[k];
                coeff_cb[(size_t)blk * 64 + k] = (int)roundf(bb[zi] / (float)chrom_qt[k]);
                coeff_cr[(size_t)blk * 64 + k] = (int)roundf(br2[zi] / (float)chrom_qt[k]);
            }
        }
    }

    // 统计频率
    // 注意: DC 必须按 MCU 编码顺序统计 (与编码时的 DC 预测链一致),
    //       否则 4:2:0 等 Y块非线性顺序时会漏统计某些 DC 差分符号。
    {
        int prev = 0;
        for (int my = 0; my < mcu_h; my++)
            for (int mx = 0; mx < mcu_w; mx++)
                for (int vy = 0; vy < v_max; vy++)
                    for (int vx = 0; vx < h_max; vx++) {
                        int yb = (my * v_max + vy) * y_bw + (mx * h_max + vx);
                        int dc = coeff_y[(size_t)yb * 64];
                        freq_dc_lum[bitCategory(dc - prev)]++;
                        prev = dc;
                    }
    }
    {
        int prevb = 0, prevr = 0;
        for (int my = 0; my < mcu_h; my++)
            for (int mx = 0; mx < mcu_w; mx++) {
                int cb = my * c_bw + mx;
                int dcb = coeff_cb[(size_t)cb * 64];
                int dcr = coeff_cr[(size_t)cb * 64];
                freq_dc_chrom[bitCategory(dcb - prevb)]++;
                freq_dc_chrom[bitCategory(dcr - prevr)]++;
                prevb = dcb; prevr = dcr;
            }
    }
    // AC 符号与块顺序无关, 线性统计即可
    countACFreq(coeff_y, y_blocks, freq_ac_lum);
    countACFreq(coeff_cb, c_blocks, freq_ac_chrom);
    countACFreq(coeff_cr, c_blocks, freq_ac_chrom);

    // 构建优化表
    uint8_t opt_dc_lum_bits[16], opt_dc_lum_vals[256];
    uint8_t opt_dc_chrom_bits[16], opt_dc_chrom_vals[256];
    uint8_t opt_ac_lum_bits[16], opt_ac_lum_vals[256];
    uint8_t opt_ac_chrom_bits[16], opt_ac_chrom_vals[256];
    int n_dc_lum = buildOptimalHuffTable(freq_dc_lum, opt_dc_lum_bits, opt_dc_lum_vals);
    int n_dc_chrom = buildOptimalHuffTable(freq_dc_chrom, opt_dc_chrom_bits, opt_dc_chrom_vals);
    int n_ac_lum = buildOptimalHuffTable(freq_ac_lum, opt_ac_lum_bits, opt_ac_lum_vals);
    int n_ac_chrom = buildOptimalHuffTable(freq_ac_chrom, opt_ac_chrom_bits, opt_ac_chrom_vals);

    HuffTable dc_lum_ht, dc_chrom_ht, ac_lum_ht, ac_chrom_ht;
    memcpy(dc_lum_ht.bits, opt_dc_lum_bits, 16);
    memcpy(dc_lum_ht.vals, opt_dc_lum_vals, n_dc_lum);
    dc_lum_ht.build();
    memcpy(dc_chrom_ht.bits, opt_dc_chrom_bits, 16);
    memcpy(dc_chrom_ht.vals, opt_dc_chrom_vals, n_dc_chrom);
    dc_chrom_ht.build();
    memcpy(ac_lum_ht.bits, opt_ac_lum_bits, 16);
    memcpy(ac_lum_ht.vals, opt_ac_lum_vals, n_ac_lum);
    ac_lum_ht.build();
    memcpy(ac_chrom_ht.bits, opt_ac_chrom_bits, 16);
    memcpy(ac_chrom_ht.vals, opt_ac_chrom_vals, n_ac_chrom);
    ac_chrom_ht.build();

    std::vector<uint8_t> buf;
    buf.reserve(width * height * 3);

    // SOI
    writeWord(buf, 0xFFD8);
    
    // 插入保存的元数据 (保持顺序: APP0 -> EXIF -> ICC -> XMP -> 其他)
    if (!metadata.app0.empty()) {
        buf.insert(buf.end(), metadata.app0.begin(), metadata.app0.end());
    } else {
        // APP0 (JFIF)
        writeWord(buf, 0xFFE0);
        writeWord(buf, 16);
        buf.push_back('J'); buf.push_back('F'); buf.push_back('I'); buf.push_back('F'); buf.push_back(0);
        writeWord(buf, 0x0101);
        writeByte(buf, 0);
        writeWord(buf, 1); writeWord(buf, 1);
        writeByte(buf, 0); writeByte(buf, 0);
    }
    
    if (!metadata.exif.empty()) {
        buf.insert(buf.end(), metadata.exif.begin(), metadata.exif.end());
    }
    
    if (!metadata.icc_profile.empty()) {
        buf.insert(buf.end(), metadata.icc_profile.begin(), metadata.icc_profile.end());
    }
    
    if (!metadata.xmp.empty()) {
        buf.insert(buf.end(), metadata.xmp.begin(), metadata.xmp.end());
    }
    
    if (!metadata.other_apps.empty()) {
        buf.insert(buf.end(), metadata.other_apps.begin(), metadata.other_apps.end());
    }

    // DQT
    writeDQT(buf, 0, lum_qt, bits_per_sample);
    writeDQT(buf, 1, chrom_qt, bits_per_sample);

    // SOF
    if (progressive) {
        writeWord(buf, 0xFFC2); // SOF2 progressive
    } else {
        writeWord(buf, 0xFFC0); // SOF0 baseline
    }
    writeWord(buf, 2 + 1 + 2 + 2 + 1 + 3 * 3);
    writeByte(buf, bits_per_sample);
    writeWord(buf, (uint16_t)height);
    writeWord(buf, (uint16_t)width);
    writeByte(buf, 3);
    // Y: 采样 h_max x v_max, 量化表0; Cb/Cr: 1x1, 量化表1
    writeByte(buf, 1); writeByte(buf, (uint8_t)((h_max << 4) | v_max)); writeByte(buf, 0);
    writeByte(buf, 2); writeByte(buf, 0x11); writeByte(buf, 1);
    writeByte(buf, 3); writeByte(buf, 0x11); writeByte(buf, 1);

    if (!progressive) {
        // ===== Baseline 编码 =====
        writeDHT(buf, 0x00, opt_dc_lum_bits, opt_dc_lum_vals, n_dc_lum);
        writeDHT(buf, 0x10, opt_ac_lum_bits, opt_ac_lum_vals, n_ac_lum);
        writeDHT(buf, 0x01, opt_dc_chrom_bits, opt_dc_chrom_vals, n_dc_chrom);
        writeDHT(buf, 0x11, opt_ac_chrom_bits, opt_ac_chrom_vals, n_ac_chrom);

        writeWord(buf, 0xFFDA);
        writeWord(buf, 2 + 1 + 3 * 2 + 3);
        writeByte(buf, 3);
        writeByte(buf, 1); writeByte(buf, 0x00);
        writeByte(buf, 2); writeByte(buf, 0x11);
        writeByte(buf, 3); writeByte(buf, 0x11);
        writeByte(buf, 0); writeByte(buf, 63); writeByte(buf, 0);

        std::vector<uint8_t> scan_data;
        BitWriter bw(scan_data);
        int prev_dc_y = 0, prev_dc_cb = 0, prev_dc_cr = 0;

        // 编码单个块 (DC + AC)
        auto encBlock = [&](const std::vector<int>& coeff, int blk,
                            const HuffTable& dcht, const HuffTable& acht, int& prev_dc) {
            const int* cf = &coeff[(size_t)blk * 64];
            int dc_diff = cf[0] - prev_dc;
            prev_dc = cf[0];
            int cat = bitCategory(dc_diff);
            bw.writeBits(dcht.enc_code[cat], dcht.enc_len[cat]);
            if (cat > 0) bw.writeBits(encodeValue(dc_diff, cat), cat);
            int zero_run = 0;
            for (int k = 1; k < 64; k++) {
                int v = cf[k];
                if (v == 0) { zero_run++; continue; }
                while (zero_run > 15) { bw.writeBits(acht.enc_code[0xF0], acht.enc_len[0xF0]); zero_run -= 16; }
                int ac = bitCategory(v);
                uint8_t sym = (uint8_t)((zero_run << 4) | ac);
                bw.writeBits(acht.enc_code[sym], acht.enc_len[sym]);
                bw.writeBits(encodeValue(v, ac), ac);
                zero_run = 0;
            }
            if (zero_run > 0) bw.writeBits(acht.enc_code[0], acht.enc_len[0]);
        };

        // 按 MCU 顺序编码: 每MCU含 v_max*h_max 个Y块 + 1个Cb块 + 1个Cr块
        for (int my = 0; my < mcu_h; my++) {
            for (int mx = 0; mx < mcu_w; mx++) {
                for (int vy = 0; vy < v_max; vy++)
                    for (int vx = 0; vx < h_max; vx++) {
                        int yb = (my * v_max + vy) * y_bw + (mx * h_max + vx);
                        encBlock(coeff_y, yb, dc_lum_ht, ac_lum_ht, prev_dc_y);
                    }
                int cb = my * c_bw + mx;
                encBlock(coeff_cb, cb, dc_chrom_ht, ac_chrom_ht, prev_dc_cb);
                encBlock(coeff_cr, cb, dc_chrom_ht, ac_chrom_ht, prev_dc_cr);
            }
        }
        bw.flush();
        buf.insert(buf.end(), scan_data.begin(), scan_data.end());

    } else {
        // ===== Progressive 编码 (2次扫描: DC + AC) =====

        // --- Scan 1: DC 系数 (Ss=0, Se=0, Ah=0, Al=0) ---
        writeDHT(buf, 0x00, opt_dc_lum_bits, opt_dc_lum_vals, n_dc_lum);
        writeDHT(buf, 0x01, opt_dc_chrom_bits, opt_dc_chrom_vals, n_dc_chrom);

        writeWord(buf, 0xFFDA);
        writeWord(buf, 2 + 1 + 3 * 2 + 3);
        writeByte(buf, 3);
        writeByte(buf, 1); writeByte(buf, 0x00); // Y: DC表0
        writeByte(buf, 2); writeByte(buf, 0x11); // Cb: DC表1
        writeByte(buf, 3); writeByte(buf, 0x11); // Cr: DC表1
        writeByte(buf, 0);  // Ss
        writeByte(buf, 0);  // Se
        writeByte(buf, 0);  // Ah=0, Al=0

        {
            std::vector<uint8_t> scan_data;
            BitWriter bw(scan_data);
            int prev_dc[3] = {0, 0, 0};
            // 编码单个DC系数
            auto encDC = [&](int dc_val, const HuffTable& ht, int& prev) {
                int dc_diff = dc_val - prev;
                prev = dc_val;
                int cat = bitCategory(dc_diff);
                bw.writeBits(ht.enc_code[cat], ht.enc_len[cat]);
                if (cat > 0) bw.writeBits(encodeValue(dc_diff, cat), cat);
            };
            // 按MCU顺序: 每MCU含 v_max*h_max 个Y的DC + 1个Cb + 1个Cr
            for (int my = 0; my < mcu_h; my++) {
                for (int mx = 0; mx < mcu_w; mx++) {
                    for (int vy = 0; vy < v_max; vy++)
                        for (int vx = 0; vx < h_max; vx++) {
                            int yb = (my * v_max + vy) * y_bw + (mx * h_max + vx);
                            encDC(coeff_y[(size_t)yb * 64], dc_lum_ht, prev_dc[0]);
                        }
                    int cb = my * c_bw + mx;
                    encDC(coeff_cb[(size_t)cb * 64], dc_chrom_ht, prev_dc[1]);
                    encDC(coeff_cr[(size_t)cb * 64], dc_chrom_ht, prev_dc[2]);
                }
            }
            bw.flush();
            buf.insert(buf.end(), scan_data.begin(), scan_data.end());
        }

        // --- AC 扫描: 每个分量单独一个扫描 (progressive 标准要求 AC 扫描 ns=1) ---
        writeDHT(buf, 0x10, opt_ac_lum_bits, opt_ac_lum_vals, n_ac_lum);
        writeDHT(buf, 0x11, opt_ac_chrom_bits, opt_ac_chrom_vals, n_ac_chrom);

        for (int c = 0; c < 3; c++) {
            int comp_id = c + 1;              // 1=Y, 2=Cb, 3=Cr
            uint8_t ta = (c == 0) ? 0x00 : 0x11; // Y用AC表0, Cb/Cr用AC表1
            const HuffTable* ac_ht = (c == 0) ? &ac_lum_ht : &ac_chrom_ht;
            const std::vector<int>* coeff = (c == 0) ? &coeff_y : (c == 1 ? &coeff_cb : &coeff_cr);
            int nblocks = (c == 0) ? y_blocks : c_blocks;

            writeWord(buf, 0xFFDA);
            writeWord(buf, 2 + 1 + 1 * 2 + 3);
            writeByte(buf, 1);                 // ns=1 (单分量)
            writeByte(buf, (uint8_t)comp_id); writeByte(buf, ta);
            writeByte(buf, 1);   // Ss
            writeByte(buf, 63);  // Se
            writeByte(buf, 0);   // Ah=0, Al=0

            std::vector<uint8_t> scan_data;
            BitWriter bw(scan_data);
            for (int blk = 0; blk < nblocks; blk++) {
                const int* cf = &(*coeff)[(size_t)blk * 64];
                int zero_run = 0;
                for (int k = 1; k < 64; k++) {
                    int v = cf[k];
                    if (v == 0) { zero_run++; continue; }
                    while (zero_run > 15) {
                        bw.writeBits(ac_ht->enc_code[0xF0], ac_ht->enc_len[0xF0]);
                        zero_run -= 16;
                    }
                    int ac = bitCategory(v);
                    uint8_t sym = (uint8_t)((zero_run << 4) | ac);
                    bw.writeBits(ac_ht->enc_code[sym], ac_ht->enc_len[sym]);
                    bw.writeBits(encodeValue(v, ac), ac);
                    zero_run = 0;
                }
                if (zero_run > 0)
                    bw.writeBits(ac_ht->enc_code[0], ac_ht->enc_len[0]);
            }
            bw.flush();
            buf.insert(buf.end(), scan_data.begin(), scan_data.end());
        }
    }

    // EOI
    writeWord(buf, 0xFFD9);
    return buf;
}

bool jpg::read(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return false;

    std::vector<uint8_t> file((std::istreambuf_iterator<char>(ifs)),
                               std::istreambuf_iterator<char>());
    if (file.size() < 4) return false;

    size_t pos = 0;
    auto readU8 = [&]() -> uint8_t { return (pos < file.size()) ? file[pos++] : 0; };
    auto readU16 = [&]() -> uint16_t {
        uint16_t hi = readU8();
        uint16_t lo = readU8();
        return (hi << 8) | lo;
    };

    // 验证 SOI
    if (readU8() != 0xFF || readU8() != 0xD8) return false;

    uint8_t quant_tables[4][64];
    memset(quant_tables, 0, sizeof(quant_tables));
    HuffTable huff_tables[8]; // 0=DC0,1=DC1,2=DC2,3=DC3, 4=AC0,5=AC1,6=AC2,7=AC3

    int img_w = 0, img_h = 0;
    int num_comp = 0;
    uint8_t comp_id[4], comp_samp[4], comp_qt[4];
    bool is_progressive = false;

    std::vector<ScanInfo> scans;

    // 解析所有标记段
    while (pos + 1 < file.size()) {
        if (file[pos] != 0xFF) { pos++; continue; }
        uint8_t marker = file[pos + 1];
        
        pos += 2;

        if (marker == 0xD9) break; // EOI
        if (marker == 0xDA) { // SOS
            ScanInfo si;
            memset(&si, 0, sizeof(si));
            readU16(); // seg_len
            si.ns = readU8();
            for (int i = 0; i < si.ns && i < 4; i++) {
                si.comp_sel[i] = readU8();
                si.td_ta[i] = readU8();
            }
            si.Ss = readU8();
            si.Se = readU8();
            uint8_t ah_al = readU8();
            si.Ah = (ah_al >> 4) & 0x0F;
            si.Al = ah_al & 0x0F;
            // 扫描数据: 从当前位置到下一个标记
            si.data = &file[pos];
            size_t end = pos;
            while (end + 1 < file.size()) {
                if (file[end] == 0xFF) {
                    if (file[end + 1] == 0x00) {
                        end += 2;
                    } else if (file[end + 1] >= 0xD0 && file[end + 1] <= 0xD7) {
                        end += 2;
                    } else {
                        break;
                    }
                } else {
                    end++;
                }
            }
            si.len = end - pos;
            pos = end;
            // 快照当前Huffman表 (progressive 可能在扫描间重定义表)
            memcpy(si.tables, huff_tables, sizeof(huff_tables));
            scans.push_back(si);
            continue;
        }

        if (marker == 0xD8 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7))
            continue;

        uint16_t seg_len = readU16();
        size_t seg_end = pos + seg_len - 2;

        // 保存扩展标记数据
        if (marker == 0xE0) { // APP0 (JFIF)
            metadata.app0.resize(seg_len + 2);
            metadata.app0[0] = 0xFF;
            metadata.app0[1] = 0xE0;
            metadata.app0[2] = (seg_len >> 8) & 0xFF;
            metadata.app0[3] = seg_len & 0xFF;
            for (size_t i = 4; i < seg_len + 2 && pos < seg_end; i++) {
                metadata.app0[i] = readU8();
            }
            pos = seg_end;
            continue;
        } else if (marker == 0xE1) { // APP1 (EXIF)
            metadata.exif.resize(seg_len + 2);
            metadata.exif[0] = 0xFF;
            metadata.exif[1] = 0xE1;
            metadata.exif[2] = (seg_len >> 8) & 0xFF;
            metadata.exif[3] = seg_len & 0xFF;
            for (size_t i = 4; i < seg_len + 2 && pos < seg_end; i++) {
                metadata.exif[i] = readU8();
            }
            pos = seg_end;
            continue;
        } else if (marker == 0xE2) { // APP2 (ICC Profile)
            metadata.icc_profile.resize(seg_len + 2);
            metadata.icc_profile[0] = 0xFF;
            metadata.icc_profile[1] = 0xE2;
            metadata.icc_profile[2] = (seg_len >> 8) & 0xFF;
            metadata.icc_profile[3] = seg_len & 0xFF;
            for (size_t i = 4; i < seg_len + 2 && pos < seg_end; i++) {
                metadata.icc_profile[i] = readU8();
            }
            pos = seg_end;
            continue;
        } else if (marker == 0xEF) { // APP15 (XMP)
            metadata.xmp.resize(seg_len + 2);
            metadata.xmp[0] = 0xFF;
            metadata.xmp[1] = 0xEF;
            metadata.xmp[2] = (seg_len >> 8) & 0xFF;
            metadata.xmp[3] = seg_len & 0xFF;
            for (size_t i = 4; i < seg_len + 2 && pos < seg_end; i++) {
                metadata.xmp[i] = readU8();
            }
            pos = seg_end;
            continue;
        } else if (marker >= 0xE0 && marker <= 0xEF) { // 其他 APP 标记
            metadata.other_apps.push_back(0xFF);
            metadata.other_apps.push_back(marker);
            metadata.other_apps.push_back((seg_len >> 8) & 0xFF);
            metadata.other_apps.push_back(seg_len & 0xFF);
            for (size_t i = 0; i < seg_len - 2 && pos < seg_end; i++) {
                metadata.other_apps.push_back(readU8());
            }
            pos = seg_end;
            continue;
        }

        if (marker == 0xDB) { // DQT
            while (pos < seg_end) {
                uint8_t info = readU8();
                int tid = info & 0x0F;
                int prec = (info >> 4) & 0x0F;
                if (prec == 0) {
                    for (int i = 0; i < 64; i++) quant_tables[tid][i] = readU8();
                } else {
                    for (int i = 0; i < 64; i++) quant_tables[tid][i] = (uint8_t)readU16();
                }
            }
        } else if (marker == 0xC0 || marker == 0xC1) { // SOF0/SOF1 baseline
            uint8_t prec = readU8();
            if (prec != 8 && prec != 12) return false;
            img_h = readU16();
            img_w = readU16();
            if (img_w <= 0 || img_h <= 0 || img_w > 65535 || img_h > 65535) return false;
            num_comp = readU8();
            if (num_comp < 1 || num_comp > 4) return false;
            for (int i = 0; i < num_comp; i++) {
                comp_id[i] = readU8();
                comp_samp[i] = readU8();
                comp_qt[i] = readU8();
            }
        } else if (marker == 0xC2) { // SOF2 progressive
            is_progressive = true;
            uint8_t prec = readU8();
            if (prec != 8 && prec != 12) return false;
            img_h = readU16();
            img_w = readU16();
            if (img_w <= 0 || img_h <= 0 || img_w > 65535 || img_h > 65535) return false;
            num_comp = readU8();
            if (num_comp < 1 || num_comp > 4) return false;
            for (int i = 0; i < num_comp; i++) {
                comp_id[i] = readU8();
                comp_samp[i] = readU8();
                comp_qt[i] = readU8();
            }
        } else if (marker == 0xC4) { // DHT
            while (pos < seg_end) {
                uint8_t info = readU8();
                int cls = (info >> 4) & 0x0F;
                int tid = info & 0x0F;
                int ht_idx = cls * 4 + tid;
                if (ht_idx < 0 || ht_idx >= 8) { pos = seg_end; break; }
                HuffTable& ht = huff_tables[ht_idx];
                int total = 0;
                for (int i = 0; i < 16; i++) {
                    ht.bits[i] = readU8();
                    total += ht.bits[i];
                }
                for (int i = 0; i < total && i < 256; i++) {
                    ht.vals[i] = readU8();
                }
                ht.build();
            }
        }
        pos = seg_end;
    }

    if (scans.empty() || img_w <= 0 || img_h <= 0 || num_comp < 1) return false;

    int h_max = 1, v_max = 1;
    for (int c = 0; c < num_comp; c++) {
        h_max = std::max(h_max, (int)((comp_samp[c] >> 4) & 0x0F));
        v_max = std::max(v_max, (int)(comp_samp[c] & 0x0F));
    }

    int mcu_cols = (img_w + h_max * 8 - 1) / (h_max * 8);
    int mcu_rows = (img_h + v_max * 8 - 1) / (v_max * 8);

    // 分量缓冲: 存储量化后的 DCT 系数 (zigzag顺序)
    // comp_coeffs[c][block_idx * 64 + k]
    int comp_blocks[4]; // 每个分量的块数
    std::vector<std::vector<int>> comp_coeffs(num_comp);
    int comp_bw[4], comp_bh[4]; // 分量像素宽高
    for (int c = 0; c < num_comp; c++) {
        int hc = (comp_samp[c] >> 4) & 0x0F;
        int vc = comp_samp[c] & 0x0F;
        comp_bw[c] = mcu_cols * hc * 8;
        comp_bh[c] = mcu_rows * vc * 8;
        int blocks_x = mcu_cols * hc;
        int blocks_y = mcu_rows * vc;
        comp_blocks[c] = blocks_x * blocks_y;
        comp_coeffs[c].resize((size_t)comp_blocks[c] * 64, 0);
    }

    // 找到分量索引
    auto findComp = [&](uint8_t cid) -> int {
        for (int c = 0; c < num_comp; c++)
            if (comp_id[c] == cid) return c;
        return -1;
    };

    if (!is_progressive) {
        // ===== Baseline 解码 =====
        const ScanInfo& si = scans[0];
        BitReader br(si.data, si.len);
        int prev_dc[4] = {0, 0, 0, 0};

        for (int mcu_r = 0; mcu_r < mcu_rows; mcu_r++) {
            for (int mcu_c = 0; mcu_c < mcu_cols; mcu_c++) {
                for (int c = 0; c < num_comp; c++) {
                    int hc = (comp_samp[c] >> 4) & 0x0F;
                    int vc = comp_samp[c] & 0x0F;
                    int dc_idx = (si.td_ta[0] >> 4) & 0x0F;
                    int ac_idx = 4 + (si.td_ta[0] & 0x0F);
                    // 查找对应分量的表
                    for (int s = 0; s < si.ns; s++) {
                        if (findComp(si.comp_sel[s]) == c) {
                            dc_idx = (si.td_ta[s] >> 4) & 0x0F;
                            ac_idx = 4 + (si.td_ta[s] & 0x0F);
                        }
                    }
                    int qt_id = comp_qt[c];
                    int blocks_x = mcu_cols * hc;

                    for (int v = 0; v < vc; v++) {
                        for (int h = 0; h < hc; h++) {
                            float block[64];
                            if (!decodeBlock(br, block, quant_tables[qt_id],
                                             prev_dc[c], si.tables[dc_idx], si.tables[ac_idx]))
                                return false;
                            idct(block);
                            // 直接写入像素缓冲(稍后统一转换)
                            int blk_idx = (mcu_r * vc + v) * blocks_x + (mcu_c * hc + h);
                            // baseline: 直接存像素值到 comp_coeffs 临时用作像素缓冲
                            for (int j = 0; j < 8; j++)
                                for (int i = 0; i < 8; i++)
                                    comp_coeffs[c][blk_idx * 64 + j * 8 + i] =
                                        (int)(block[j * 8 + i] + 128.5f);
                        }
                    }
                }
            }
        }

        // 重建图像
        width = img_w; height = img_h;
        pixels.resize((size_t)img_w * img_h);
        for (int y = 0; y < img_h; y++) {
            for (int x = 0; x < img_w; x++) {
                float yv, cbv, crv;
                if (num_comp == 1) {
                    int hc0 = (comp_samp[0] >> 4) & 0x0F;
                    int blk_x = x / 8, blk_y = y / 8;
                    int blocks_x0 = mcu_cols * hc0;
                    yv = comp_coeffs[0][(blk_y * blocks_x0 + blk_x) * 64 + (y%8)*8 + x%8];
                    cbv = 128; crv = 128;
                } else {
                    auto sample = [&](int c, int px, int py) -> float {
                        int hc = (comp_samp[c] >> 4) & 0x0F;
                        int vc = comp_samp[c] & 0x0F;
                        int sx = px * hc / h_max;
                        int sy = py * vc / v_max;
                        int blocks_x = mcu_cols * hc;
                        int blocks_y = mcu_rows * vc;
                        int max_sx = blocks_x * 8 - 1;
                        int max_sy = blocks_y * 8 - 1;
                        sx = std::min(std::max(sx, 0), max_sx);
                        sy = std::min(std::max(sy, 0), max_sy);
                        int bx = sx / 8, by = sy / 8;
                        int blk = by * blocks_x + bx;
                        return (float)comp_coeffs[c][blk * 64 + (sy%8)*8 + sx%8];
                    };
                    yv = sample(0, x, y);
                    cbv = sample(1, x, y);
                    crv = sample(2, x, y);
                }
                uint8_t r, g, b;
            yCbCrToRgb(yv, cbv, crv, r, g, b);
            pixels[y * img_w + x] = {r, g, b};
        }
    }
    metadata.parseExif();
    return true;
}

// ===== Progressive 解码 =====
    int prev_dc[4] = {0, 0, 0, 0};

    for (size_t scan_idx = 0; scan_idx < scans.size(); scan_idx++) {
        auto& si = scans[scan_idx];
        BitReader br(si.data, si.len);
        int Al = si.Al;
        int eobrun[4] = {0, 0, 0, 0}; // EOB游程计数器 (每分量)

        for (int mcu_r = 0; mcu_r < mcu_rows; mcu_r++) {
            for (int mcu_c = 0; mcu_c < mcu_cols; mcu_c++) {
                for (int s = 0; s < si.ns; s++) {
                    int c = findComp(si.comp_sel[s]);
                    if (c < 0) continue;
                    int hc = (comp_samp[c] >> 4) & 0x0F;
                    int vc = comp_samp[c] & 0x0F;
                    int dc_tbl = (si.td_ta[s] >> 4) & 0x0F;
                    int ac_tbl = 4 + (si.td_ta[s] & 0x0F);
                    int blocks_x = mcu_cols * hc;

                    for (int v = 0; v < vc; v++) {
                        for (int h = 0; h < hc; h++) {
                            int blk_idx = (mcu_r * vc + v) * blocks_x + (mcu_c * hc + h);
                            int* coeff = &comp_coeffs[c][(size_t)blk_idx * 64];

                            if (si.Ss == 0) {
                                // DC 扫描
                                if (si.Ah == 0) {
                                    // DC 初始
                                    int cat = huffDecodeSymbol(br, si.tables[dc_tbl]);
                                    if (cat < 0) return false;
                                    int diff;
                                    receiveExtend(br, cat, diff);
                                    prev_dc[c] += diff;
                                    coeff[0] = prev_dc[c] << Al;
                                } else {
                                    // DC 精化: 补上低位 bit
                                    int bit = br.readBit();
                                    coeff[0] |= (bit << Al);
                                }
                            } else {
                                // AC 扫描
                                if (si.Ah == 0) {
                                    // AC 初始: EOB游程跳过整块
                                    if (eobrun[c] > 0) {
                                        eobrun[c]--;
                                    } else {
                                        int k = si.Ss;
                                        while (k <= si.Se) {
                                            int sym = huffDecodeSymbol(br, si.tables[ac_tbl]);
                                            if (sym < 0) return false;
                                            int run = (sym >> 4) & 0x0F;
                                            int cat = sym & 0x0F;
                                            if (cat == 0) {
                                                if (run == 15) { k += 16; continue; } // ZRL
                                                eobrun[c] = (1 << run) + br.readBits(run) - 1; // EOB游程
                                                break;
                                            }
                                            k += run;
                                            if (k > si.Se) break;
                                            int val;
                                            receiveExtend(br, cat, val);
                                            coeff[k] = val << Al; // zigzag索引存储
                                            k++;
                                        }
                                    }
                                } else {
                                    // AC 精化 (Ah > 0): 按 libjpeg decode_mcu_AC_refine
                                    // 关键: EOB游程不跳过整块, 仍需精化剩余非零系数
                                    int p1 = 1 << Al;
                                    int m1 = -p1;
                                    int k = si.Ss;
                                    if (eobrun[c] == 0) {
                                        while (k <= si.Se) {
                                            int sym = huffDecodeSymbol(br, si.tables[ac_tbl]);
                                            if (sym < 0) return false;
                                            int run = (sym >> 4) & 0x0F;
                                            int cat = sym & 0x0F;
                                            int newval = 0;
                                            if (cat != 0) {
                                                // 新系数: 读符号位 (cat 应为1)
                                                newval = br.readBit() ? p1 : m1;
                                            } else if (run != 15) {
                                                // EOB: 游程 = 2^run + 附加位 (不在此处-1)
                                                eobrun[c] = 1 << run;
                                                if (run) eobrun[c] += br.readBits(run);
                                                break; // 剩余系数由下方 EOB 逻辑处理
                                            }
                                            // ZRL(run==15) 或 新系数: 前进跳过已非零(精化)和run个零
                                            do {
                                                if (coeff[k] != 0) {
                                                    if (br.readBit()) {
                                                        if ((coeff[k] & p1) == 0)
                                                            coeff[k] += (coeff[k] >= 0) ? p1 : m1;
                                                    }
                                                } else {
                                                    if (--run < 0) break; // 到达目标零系数
                                                }
                                                k++;
                                            } while (k <= si.Se);
                                            if (newval != 0) coeff[k] = newval; // 放置新系数
                                            k++; // 对应 libjpeg for 循环的 k++ (越过已处理位置)
                                        }
                                    }
                                    if (eobrun[c] > 0) {
                                        // 精化剩余系数 (k 到 Se), 然后计一个块
                                        for (; k <= si.Se; k++) {
                                            if (coeff[k] != 0) {
                                                if (br.readBit()) {
                                                    if ((coeff[k] & p1) == 0)
                                                        coeff[k] += (coeff[k] >= 0) ? p1 : m1;
                                                }
                                            }
                                        }
                                        eobrun[c]--;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 反量化 + IDCT + 重建图像
    width = img_w; height = img_h;
    pixels.resize((size_t)img_w * img_h);

    // 先对每个分量做反量化+IDCT，存入像素缓冲
    std::vector<std::vector<float>> comp_pixels(num_comp);
    for (int c = 0; c < num_comp; c++) {
        comp_pixels[c].resize((size_t)comp_bw[c] * comp_bh[c], 128.0f);
        int hc = (comp_samp[c] >> 4) & 0x0F;
        int vc = comp_samp[c] & 0x0F;
        int blocks_x = mcu_cols * hc;
        int qt_id = comp_qt[c];

        for (int by = 0; by < mcu_rows * vc; by++) {
            for (int bx = 0; bx < blocks_x; bx++) {
                int blk_idx = by * blocks_x + bx;
                int* coeff = &comp_coeffs[c][(size_t)blk_idx * 64];
                float block[64];
                // 反量化 (zigzag顺序 -> 空间位置)
                for (int k = 0; k < 64; k++) {
                    int zi = zigzag_order[k];
                    block[zi] = (float)(coeff[k] * quant_tables[qt_id][k]);
                }
                idct(block);
                // 写入分量像素缓冲
                for (int j = 0; j < 8; j++) {
                    for (int i = 0; i < 8; i++) {
                        int px = bx * 8 + i;
                        int py = by * 8 + j;
                        if (px < comp_bw[c] && py < comp_bh[c])
                            comp_pixels[c][py * comp_bw[c] + px] = block[j * 8 + i] + 128.0f;
                    }
                }
            }
        }
    }

    // 颜色空间转换 + 上采样
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            float yv, cbv, crv;
            if (num_comp == 1) {
                yv = comp_pixels[0][y * comp_bw[0] + x];
                cbv = 128; crv = 128;
            } else {
                auto sampleComp = [&](int c, int px, int py) -> float {
                    int hc = (comp_samp[c] >> 4) & 0x0F;
                    int vc = comp_samp[c] & 0x0F;
                    int sx = px * hc / h_max;
                    int sy = py * vc / v_max;
                    return comp_pixels[c][sy * comp_bw[c] + sx];
                };
                yv = sampleComp(0, x, y);
                cbv = sampleComp(1, x, y);
                crv = sampleComp(2, x, y);
            }
            uint8_t r, g, b;
            yCbCrToRgb(yv, cbv, crv, r, g, b);
            pixels[y * img_w + x] = {r, g, b};
        }
    }
    metadata.parseExif();
    return true;
}

void JPEGMetadata::parseExif() {
    if (exif.size() < 20) return;
    
    size_t pos = 0;
    if (exif[pos] != 0xFF || exif[pos+1] != 0xE1) return;
    pos += 4;
    
    if (exif.size() - pos < 6 || 
        exif[pos] != 'E' || exif[pos+1] != 'x' || 
        exif[pos+2] != 'i' || exif[pos+3] != 'f' ||
        exif[pos+4] != 0 || exif[pos+5] != 0) {
        return;
    }
    size_t tiff_start = pos + 6;
    pos = tiff_start;
    
    if (exif.size() - pos < 8) return;
    
    bool is_little_endian = (exif[pos] == 'I' && exif[pos+1] == 'I');
    pos += 2;
    
    uint16_t magic = (is_little_endian) 
        ? (exif[pos] | (exif[pos+1] << 8))
        : ((exif[pos] << 8) | exif[pos+1]);
    if (magic != 0x002A) return;
    pos += 2;
    
    uint32_t ifd_offset = (is_little_endian)
        ? (exif[pos] | (exif[pos+1] << 8) | (exif[pos+2] << 16) | (exif[pos+3] << 24))
        : ((exif[pos] << 24) | (exif[pos+1] << 16) | (exif[pos+2] << 8) | exif[pos+3]);
    pos += 4;
    
    size_t ifd_pos = tiff_start + ifd_offset;
    if (ifd_pos >= exif.size()) return;
    pos = ifd_pos;
    
    if (exif.size() - pos < 2) return;
    uint16_t num_entries = (is_little_endian)
        ? (exif[pos] | (exif[pos+1] << 8))
        : ((exif[pos] << 8) | exif[pos+1]);
    pos += 2;
    
    auto readU16 = [&](size_t p) -> uint16_t {
        if (p + 1 >= exif.size()) return 0;
        return (is_little_endian)
            ? (exif[p] | (exif[p+1] << 8))
            : ((exif[p] << 8) | exif[p+1]);
    };
    
    auto readU32 = [&](size_t p) -> uint32_t {
        if (p + 3 >= exif.size()) return 0;
        return (is_little_endian)
            ? (exif[p] | (exif[p+1] << 8) | (exif[p+2] << 16) | (exif[p+3] << 24))
            : ((exif[p] << 24) | (exif[p+1] << 16) | (exif[p+2] << 8) | exif[p+3]);
    };
    
    for (uint16_t i = 0; i < num_entries && pos + 11 < exif.size(); i++) {
        uint16_t tag = readU16(pos);
        uint16_t format = readU16(pos + 2);
        uint32_t count = readU32(pos + 4);
        uint32_t value_offset = readU32(pos + 8);
        pos += 12;
        
        if (tag == 0x010F) {
            if (format == 2 && count > 0) {
                size_t str_pos = tiff_start + value_offset;
                if (str_pos + count <= exif.size()) {
                    make.assign(reinterpret_cast<const char*>(&exif[str_pos]), count);
                    size_t null_pos = make.find('\0');
                    if (null_pos != std::string::npos) make.resize(null_pos);
                }
            }
        } else if (tag == 0x0110) {
            if (format == 2 && count > 0) {
                size_t str_pos = tiff_start + value_offset;
                if (str_pos + count <= exif.size()) {
                    model.assign(reinterpret_cast<const char*>(&exif[str_pos]), count);
                    size_t null_pos = model.find('\0');
                    if (null_pos != std::string::npos) model.resize(null_pos);
                }
            }
        } else if (tag == 0x0112) {
            if (format == 3 && count == 1) {
                size_t val_pos = pos - 12 + 8;
                orientation = static_cast<int>(readU16(val_pos));
            }
        }
    }
}

} // namespace image
