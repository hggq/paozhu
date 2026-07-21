#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace qr {

enum class Ecc { L = 0, M = 1, Q = 2, H = 3 };

class qrcode {
public:
    int version = 0;
    int size = 0;        // 最终边长（= 模块数 * dotSize）
    int scale = 1;       // 黑点大小：每个模块放大成 scale*scale 个点
    std::vector<uint8_t> data;

    int width()  const { return size; }
    int height() const { return size; }

    uint8_t at(int x, int y) const;

    // dotSize = 黑点大小（每个模块的点数），默认 1 表示不放大
    void text(const std::string& s, Ecc ecl = Ecc::M, int dotSize = 1,
              int minVersion = 1, int maxVersion = 40);

private:
    Ecc ecc_ = Ecc::M;
    std::vector<uint8_t> isFunction_;

    bool module(int x, int y) const;
    static bool getBit(long x, int i);

    static uint8_t gfMul(uint8_t x, uint8_t y);
    static std::vector<uint8_t> rsDivisor(int degree);
    static std::vector<uint8_t> rsRemainder(const std::vector<uint8_t>& data,
                                            const std::vector<uint8_t>& divisor);

    static int eccPerBlock(Ecc e, int v);
    static int numEccBlocks(Ecc e, int v);
    static int numRawDataModules(int v);
    static int numDataCodewords(int v, Ecc e);

    std::vector<int> alignmentPositions() const;

    void encode(const std::vector<uint8_t>& bytes, Ecc ecl, int minVersion, int maxVersion);
    void expandDots();   // 按 scale 放大每个模块（黑点大小）
    std::vector<uint8_t> buildDataCodewords(const std::vector<uint8_t>& bytes, Ecc ecl);
    std::vector<uint8_t> addEccAndInterleave(const std::vector<uint8_t>& dataCw, Ecc ecl);

    void setFunc(int x, int y, bool dark);
    void drawFunctionPatterns();
    void drawFinder(int cx, int cy);
    void drawAlignment(int cx, int cy);
    void drawFormatBits(int mask);
    void drawVersion();
    void drawCodewords(const std::vector<uint8_t>& codewords);
    void applyMask(int mask);

    long penaltyScore();
    int countFinderLike(const std::array<int, 7>& h) const;
    int terminateAndCount(bool runColor, int runLen, std::array<int, 7>& hist);
    void addHistory(int runLen, std::array<int, 7>& hist);
};

}  // namespace qr
