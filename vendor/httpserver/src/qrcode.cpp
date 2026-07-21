#include "qrcode.h"
#include <algorithm>
#include <climits>

namespace qr {

uint8_t qrcode::at(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size)
        throw std::out_of_range("qr::qrcode::at() 坐标越界");
    return data[y * size + x];
}

void qrcode::text(const std::string& s, Ecc ecl, int dotSize, int minVersion, int maxVersion) {
    scale = std::max(1, dotSize);
    encode(std::vector<uint8_t>(s.begin(), s.end()), ecl, minVersion, maxVersion);
}

bool qrcode::module(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size)
        throw std::out_of_range("qr::qrcode::module() 坐标越界");
    return data[y * size + x] != 0;
}

bool qrcode::getBit(long x, int i) {
    return ((x >> i) & 1) != 0;
}

uint8_t qrcode::gfMul(uint8_t x, uint8_t y) {
    int z = 0;
    for (int i = 7; i >= 0; --i) {
        z = (z << 1) ^ ((z >> 7) * 0x11D);
        z ^= ((y >> i) & 1) * x;
    }
    return static_cast<uint8_t>(z);
}

std::vector<uint8_t> qrcode::rsDivisor(int degree) {
    std::vector<uint8_t> result(degree, 0);
    result[degree - 1] = 1;
    uint8_t root = 1;
    for (int i = 0; i < degree; ++i) {
        for (size_t j = 0; j < result.size(); ++j) {
            result[j] = gfMul(result[j], root);
            if (j + 1 < result.size())
                result[j] ^= result[j + 1];
        }
        root = gfMul(root, 0x02);
    }
    return result;
}

std::vector<uint8_t> qrcode::rsRemainder(const std::vector<uint8_t>& data,
                                        const std::vector<uint8_t>& divisor) {
    std::vector<uint8_t> result(divisor.size(), 0);
    for (uint8_t b : data) {
        uint8_t factor = b ^ result[0];
        result.erase(result.begin());
        result.push_back(0);
        for (size_t i = 0; i < result.size(); ++i)
            result[i] ^= gfMul(divisor[i], factor);
    }
    return result;
}

int qrcode::eccPerBlock(Ecc e, int v) {
    static const int8_t T[4][41] = {
        {-1, 7,10,15,20,26,18,20,24,30,18,20,24,26,30,22,24,28,30,28,28,28,28,30,30,26,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
        {-1,10,16,26,18,24,16,18,22,22,26,30,22,22,24,24,28,28,26,26,26,26,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28},
        {-1,13,22,18,26,18,24,18,22,20,24,28,26,24,20,30,24,28,28,26,30,28,30,30,30,30,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
        {-1,17,28,22,16,22,28,26,26,24,28,24,28,22,24,24,30,28,28,26,28,30,24,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
    };
    return T[static_cast<int>(e)][v];
}

int qrcode::numEccBlocks(Ecc e, int v) {
    static const int8_t T[4][41] = {
        {-1,1,1,1,1,1,2,2,2,2,4,4,4,4,4,6,6,6,6,7,8,8,9,9,10,12,12,12,13,14,15,16,17,18,19,19,20,21,22,24,25},
        {-1,1,1,1,2,2,4,4,4,5,5,5,8,9,9,10,10,11,13,14,16,17,17,18,20,21,23,25,26,28,29,31,33,35,37,38,40,43,45,47,49},
        {-1,1,1,2,2,4,4,6,6,8,8,8,10,12,16,12,17,16,18,21,20,23,23,25,27,29,34,34,35,38,40,43,45,48,51,53,56,59,62,65,68},
        {-1,1,1,2,4,4,4,5,6,8,8,11,11,16,16,18,16,19,21,25,25,25,34,30,32,35,37,40,42,45,48,51,54,57,60,63,66,70,74,77,81},
    };
    return T[static_cast<int>(e)][v];
}

int qrcode::numRawDataModules(int v) {
    int result = (16 * v + 128) * v + 64;
    if (v >= 2) {
        int numAlign = v / 7 + 2;
        result -= (25 * numAlign - 10) * numAlign - 55;
        if (v >= 7) result -= 36;
    }
    return result;
}

int qrcode::numDataCodewords(int v, Ecc e) {
    return numRawDataModules(v) / 8 - eccPerBlock(e, v) * numEccBlocks(e, v);
}

std::vector<int> qrcode::alignmentPositions() const {
    if (version == 1) return {};
    int numAlign = version / 7 + 2;
    int step = (version == 32) ? 26
             : (version * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
    std::vector<int> result;
    for (int i = 0, pos = size - 7; i < numAlign - 1; ++i, pos -= step)
        result.insert(result.begin(), pos);
    result.insert(result.begin(), 6);
    return result;
}

void qrcode::encode(const std::vector<uint8_t>& bytes, Ecc ecl, int minVersion, int maxVersion) {
    ecc_ = ecl;
    int usedBits = 0;
    for (version = std::max(1, minVersion); ; ++version) {
        if (version > maxVersion)
            throw std::length_error("数据太长，超出所选纠错等级的最大容量");
        int ccBits = (version <= 9) ? 8 : 16;
        int need = 4 + ccBits + 8 * static_cast<int>(bytes.size());
        int cap  = numDataCodewords(version, ecl) * 8;
        if (need <= cap) { usedBits = need; break; }
    }
    (void)usedBits;
    size = 21 + (version - 1) * 4;
    data.assign(size * size, 0);
    isFunction_.assign(size * size, 0);

    std::vector<uint8_t> dataCodewords = buildDataCodewords(bytes, ecl);
    std::vector<uint8_t> allCodewords = addEccAndInterleave(dataCodewords, ecl);

    drawFunctionPatterns();
    drawCodewords(allCodewords);

    int bestMask = 0;
    long minPenalty = LONG_MAX;
    for (int m = 0; m < 8; ++m) {
        applyMask(m);
        drawFormatBits(m);
        long p = penaltyScore();
        if (p < minPenalty) { minPenalty = p; bestMask = m; }
        applyMask(m);
    }
    applyMask(bestMask);
    drawFormatBits(bestMask);

    expandDots();   // 最后一步：按黑点大小放大
}

void qrcode::expandDots() {
    if (scale <= 1) return;
    int n = size;                       // 原模块数
    int big = n * scale;                // 放大后边长
    std::vector<uint8_t> out(static_cast<size_t>(big) * big, 0);
    for (int y = 0; y < n; ++y)
        for (int x = 0; x < n; ++x) {
            if (!data[y * n + x]) continue;
            for (int dy = 0; dy < scale; ++dy)
                for (int dx = 0; dx < scale; ++dx)
                    out[(y * scale + dy) * big + (x * scale + dx)] = 1;
        }
    data = std::move(out);
    size = big;                         // width()/height()/at() 自动对齐
}

std::vector<uint8_t> qrcode::buildDataCodewords(const std::vector<uint8_t>& bytes, Ecc ecl) {
    std::vector<bool> bits;
    auto appendBits = [&](uint32_t val, int n) {
        for (int i = n - 1; i >= 0; --i) bits.push_back(((val >> i) & 1) != 0);
    };
    appendBits(0b0100, 4);
    int ccBits = (version <= 9) ? 8 : 16;
    appendBits(static_cast<uint32_t>(bytes.size()), ccBits);
    for (uint8_t b : bytes) appendBits(b, 8);

    int capacityBits = numDataCodewords(version, ecl) * 8;
    appendBits(0, std::min(4, capacityBits - static_cast<int>(bits.size())));
    appendBits(0, (8 - static_cast<int>(bits.size()) % 8) % 8);
    for (uint8_t pad = 0xEC; static_cast<int>(bits.size()) < capacityBits; pad ^= 0xEC ^ 0x11)
        appendBits(pad, 8);

    std::vector<uint8_t> cw(bits.size() / 8, 0);
    for (size_t i = 0; i < bits.size(); ++i)
        if (bits[i]) cw[i >> 3] |= 1 << (7 - (i & 7));
    return cw;
}

std::vector<uint8_t> qrcode::addEccAndInterleave(const std::vector<uint8_t>& dataCw, Ecc ecl) {
    int numBlocks = numEccBlocks(ecl, version);
    int blockEccLen = eccPerBlock(ecl, version);
    int rawCw = numRawDataModules(version) / 8;
    int numShort = numBlocks - rawCw % numBlocks;
    int shortLen = rawCw / numBlocks;

    std::vector<std::vector<uint8_t>> blocks;
    std::vector<uint8_t> divisor = rsDivisor(blockEccLen);
    for (int i = 0, k = 0; i < numBlocks; ++i) {
        int datLen = shortLen - blockEccLen + (i < numShort ? 0 : 1);
        std::vector<uint8_t> dat(dataCw.begin() + k, dataCw.begin() + k + datLen);
        k += datLen;
        std::vector<uint8_t> block(shortLen + 1, 0);
        std::copy(dat.begin(), dat.end(), block.begin());
        std::vector<uint8_t> ecc = rsRemainder(dat, divisor);
        std::copy(ecc.begin(), ecc.end(), block.end() - blockEccLen);
        blocks.push_back(std::move(block));
    }

    std::vector<uint8_t> result;
    result.reserve(rawCw);
    for (size_t i = 0; i < blocks[0].size(); ++i)
        for (size_t j = 0; j < blocks.size(); ++j)
            if (i != static_cast<size_t>(shortLen - blockEccLen) || j >= static_cast<size_t>(numShort))
                result.push_back(blocks[j][i]);
    return result;
}

void qrcode::setFunc(int x, int y, bool dark) {
    data[y * size + x] = dark ? 1 : 0;
    isFunction_[y * size + x] = 1;
}

void qrcode::drawFunctionPatterns() {
    for (int i = 0; i < size; ++i) {
        setFunc(6, i, i % 2 == 0);
        setFunc(i, 6, i % 2 == 0);
    }
    drawFinder(3, 3);
    drawFinder(size - 4, 3);
    drawFinder(3, size - 4);
    auto pos = alignmentPositions();
    size_t n = pos.size();
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j) {
            if ((i == 0 && j == 0) || (i == 0 && j == n - 1) || (i == n - 1 && j == 0)) continue;
            drawAlignment(pos[i], pos[j]);
        }
    drawFormatBits(0);
    drawVersion();
}

void qrcode::drawFinder(int cx, int cy) {
    for (int dy = -4; dy <= 4; ++dy)
        for (int dx = -4; dx <= 4; ++dx) {
            int dist = std::max(std::abs(dx), std::abs(dy));
            int x = cx + dx, y = cy + dy;
            if (0 <= x && x < size && 0 <= y && y < size)
                setFunc(x, y, dist != 2 && dist != 4);
        }
}

void qrcode::drawAlignment(int cx, int cy) {
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 2; ++dx) {
            int x = cx + dx, y = cy + dy;
            if (0 <= x && x < size && 0 <= y && y < size)
                setFunc(x, y, std::max(std::abs(dx), std::abs(dy)) != 1);
        }
}

void qrcode::drawFormatBits(int mask) {
    static const int fmtBits[4] = {1, 0, 3, 2};
    int d = fmtBits[static_cast<int>(ecc_)] << 3 | mask;
    int rem = d;
    for (int i = 0; i < 10; ++i) rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    int bits = (d << 10 | rem) ^ 0x5412;

    for (int i = 0; i <= 5; ++i) setFunc(8, i, getBit(bits, i));
    setFunc(8, 7, getBit(bits, 6));
    setFunc(8, 8, getBit(bits, 7));
    setFunc(7, 8, getBit(bits, 8));
    for (int i = 9; i < 15; ++i) setFunc(14 - i, 8, getBit(bits, i));

    for (int i = 0; i < 8; ++i) setFunc(size - 1 - i, 8, getBit(bits, i));
    for (int i = 8; i < 15; ++i) setFunc(8, size - 15 + i, getBit(bits, i));
    setFunc(8, size - 8, true);
}

void qrcode::drawVersion() {
    if (version < 7) return;
    int rem = version;
    for (int i = 0; i < 12; ++i) rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
    long bits = static_cast<long>(version) << 12 | rem;
    for (int i = 0; i < 18; ++i) {
        bool bit = getBit(bits, i);
        int a = size - 11 + i % 3, b = i / 3;
        setFunc(a, b, bit);
        setFunc(b, a, bit);
    }
}

void qrcode::drawCodewords(const std::vector<uint8_t>& codewords) {
    size_t i = 0;
    for (int right = size - 1; right >= 1; right -= 2) {
        if (right == 6) right = 5;
        for (int vert = 0; vert < size; ++vert) {
            for (int j = 0; j < 2; ++j) {
                int x = right - j;
                bool upward = ((right + 1) & 2) == 0;
                int y = upward ? size - 1 - vert : vert;
                if (!isFunction_[y * size + x] && i < codewords.size() * 8) {
                    data[y * size + x] = getBit(codewords[i >> 3], 7 - static_cast<int>(i & 7)) ? 1 : 0;
                    ++i;
                }
            }
        }
    }
}

void qrcode::applyMask(int mask) {
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            if (isFunction_[y * size + x]) continue;
            bool inv = false;
            switch (mask) {
                case 0: inv = (x + y) % 2 == 0; break;
                case 1: inv = y % 2 == 0; break;
                case 2: inv = x % 3 == 0; break;
                case 3: inv = (x + y) % 3 == 0; break;
                case 4: inv = (x / 3 + y / 2) % 2 == 0; break;
                case 5: inv = x * y % 2 + x * y % 3 == 0; break;
                case 6: inv = (x * y % 2 + x * y % 3) % 2 == 0; break;
                case 7: inv = ((x + y) % 2 + x * y % 3) % 2 == 0; break;
            }
            if (inv) data[y * size + x] ^= 1;
        }
}

long qrcode::penaltyScore() {
    const int N1 = 3, N2 = 3, N3 = 40, N4 = 10;
    long result = 0;

    for (int y = 0; y < size; ++y) {
        bool runColor = false; int runLen = 0;
        std::array<int, 7> hist{};
        for (int x = 0; x < size; ++x) {
            if (module(x, y) == runColor) {
                if (++runLen == 5) result += N1;
                else if (runLen > 5) ++result;
            } else {
                addHistory(runLen, hist);
                if (!runColor) result += countFinderLike(hist) * N3;
                runColor = module(x, y); runLen = 1;
            }
        }
        result += terminateAndCount(runColor, runLen, hist) * N3;
    }
    for (int x = 0; x < size; ++x) {
        bool runColor = false; int runLen = 0;
        std::array<int, 7> hist{};
        for (int y = 0; y < size; ++y) {
            if (module(x, y) == runColor) {
                if (++runLen == 5) result += N1;
                else if (runLen > 5) ++result;
            } else {
                addHistory(runLen, hist);
                if (!runColor) result += countFinderLike(hist) * N3;
                runColor = module(x, y); runLen = 1;
            }
        }
        result += terminateAndCount(runColor, runLen, hist) * N3;
    }

    for (int y = 0; y < size - 1; ++y)
        for (int x = 0; x < size - 1; ++x) {
            bool c = module(x, y);
            if (c == module(x + 1, y) && c == module(x, y + 1) && c == module(x + 1, y + 1))
                result += N2;
        }

    int dark = 0;
    for (uint8_t v : data) dark += v;
    int total = size * size;
    int k = static_cast<int>((std::abs(dark * 20L - total * 10L) + total - 1) / total) - 1;
    result += static_cast<long>(k) * N4;
    return result;
}

int qrcode::countFinderLike(const std::array<int, 7>& h) const {
    int n = h[1];
    bool core = n > 0 && h[2] == n && h[3] == n * 3 && h[4] == n && h[5] == n;
    return (core && h[0] >= n * 4 && h[6] >= n ? 1 : 0)
         + (core && h[6] >= n * 4 && h[0] >= n ? 1 : 0);
}

int qrcode::terminateAndCount(bool runColor, int runLen, std::array<int, 7>& hist) {
    if (runColor) { addHistory(runLen, hist); runLen = 0; }
    runLen += size;
    addHistory(runLen, hist);
    return countFinderLike(hist);
}

void qrcode::addHistory(int runLen, std::array<int, 7>& hist) {
    if (hist[0] == 0) runLen += size;
    for (int i = 6; i >= 1; --i) hist[i] = hist[i - 1];
    hist[0] = runLen;
}

}  // namespace qr
