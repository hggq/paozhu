// ttffont.cpp
// TrueType/OpenType 字体字形读取与绘制实现 (验证码用途)
// 表解析部分 (offset table / cmap / hmtx / CFF INDEX-DICT) 移植自
// vendor/webpdf/src/ttfparser.cpp, 额外实现字形轮廓提取:
//   TTF: loca/glyf 二次贝塞尔轮廓; OTF: Type2 charstring 三次贝塞尔轮廓。
// 轮廓画法: 贝塞尔展平成短线段后用 Xiaolin Wu 抗锯齿描线, 不填充;
// 填充画法: 扫描线非零环绕规则 + 4 子行采样抗锯齿。

#include "ttffont.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <filesystem>

namespace image {

namespace {

// ---------- 内部像素级画布 (统一 png/jpg 的混合操作) ----------
struct PngCanvas {
    png *img;
    int w, h;
    ColorRGBA c;
    float ca; // 颜色自带 alpha (0~1)
    PngCanvas(png &p, const ColorRGBA &col)
        : img(&p), w((int)p.info.width), h((int)p.info.height), c(col), ca(col.a / 255.0f) {}
    void blend(int x, int y, float a) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        a *= ca;
        if (a <= 0.0f) return;
        if (a > 1.0f) a = 1.0f;
        float inv = 1.0f - a;
        ColorRGBA &bg = img->at((uint32_t)x, (uint32_t)y);
        bg.r = (uint8_t)(c.r * a + bg.r * inv + 0.5f);
        bg.g = (uint8_t)(c.g * a + bg.g * inv + 0.5f);
        bg.b = (uint8_t)(c.b * a + bg.b * inv + 0.5f);
        bg.a = 255;
    }
};

struct JpgCanvas {
    jpg *img;
    int w, h;
    ColorRGB c;
    JpgCanvas(jpg &j, const ColorRGB &col) : img(&j), w(j.width), h(j.height), c(col) {}
    void blend(int x, int y, float a) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        if (a <= 0.0f) return;
        if (a > 1.0f) a = 1.0f;
        float inv = 1.0f - a;
        ColorRGB &bg = img->at((uint32_t)x, (uint32_t)y);
        bg.r = (uint8_t)(c.r * a + bg.r * inv + 0.5f);
        bg.g = (uint8_t)(c.g * a + bg.g * inv + 0.5f);
        bg.b = (uint8_t)(c.b * a + bg.b * inv + 0.5f);
    }
};

struct PtF {
    float x, y;
};

// ---------- Xiaolin Wu 抗锯齿描线 (与 pzpng drawSoftLine 同算法, 走 canvas.blend) ----------
template <typename Canvas>
void strokeLine(Canvas &cv, float x0, float y0, float x1, float y1) {
    bool steep = std::fabs(y1 - y0) > std::fabs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = (dx == 0.0f) ? 0.0f : dy / dx;

    auto fpart = [](float v) { return v - std::floor(v); };
    auto rfpart = [&](float v) { return 1.0f - fpart(v); };
    auto plot = [&](int px, int py, float a) {
        if (steep) cv.blend(py, px, a);
        else cv.blend(px, py, a);
    };

    // 首端点
    float xend = std::round(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = (int)xend;
    int ypxl1 = (int)std::floor(yend);
    plot(xpxl1, ypxl1, rfpart(yend) * xgap);
    plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap);
    float intery = yend + gradient;

    // 尾端点
    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = (int)xend;
    int ypxl2 = (int)std::floor(yend);
    plot(xpxl2, ypxl2, rfpart(yend) * xgap);
    plot(xpxl2, ypxl2 + 1, fpart(yend) * xgap);

    for (int x = xpxl1 + 1; x < xpxl2; x++) {
        int yy = (int)std::floor(intery);
        plot(x, yy, rfpart(intery));
        plot(x, yy + 1, fpart(intery));
        intery += gradient;
    }
}

// ---------- 二次贝塞尔展平 (每约 3 像素一段, 上限 24 段) ----------
void quadTo(std::vector<PtF> &out, const PtF &p0, const PtF &ctrl, const PtF &p1) {
    float d = std::fabs(ctrl.x - p0.x) + std::fabs(ctrl.y - p0.y) +
              std::fabs(p1.x - ctrl.x) + std::fabs(p1.y - ctrl.y);
    int n = (int)(d / 3.0f) + 2;
    if (n > 24) n = 24;
    for (int i = 1; i <= n; i++) {
        float t = (float)i / (float)n;
        float mt = 1.0f - t;
        float a = mt * mt, b = 2.0f * mt * t, cc = t * t;
        out.push_back({a * p0.x + b * ctrl.x + cc * p1.x,
                       a * p0.y + b * ctrl.y + cc * p1.y});
    }
}

// 单条轮廓 (像素坐标, 保留 on/off 标志) 展平为闭合折线
// TrueType 规则: 相邻两个 off-curve 点之间隐含一个 on-curve 中点
void flattenContour(const std::vector<GlyphPoint> &pts, std::vector<PtF> &poly) {
    poly.clear();
    size_t n = pts.size();
    if (n < 2) return;

    // 展开成以 on-curve 点开头、隐含中点已插入的序列
    std::vector<GlyphPoint> exp;
    exp.reserve(n * 2);
    size_t first_on = n;
    for (size_t i = 0; i < n; i++) {
        if (pts[i].oncurve) {
            first_on = i;
            break;
        }
    }
    if (first_on == n) {
        // 全部 off-curve: 起点取首尾两点的中点
        GlyphPoint mid;
        mid.x = (pts[n - 1].x + pts[0].x) * 0.5f;
        mid.y = (pts[n - 1].y + pts[0].y) * 0.5f;
        mid.oncurve = true;
        exp.push_back(mid);
        first_on = 0;
        for (size_t k = 0; k < n; k++) exp.push_back(pts[k]);
    } else {
        for (size_t k = 0; k < n; k++) exp.push_back(pts[(first_on + k) % n]);
    }
    // 相邻 off-off 之间插入隐含 on-curve 中点
    std::vector<GlyphPoint> seq;
    seq.reserve(exp.size() * 2);
    for (size_t i = 0; i < exp.size(); i++) {
        seq.push_back(exp[i]);
        const GlyphPoint &a = exp[i];
        const GlyphPoint &b = exp[(i + 1) % exp.size()];
        if (!a.oncurve && !b.oncurve) {
            GlyphPoint mid;
            mid.x = (a.x + b.x) * 0.5f;
            mid.y = (a.y + b.y) * 0.5f;
            mid.oncurve = true;
            seq.push_back(mid);
        }
    }

    // 此时 seq 以 on 开头且不存在相邻 off-off, 按 on[-off]-on 走线
    PtF cur = {seq[0].x, seq[0].y};
    poly.push_back(cur);
    size_t m = seq.size();
    size_t i = 1;
    while (i <= m) {
        const GlyphPoint &p = seq[i % m];
        if (p.oncurve) {
            poly.push_back({p.x, p.y});
            cur = {p.x, p.y};
            i += 1;
        } else {
            const GlyphPoint &q = seq[(i + 1) % m]; // 必为 on-curve
            quadTo(poly, cur, {p.x, p.y}, {q.x, q.y});
            cur = {q.x, q.y};
            i += 2;
        }
        if (i > m) break;
        if (i == m) {
            // 已回到起点则闭合
            if (std::fabs(cur.x - poly[0].x) > 0.01f || std::fabs(cur.y - poly[0].y) > 0.01f)
                poly.push_back(poly[0]);
            break;
        }
    }
}

// 字形轮廓 -> 像素空间闭合折线集合 (含缩放/平移/绕包围盒中心旋转)
void glyphToPolys(const GlyphOutline &g, float ox, float oy, float scale, float rotate,
                  std::vector<std::vector<PtF>> &polys) {
    polys.clear();
    polys.reserve(g.contours.size());
    std::vector<GlyphPoint> tmp;
    for (const auto &contour : g.contours) {
        tmp.clear();
        tmp.reserve(contour.size());
        for (const auto &p : contour) {
            GlyphPoint t;
            t.x = ox + p.x * scale;
            t.y = oy - p.y * scale; // 字体 y 向上, 图像 y 向下
            t.oncurve = p.oncurve;
            tmp.push_back(t);
        }
        polys.emplace_back();
        flattenContour(tmp, polys.back());
        if (polys.back().size() < 3) polys.pop_back();
    }
    if (rotate != 0.0f && !polys.empty()) {
        // 绕字形包围盒中心旋转
        float minx = 1e30f, miny = 1e30f, maxx = -1e30f, maxy = -1e30f;
        for (auto &poly : polys)
            for (auto &p : poly) {
                minx = std::min(minx, p.x);
                miny = std::min(miny, p.y);
                maxx = std::max(maxx, p.x);
                maxy = std::max(maxy, p.y);
            }
        float cx = (minx + maxx) * 0.5f, cy = (miny + maxy) * 0.5f;
        float cs = std::cos(rotate), sn = std::sin(rotate);
        for (auto &poly : polys)
            for (auto &p : poly) {
                float dx = p.x - cx, dy = p.y - cy;
                p.x = cx + dx * cs - dy * sn;
                p.y = cy + dx * sn + dy * cs;
            }
    }
}

// ---------- 扫描线填充 (非零环绕, 每行 4 子行采样抗锯齿) ----------
struct Edge {
    float x0, y0, x1, y1;
};

void addSpan(std::vector<float> &cov, int xbase, int W, float xa, float xb, float w) {
    xa -= (float)xbase;
    xb -= (float)xbase;
    if (xb <= 0.0f || xa >= (float)W || xb <= xa) return;
    if (xa < 0.0f) xa = 0.0f;
    if (xb > (float)W) xb = (float)W;
    int ia = (int)xa;
    int ib = (int)xb;
    if (ia == ib) {
        cov[ia] += (xb - xa) * w;
        return;
    }
    cov[ia] += ((float)(ia + 1) - xa) * w;
    for (int i = ia + 1; i < ib; i++) cov[i] += w;
    if (ib < W) cov[ib] += (xb - (float)ib) * w;
}

template <typename Canvas>
void fillPolys(Canvas &cv, const std::vector<std::vector<PtF>> &polys) {
    std::vector<Edge> edges;
    float minx = 1e30f, miny = 1e30f, maxx = -1e30f, maxy = -1e30f;
    for (const auto &poly : polys) {
        for (size_t i = 0; i + 1 < poly.size(); i++) {
            edges.push_back({poly[i].x, poly[i].y, poly[i + 1].x, poly[i + 1].y});
            minx = std::min({minx, poly[i].x, poly[i + 1].x});
            maxx = std::max({maxx, poly[i].x, poly[i + 1].x});
            miny = std::min({miny, poly[i].y, poly[i + 1].y});
            maxy = std::max({maxy, poly[i].y, poly[i + 1].y});
        }
        // 保险闭合 (glyphToPolys 已闭合, 此处仅防御)
        if (poly.size() >= 2 &&
            (poly.back().x != poly.front().x || poly.back().y != poly.front().y))
            edges.push_back({poly.back().x, poly.back().y, poly.front().x, poly.front().y});
    }
    if (edges.empty()) return;

    int y0 = std::max(0, (int)std::floor(miny));
    int y1 = std::min(cv.h - 1, (int)std::ceil(maxy));
    int x0 = std::max(0, (int)std::floor(minx));
    int x1 = std::min(cv.w - 1, (int)std::ceil(maxx));
    if (x1 < x0 || y1 < y0) return;
    int W = x1 - x0 + 1;

    constexpr int NSUB = 4; // 每像素行子采样数
    constexpr float wsub = 1.0f / (float)NSUB;
    std::vector<float> cov((size_t)W);
    std::vector<std::pair<float, int>> xs; // (交点x, 环绕方向)
    xs.reserve(32);

    for (int y = y0; y <= y1; y++) {
        std::fill(cov.begin(), cov.end(), 0.0f);
        bool any = false;
        for (int s = 0; s < NSUB; s++) {
            float sy = (float)y + ((float)s + 0.5f) * wsub;
            xs.clear();
            for (const auto &e : edges) {
                if (e.y0 == e.y1) continue;
                float ymin = std::min(e.y0, e.y1);
                float ymax = std::max(e.y0, e.y1);
                if (sy < ymin || sy >= ymax) continue; // 半开区间避免顶点重复计数
                float t = (sy - e.y0) / (e.y1 - e.y0);
                xs.push_back({e.x0 + t * (e.x1 - e.x0), e.y1 > e.y0 ? 1 : -1});
            }
            if (xs.empty()) continue;
            std::sort(xs.begin(), xs.end(),
                      [](const std::pair<float, int> &a, const std::pair<float, int> &b) {
                          return a.first < b.first;
                      });
            int wind = 0;
            float spanx = 0.0f;
            for (const auto &cr : xs) {
                if (wind == 0) spanx = cr.first;
                int neww = wind + cr.second;
                if (wind != 0 && neww == 0) {
                    addSpan(cov, x0, W, spanx, cr.first, wsub);
                    any = true;
                }
                wind = neww;
            }
        }
        if (!any) continue;
        for (int i = 0; i < W; i++)
            if (cov[(size_t)i] > 0.004f) cv.blend(x0 + i, y, cov[(size_t)i]);
    }
}

template <typename Canvas>
void strokePolys(Canvas &cv, const std::vector<std::vector<PtF>> &polys) {
    for (const auto &poly : polys)
        for (size_t i = 0; i + 1 < poly.size(); i++)
            strokeLine(cv, poly[i].x, poly[i].y, poly[i + 1].x, poly[i + 1].y);
}

} // namespace

// ==================== 字体解析 ====================

uint16_t ttffont::getUShort(size_t pos) const {
    if (pos + 1 >= data_.size()) return 0;
    return (uint16_t)(((uint8_t)data_[pos] << 8) | (uint8_t)data_[pos + 1]);
}

uint32_t ttffont::getULong(size_t pos) const {
    if (pos + 3 >= data_.size()) return 0;
    return ((uint32_t)(uint8_t)data_[pos] << 24) | ((uint32_t)(uint8_t)data_[pos + 1] << 16) |
           ((uint32_t)(uint8_t)data_[pos + 2] << 8) | (uint8_t)data_[pos + 3];
}

bool ttffont::load(const std::string &file) {
    loaded_ = false;
    error.clear();
    tables_.clear();
    chars_.clear();
    advance_.clear();
    lsb_.clear();
    loca_.clear();
    cache_.clear();
    is_cff_ = false;
    cff_charstrings_.clear();
    cff_gsubrs_.clear();
    cff_lsubrs_.clear();
    cff_fdselect_.clear();

    std::ifstream f(file, std::ios::binary);
    if (!f.is_open()) {
        error = "Can't open file: " + file;
        return false;
    }
    f.seekg(0, std::ios::end);
    std::streamoff size = f.tellg();
    if (size <= 12 || size > 256 * 1024 * 1024) {
        error = "Invalid font file size";
        return false;
    }
    f.seekg(0, std::ios::beg);
    data_.resize((size_t)size);
    f.read(&data_[0], size);
    f.close();

    if (!parseOffsetTable()) return false;

    auto th = tables_.find("head");
    if (th == tables_.end()) {
        error = "No head table";
        return false;
    }
    size_t head = th->second.first;
    unitsPerEm = getUShort(head + 18);
    if (unitsPerEm == 0) unitsPerEm = 1000;
    int16_t indexToLocFormat = (int16_t)getUShort(head + 50);

    auto tm = tables_.find("maxp");
    if (tm == tables_.end()) {
        error = "No maxp table";
        return false;
    }
    numGlyphs = getUShort(tm->second.first + 4);

    auto ta = tables_.find("hhea");
    uint16_t numberOfHMetrics = 0;
    if (ta != tables_.end()) {
        ascender = (int16_t)getUShort(ta->second.first + 4);
        descender = (int16_t)getUShort(ta->second.first + 6);
        numberOfHMetrics = getUShort(ta->second.first + 34);
    }
    parseHmtx(numberOfHMetrics);

    if (!parseCmap()) return false;

    // 轮廓表: TTF 用 glyf/loca, OTF 用 "CFF "
    auto tg = tables_.find("glyf");
    auto tc = tables_.find("CFF ");
    if (tg != tables_.end() && tables_.count("loca")) {
        if (!parseLoca(indexToLocFormat)) return false;
        glyf_off_ = tg->second.first;
        glyf_len_ = tg->second.second;
    } else if (tc != tables_.end()) {
        if (!parseCFF(tc->second.first, tc->second.second)) return false;
    } else {
        error = "No glyf or CFF outline table";
        return false;
    }

    loaded_ = true;
    return true;
}

// ==================== 字体查找 ====================

namespace {
// 不区分大小写子串匹配
bool icontains(const std::string &hay, const std::string &needle) {
    if (needle.empty()) return true;
    std::string h = hay, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), [](unsigned char ch) { return std::tolower(ch); });
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char ch) { return std::tolower(ch); });
    return h.find(n) != std::string::npos;
}
} // namespace

bool ttffont::setFont(const std::string &name) {
    namespace fs = std::filesystem;
    std::error_code ec;

    // 1. 名字本身就是可用路径
    if (fs::is_regular_file(name, ec) && load(name)) return true;

    // 2. 字体目录下精确拼接: 名字 / 名字.ttf / 名字.otf 等
    if (!font_path_.empty()) {
        const char *exts[] = {"", ".ttf", ".otf", ".TTF", ".OTF"};
        for (const char *ext : exts) {
            fs::path p = fs::path(font_path_) / (name + ext);
            if (fs::is_regular_file(p, ec) && load(p.string())) return true;
        }
        // 3. 目录下模糊匹配 (文件名含 name 的 .ttf/.otf, 不区分大小写)
        for (const auto &entry : fs::directory_iterator(font_path_, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            std::string fn = entry.path().filename().string();
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char ch) { return std::tolower(ch); });
            if (ext != ".ttf" && ext != ".otf") continue;
            if (icontains(fn, name) && load(entry.path().string())) return true;
        }
    }

    // 4. 回退系统默认字体
    return loadSystemDefault();
}

bool ttffont::loadSystemDefault() {
    // 常见系统 TrueType 字体路径 (.ttc 合集不支持, 不列入)
    static const char *candidates[] = {
#if defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Verdana.ttf",
        "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
        "/Library/Fonts/Arial.ttf",
#elif defined(_WIN32)
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\verdana.ttf",
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\times.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#endif
        "docs/font/AlibabaSans-Regular.ttf", // 项目自带字体 (从工程根目录运行时)
    };
    for (const char *p : candidates) {
        if (load(p)) return true;
    }
    // 最后尝试字体目录下任意一个 .ttf/.otf
    if (!font_path_.empty()) {
        namespace fs = std::filesystem;
        std::error_code ec;
        for (const auto &entry : fs::directory_iterator(font_path_, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char ch) { return std::tolower(ch); });
            if ((ext == ".ttf" || ext == ".otf") && load(entry.path().string())) return true;
        }
    }
    if (error.empty()) error = "No usable system default font found";
    return false;
}

bool ttffont::parseOffsetTable() {
    if (data_.size() < 12) {
        error = "File too small";
        return false;
    }
    std::string version(data_.data(), 4);
    if (version != std::string("\x00\x01\x00\x00", 4) && version != "true" && version != "OTTO") {
        error = "Unrecognized file format";
        return false;
    }
    uint16_t numTables = getUShort(4);
    size_t p = 12;
    for (uint16_t i = 0; i < numTables; i++) {
        if (p + 16 > data_.size()) break;
        std::string tag(data_.data() + p, 4);
        uint32_t offset = getULong(p + 8);
        uint32_t length = getULong(p + 12);
        // 校验表范围, 避免损坏字体导致越界 (同 ttfparser.cpp _get_table 的防护)
        if (offset <= data_.size() && length <= data_.size() - offset)
            tables_[tag] = std::make_pair(offset, length);
        p += 16;
    }
    return true;
}

void ttffont::parseHmtx(uint16_t numberOfHMetrics) {
    advance_.assign(numGlyphs, 0);
    lsb_.assign(numGlyphs, 0);
    auto it = tables_.find("hmtx");
    if (it == tables_.end()) return;
    size_t base = it->second.first;
    size_t hsz = it->second.second;
    uint16_t last_aw = 0;
    size_t nHM = std::min((size_t)numberOfHMetrics, hsz / 4);
    for (size_t i = 0; i < nHM && i < (size_t)numGlyphs; i++) {
        last_aw = getUShort(base + i * 4);
        advance_[i] = last_aw;
        lsb_[i] = (int16_t)getUShort(base + i * 4 + 2);
    }
    size_t lsb_start = base + nHM * 4;
    for (size_t i = nHM; i < (size_t)numGlyphs; i++) {
        advance_[i] = last_aw;
        size_t off = lsb_start + (i - nHM) * 2;
        if (off + 1 < base + hsz) lsb_[i] = (int16_t)getUShort(off);
    }
}

bool ttffont::parseCmap() {
    auto it = tables_.find("cmap");
    if (it == tables_.end()) {
        error = "No cmap table";
        return false;
    }
    size_t cmap = it->second.first;
    uint16_t tableCount = getUShort(cmap + 2);
    uint32_t uni_off = 0;
    uint16_t fmt = 0;
    for (uint16_t i = 0; i < tableCount; i++) {
        size_t pos = cmap + 4 + (size_t)i * 8;
        uint16_t platformID = getUShort(pos);
        uint16_t encodingID = getUShort(pos + 2);
        uint32_t offset = getULong(pos + 4);
        uint16_t format = getUShort(cmap + offset);
        if ((platformID == 3 && encodingID == 1) || platformID == 0 ||
            (platformID == 3 && encodingID == 10)) {
            if (format == 12) {
                uni_off = offset;
                fmt = 12;
                break;
            } else if (format == 4 && uni_off == 0) {
                uni_off = offset;
                fmt = 4;
            }
        }
    }
    if (uni_off == 0) {
        error = "No Unicode cmap found";
        return false;
    }
    size_t sub = cmap + uni_off;
    if (fmt == 12) {
        uint32_t numGroups = getULong(sub + 12);
        size_t p = sub + 16;
        for (uint32_t i = 0; i < numGroups && p + 11 < data_.size(); i++) {
            uint32_t startChar = getULong(p);
            uint32_t endChar = getULong(p + 4);
            uint32_t startGid = getULong(p + 8);
            p += 12;
            for (uint32_t c = startChar; c <= endChar && startGid + (c - startChar) < 0xFFFF; c++) {
                uint16_t gid = (uint16_t)(startGid + (c - startChar));
                if (gid > 0 && gid < numGlyphs) chars_[c] = gid;
            }
        }
    } else {
        uint16_t segCount = getUShort(sub + 6) / 2;
        std::vector<uint16_t> endCount, startCount, idRangeOffset;
        std::vector<int16_t> idDelta;
        size_t p = sub + 14;
        for (uint16_t i = 0; i < segCount; i++, p += 2) endCount.push_back(getUShort(p));
        p += 2; // reservedPad
        for (uint16_t i = 0; i < segCount; i++, p += 2) startCount.push_back(getUShort(p));
        for (uint16_t i = 0; i < segCount; i++, p += 2) idDelta.push_back((int16_t)getUShort(p));
        size_t ro_base = p;
        for (uint16_t i = 0; i < segCount; i++, p += 2) idRangeOffset.push_back(getUShort(p));

        for (uint16_t i = 0; i < segCount; i++) {
            uint16_t c1 = startCount[i], c2 = endCount[i];
            int16_t d = idDelta[i];
            uint16_t ro = idRangeOffset[i];
            if (c1 == 0xFFFF) break;
            for (uint32_t c = c1; c <= c2 && c < 0xFFFF; c++) {
                uint16_t gid;
                if (ro > 0) {
                    size_t gi = ro_base + (size_t)i * 2 + ro + (c - c1) * 2;
                    gid = getUShort(gi);
                    if (gid != 0) {
                        int32_t g = (int32_t)gid + (int32_t)d;
                        if (g < 0) g += 65536;
                        gid = (uint16_t)(g & 0xFFFF);
                    }
                } else {
                    int32_t g = (int32_t)c + (int32_t)d;
                    if (g < 0) g += 65536;
                    gid = (uint16_t)(g & 0xFFFF);
                }
                if (gid > 0 && gid < numGlyphs) chars_[c] = gid;
            }
        }
    }
    return true;
}

bool ttffont::parseLoca(int16_t indexToLocFormat) {
    auto it = tables_.find("loca");
    if (it == tables_.end()) {
        error = "No loca table";
        return false;
    }
    size_t base = it->second.first;
    size_t len = it->second.second;
    size_t n = (size_t)numGlyphs + 1;
    loca_.resize(n);
    if (indexToLocFormat == 0) {
        if (len < n * 2) n = len / 2;
        for (size_t i = 0; i < n; i++) loca_[i] = (uint32_t)getUShort(base + i * 2) * 2;
    } else {
        if (len < n * 4) n = len / 4;
        for (size_t i = 0; i < n; i++) loca_[i] = getULong(base + i * 4);
    }
    for (size_t i = n; i < loca_.size(); i++) loca_[i] = (n > 0) ? loca_[n - 1] : 0;
    return true;
}

// ==================== CFF (OTF) 解析 ====================

namespace {
// ---- CFF 底层原语 (移植自 ttfparser.cpp) ----
struct CFFDictEntry {
    int op;
    std::vector<double> ops;
};

uint16_t cff_u16(const std::string &d, size_t p) {
    if (p + 1 >= d.size()) return 0;
    return (uint16_t)(((uint8_t)d[p] << 8) | (uint8_t)d[p + 1]);
}

// 解析 CFF DICT (Top DICT / Private DICT / Font DICT)
void cff_parse_dict(const std::string &d, std::vector<CFFDictEntry> &out) {
    size_t i = 0;
    std::vector<double> operands;
    while (i < d.size()) {
        uint8_t b0 = (uint8_t)d[i];
        if (b0 <= 21) {
            int op = b0;
            if (b0 == 12) {
                op = 1200 + (i + 1 < d.size() ? (uint8_t)d[i + 1] : 0);
                i += 2;
            } else {
                i += 1;
            }
            out.push_back({op, operands});
            operands.clear();
        } else if (b0 == 28) {
            if (i + 2 >= d.size()) break;
            operands.push_back((int16_t)(((uint8_t)d[i + 1] << 8) | (uint8_t)d[i + 2]));
            i += 3;
        } else if (b0 == 29) {
            if (i + 4 >= d.size()) break;
            operands.push_back((int32_t)(((uint8_t)d[i + 1] << 24) | ((uint8_t)d[i + 2] << 16) |
                                         ((uint8_t)d[i + 3] << 8) | (uint8_t)d[i + 4]));
            i += 5;
        } else if (b0 == 30) { // 实数, 轮廓解析用不到具体值, 跳过字节并补 0
            size_t j = i + 1;
            bool done = false;
            while (j < d.size() && !done) {
                uint8_t byte = (uint8_t)d[j++];
                if ((byte >> 4) == 0xF || (byte & 0xF) == 0xF) done = true;
            }
            operands.push_back(0);
            i = j;
        } else if (b0 >= 32 && b0 <= 246) {
            operands.push_back((int)b0 - 139);
            i += 1;
        } else if (b0 >= 247 && b0 <= 250) {
            if (i + 1 >= d.size()) break;
            operands.push_back((b0 - 247) * 256 + (uint8_t)d[i + 1] + 108);
            i += 2;
        } else if (b0 >= 251 && b0 <= 254) {
            if (i + 1 >= d.size()) break;
            operands.push_back(-((int)(b0 - 251)) * 256 - (uint8_t)d[i + 1] - 108);
            i += 2;
        } else {
            i += 1;
        }
    }
}

const CFFDictEntry *cff_find(const std::vector<CFFDictEntry> &d, int op) {
    for (auto &e : d)
        if (e.op == op) return &e;
    return nullptr;
}

// 读取 pos 处的 CFF INDEX, end_pos 指向 INDEX 后一字节
bool cff_read_index(const std::string &d, size_t pos, std::vector<std::string> &out, size_t &end_pos) {
    out.clear();
    if (pos + 2 > d.size()) { end_pos = pos; return false; }
    uint16_t count = cff_u16(d, pos);
    if (count == 0) { end_pos = pos + 2; return true; }
    if (pos + 3 > d.size()) { end_pos = pos; return false; }
    uint8_t off_size = (uint8_t)d[pos + 2];
    if (off_size < 1 || off_size > 4) { end_pos = pos; return false; }
    size_t base = pos + 3;
    std::vector<uint32_t> offs((size_t)count + 1);
    for (size_t i = 0; i <= count; i++) {
        uint32_t v = 0;
        for (int b = 0; b < off_size; b++) {
            size_t q = base + i * off_size + (size_t)b;
            v = (v << 8) | (q < d.size() ? (uint8_t)d[q] : 0);
        }
        offs[i] = v;
    }
    size_t data_base = base + ((size_t)count + 1) * off_size - 1;
    for (size_t i = 0; i < count; i++) {
        if (offs[i + 1] < offs[i]) { end_pos = pos; return false; }
        size_t s = data_base + offs[i];
        size_t len = offs[i + 1] - offs[i];
        if (s + len > d.size()) { end_pos = pos; return false; }
        out.push_back(d.substr(s, len));
    }
    end_pos = data_base + offs[count];
    return true;
}

int cff_bias(size_t n) { return n < 1240 ? 107 : (n < 33900 ? 1131 : 32768); }

// ---- Type2 charstring 解释器: 输出展平后的轮廓折线 ----
struct CSCtx {
    const std::vector<std::string> *gsubrs = nullptr;
    const std::vector<std::string> *lsubrs = nullptr;
    int gbias = 0, lbias = 0;
    float x = 0, y = 0;
    std::vector<double> st;   // 操作数栈
    int nstems = 0;
    bool width_done = false;  // 首个清栈运算符可能携带 width 操作数
    bool open = false;
    GlyphOutline *out = nullptr;
    float seg = 20.0f;        // 三次曲线展平段长 (字体单位)
    bool ok = true;

    void addPt(float px, float py) {
        GlyphPoint p;
        p.x = px;
        p.y = py;
        p.oncurve = true;
        out->contours.back().push_back(p);
    }
    void closeContour() {
        if (open && !out->contours.empty() && out->contours.back().size() < 3)
            out->contours.pop_back(); // 退化轮廓丢弃
        open = false;
    }
    void moveTo(float nx, float ny) {
        closeContour();
        out->contours.emplace_back();
        x = nx;
        y = ny;
        addPt(x, y);
        open = true;
    }
    void lineTo(float nx, float ny) {
        if (open) addPt(nx, ny);
        x = nx;
        y = ny;
    }
    void curveTo(float x1, float y1, float x2, float y2, float x3, float y3) {
        if (open) {
            float d = std::fabs(x1 - x) + std::fabs(y1 - y) + std::fabs(x2 - x1) +
                      std::fabs(y2 - y1) + std::fabs(x3 - x2) + std::fabs(y3 - y2);
            int n = (int)(d / seg) + 2;
            if (n > 32) n = 32;
            for (int i = 1; i <= n; i++) {
                float t = (float)i / (float)n, mt = 1.0f - t;
                float a = mt * mt * mt, b = 3 * mt * mt * t, c = 3 * mt * t * t, dd = t * t * t;
                addPt(a * x + b * x1 + c * x2 + dd * x3, a * y + b * y1 + c * y2 + dd * y3);
            }
        }
        x = x3;
        y = y3;
    }
    // 首个清栈运算符前剥离可选的 width 操作数
    void stripWidth(size_t expected) {
        if (!width_done && st.size() > expected) st.erase(st.begin());
        width_done = true;
    }
};

void cs_run(CSCtx &c, const std::string &cs, int depth) {
    if (!c.ok || depth > 30) {
        c.ok = false;
        return;
    }
    size_t i = 0, n = cs.size();
    auto &st = c.st;
    while (i < n) {
        uint8_t b = (uint8_t)cs[i];
        if (b >= 32 || b == 28) { // 操作数
            double v;
            if (b == 28) {
                if (i + 2 >= n) { c.ok = false; return; }
                v = (int16_t)(((uint8_t)cs[i + 1] << 8) | (uint8_t)cs[i + 2]);
                i += 3;
            } else if (b <= 246) {
                v = (int)b - 139;
                i += 1;
            } else if (b <= 250) {
                if (i + 1 >= n) { c.ok = false; return; }
                v = (b - 247) * 256 + (uint8_t)cs[i + 1] + 108;
                i += 2;
            } else if (b <= 254) {
                if (i + 1 >= n) { c.ok = false; return; }
                v = -((int)(b - 251)) * 256 - (uint8_t)cs[i + 1] - 108;
                i += 2;
            } else { // 255: 16.16 定点数
                if (i + 4 >= n) { c.ok = false; return; }
                int32_t iv = ((uint8_t)cs[i + 1] << 24) | ((uint8_t)cs[i + 2] << 16) |
                             ((uint8_t)cs[i + 3] << 8) | (uint8_t)cs[i + 4];
                v = iv / 65536.0;
                i += 5;
            }
            if (st.size() < 48) st.push_back(v);
            continue;
        }
        i += 1;
        switch (b) {
        case 1: case 3: case 18: case 23: // hstem/vstem/hstemhm/vstemhm
            if (st.size() & 1) c.stripWidth(st.size() - 1);
            c.width_done = true;
            c.nstems += (int)st.size() / 2;
            st.clear();
            break;
        case 19: case 20: { // hintmask/cntrmask (隐含 vstem + 掩码字节)
            if (st.size() & 1) c.stripWidth(st.size() - 1);
            c.width_done = true;
            c.nstems += (int)st.size() / 2;
            st.clear();
            i += (size_t)(c.nstems + 7) / 8;
            if (i > n) { c.ok = false; return; }
            break;
        }
        case 21: // rmoveto
            c.stripWidth(2);
            if (st.size() >= 2)
                c.moveTo(c.x + (float)st[st.size() - 2], c.y + (float)st.back());
            st.clear();
            break;
        case 22: // hmoveto
            c.stripWidth(1);
            if (!st.empty()) c.moveTo(c.x + (float)st.back(), c.y);
            st.clear();
            break;
        case 4: // vmoveto
            c.stripWidth(1);
            if (!st.empty()) c.moveTo(c.x, c.y + (float)st.back());
            st.clear();
            break;
        case 5: // rlineto
            for (size_t k = 0; k + 1 < st.size(); k += 2)
                c.lineTo(c.x + (float)st[k], c.y + (float)st[k + 1]);
            st.clear();
            break;
        case 6: case 7: { // hlineto / vlineto (水平/垂直交替)
            bool horiz = (b == 6);
            for (size_t k = 0; k < st.size(); k++) {
                if (horiz) c.lineTo(c.x + (float)st[k], c.y);
                else c.lineTo(c.x, c.y + (float)st[k]);
                horiz = !horiz;
            }
            st.clear();
            break;
        }
        case 8: // rrcurveto
            for (size_t k = 0; k + 5 < st.size(); k += 6) {
                float x1 = c.x + (float)st[k], y1 = c.y + (float)st[k + 1];
                float x2 = x1 + (float)st[k + 2], y2 = y1 + (float)st[k + 3];
                c.curveTo(x1, y1, x2, y2, x2 + (float)st[k + 4], y2 + (float)st[k + 5]);
            }
            st.clear();
            break;
        case 24: { // rcurveline: n 段曲线 + 1 段直线
            size_t k = 0;
            while (st.size() - k >= 8) {
                float x1 = c.x + (float)st[k], y1 = c.y + (float)st[k + 1];
                float x2 = x1 + (float)st[k + 2], y2 = y1 + (float)st[k + 3];
                c.curveTo(x1, y1, x2, y2, x2 + (float)st[k + 4], y2 + (float)st[k + 5]);
                k += 6;
            }
            if (st.size() - k >= 2)
                c.lineTo(c.x + (float)st[k], c.y + (float)st[k + 1]);
            st.clear();
            break;
        }
        case 25: { // rlinecurve: n 段直线 + 1 段曲线
            size_t k = 0;
            while (st.size() - k >= 8) {
                c.lineTo(c.x + (float)st[k], c.y + (float)st[k + 1]);
                k += 2;
            }
            if (st.size() - k >= 6) {
                float x1 = c.x + (float)st[k], y1 = c.y + (float)st[k + 1];
                float x2 = x1 + (float)st[k + 2], y2 = y1 + (float)st[k + 3];
                c.curveTo(x1, y1, x2, y2, x2 + (float)st[k + 4], y2 + (float)st[k + 5]);
            }
            st.clear();
            break;
        }
        case 26: { // vvcurveto: 可选 dx1 + 若干垂直趋势曲线
            size_t k = 0;
            float dx1 = 0;
            if (st.size() & 1) { dx1 = (float)st[0]; k = 1; }
            for (; k + 3 < st.size(); k += 4) {
                float x1 = c.x + dx1, y1 = c.y + (float)st[k];
                float x2 = x1 + (float)st[k + 1], y2 = y1 + (float)st[k + 2];
                c.curveTo(x1, y1, x2, y2, x2, y2 + (float)st[k + 3]);
                dx1 = 0;
            }
            st.clear();
            break;
        }
        case 27: { // hhcurveto: 可选 dy1 + 若干水平趋势曲线
            size_t k = 0;
            float dy1 = 0;
            if (st.size() & 1) { dy1 = (float)st[0]; k = 1; }
            for (; k + 3 < st.size(); k += 4) {
                float x1 = c.x + (float)st[k], y1 = c.y + dy1;
                float x2 = x1 + (float)st[k + 1], y2 = y1 + (float)st[k + 2];
                c.curveTo(x1, y1, x2, y2, x2 + (float)st[k + 3], y2);
                dy1 = 0;
            }
            st.clear();
            break;
        }
        case 30: case 31: { // vhcurveto / hvcurveto (水平垂直交替, 末尾可选斜分量)
            bool horiz = (b == 31);
            size_t k = 0;
            while (st.size() - k >= 4) {
                bool last = (st.size() - k == 5);
                float x1, y1, x2, y2, x3, y3;
                if (horiz) {
                    x1 = c.x + (float)st[k];
                    y1 = c.y;
                    x2 = x1 + (float)st[k + 1];
                    y2 = y1 + (float)st[k + 2];
                    y3 = y2 + (float)st[k + 3];
                    x3 = x2 + (last ? (float)st[k + 4] : 0.0f);
                } else {
                    x1 = c.x;
                    y1 = c.y + (float)st[k];
                    x2 = x1 + (float)st[k + 1];
                    y2 = y1 + (float)st[k + 2];
                    x3 = x2 + (float)st[k + 3];
                    y3 = y2 + (last ? (float)st[k + 4] : 0.0f);
                }
                c.curveTo(x1, y1, x2, y2, x3, y3);
                horiz = !horiz;
                k += 4;
            }
            st.clear();
            break;
        }
        case 10: case 29: { // callsubr / callgsubr
            if (st.empty()) { c.ok = false; return; }
            int idx = (int)st.back();
            st.pop_back();
            const std::vector<std::string> &subrs = (b == 10) ? *c.lsubrs : *c.gsubrs;
            idx += (b == 10) ? c.lbias : c.gbias;
            if (idx < 0 || idx >= (int)subrs.size()) { c.ok = false; return; }
            cs_run(c, subrs[(size_t)idx], depth + 1);
            if (!c.ok) return;
            break;
        }
        case 11: // return
            return;
        case 14: // endchar (带 4 参数的 seac 古体合字不支持, 忽略)
            c.width_done = true;
            c.closeContour();
            st.clear();
            return;
        case 12: { // 两字节运算符: flex 系列画曲线, 其余忽略
            if (i >= n) { c.ok = false; return; }
            uint8_t b2 = (uint8_t)cs[i];
            i += 1;
            if (b2 == 35 && st.size() >= 13) { // flex: 两段任意曲线
                float x1 = c.x + (float)st[0], y1 = c.y + (float)st[1];
                float x2 = x1 + (float)st[2], y2 = y1 + (float)st[3];
                float x3 = x2 + (float)st[4], y3 = y2 + (float)st[5];
                c.curveTo(x1, y1, x2, y2, x3, y3);
                float x4 = c.x + (float)st[6], y4 = c.y + (float)st[7];
                float x5 = x4 + (float)st[8], y5 = y4 + (float)st[9];
                c.curveTo(x4, y4, x5, y5, x5 + (float)st[10], y5 + (float)st[11]);
            } else if (b2 == 34 && st.size() >= 7) { // hflex: 水平对称 flex
                float y0 = c.y;
                float x1 = c.x + (float)st[0], y1 = c.y;
                float x2 = x1 + (float)st[1], y2 = y1 + (float)st[2];
                float x3 = x2 + (float)st[3], y3 = y2;
                c.curveTo(x1, y1, x2, y2, x3, y3);
                float x4 = c.x + (float)st[4], y4 = c.y;
                float x5 = x4 + (float)st[5], y5 = y0;
                c.curveTo(x4, y4, x5, y5, x5 + (float)st[6], y0);
            } else if (b2 == 36 && st.size() >= 9) { // hflex1
                float y0 = c.y;
                float x1 = c.x + (float)st[0], y1 = c.y + (float)st[1];
                float x2 = x1 + (float)st[2], y2 = y1 + (float)st[3];
                float x3 = x2 + (float)st[4], y3 = y2;
                c.curveTo(x1, y1, x2, y2, x3, y3);
                float x4 = c.x + (float)st[5], y4 = c.y;
                float x5 = x4 + (float)st[6], y5 = y4 + (float)st[7];
                c.curveTo(x4, y4, x5, y5, x5 + (float)st[8], y0);
            } else if (b2 == 37 && st.size() >= 11) { // flex1
                float sx = c.x, sy = c.y;
                float dx = (float)(st[0] + st[2] + st[4] + st[6] + st[8]);
                float dy = (float)(st[1] + st[3] + st[5] + st[7] + st[9]);
                float x1 = c.x + (float)st[0], y1 = c.y + (float)st[1];
                float x2 = x1 + (float)st[2], y2 = y1 + (float)st[3];
                float x3 = x2 + (float)st[4], y3 = y2 + (float)st[5];
                c.curveTo(x1, y1, x2, y2, x3, y3);
                float x4 = c.x + (float)st[6], y4 = c.y + (float)st[7];
                float x5 = x4 + (float)st[8], y5 = y4 + (float)st[9];
                float x6, y6;
                if (std::fabs(dx) > std::fabs(dy)) { x6 = x5 + (float)st[10]; y6 = sy; }
                else { x6 = sx; y6 = y5 + (float)st[10]; }
                c.curveTo(x4, y4, x5, y5, x6, y6);
            }
            st.clear();
            break;
        }
        default: // 其余运算符 (算术类等) 轮廓提取用不到, 清栈忽略
            st.clear();
            break;
        }
    }
}
} // namespace

bool ttffont::parseCFF(size_t off, size_t len) {
    if (off + 4 > data_.size() || len < 4) {
        error = "Bad CFF table";
        return false;
    }
    std::string cff = data_.substr(off, std::min(len, data_.size() - off));
    uint8_t hdrSize = (uint8_t)cff[2];
    size_t pos = hdrSize;
    std::vector<std::string> names, topdicts, strings;
    if (!cff_read_index(cff, pos, names, pos) ||
        !cff_read_index(cff, pos, topdicts, pos) || topdicts.empty() ||
        !cff_read_index(cff, pos, strings, pos) ||
        !cff_read_index(cff, pos, cff_gsubrs_, pos)) {
        error = "Bad CFF header INDEX";
        return false;
    }

    std::vector<CFFDictEntry> top;
    cff_parse_dict(topdicts[0], top);
    const CFFDictEntry *e_cs = cff_find(top, 17); // CharStrings
    if (!e_cs || e_cs->ops.empty()) {
        error = "CFF has no CharStrings";
        return false;
    }
    size_t endp;
    if (!cff_read_index(cff, (size_t)e_cs->ops[0], cff_charstrings_, endp)) {
        error = "Bad CFF CharStrings INDEX";
        return false;
    }

    // 读 Private DICT 里的局部子程序 (op 19 = Subrs, 偏移相对 Private 起点)
    auto read_private = [&](size_t psz, size_t poff, std::vector<std::string> &subrs) {
        subrs.clear();
        if (poff >= cff.size() || psz > cff.size() - poff) return;
        std::vector<CFFDictEntry> pd;
        cff_parse_dict(cff.substr(poff, psz), pd);
        const CFFDictEntry *es = cff_find(pd, 19);
        if (es && !es->ops.empty()) {
            size_t e2;
            cff_read_index(cff, poff + (size_t)es->ops[0], subrs, e2);
        }
    };

    const CFFDictEntry *e_fda = cff_find(top, 1236); // FDArray
    const CFFDictEntry *e_fds = cff_find(top, 1237); // FDSelect
    if (e_fda && e_fds && !e_fda->ops.empty() && !e_fds->ops.empty()) {
        // CID-keyed 字体: 每个 FD 一套局部子程序, FDSelect 映射 gid -> FD
        std::vector<std::string> fdicts;
        size_t e2;
        if (!cff_read_index(cff, (size_t)e_fda->ops[0], fdicts, e2)) {
            error = "Bad CFF FDArray";
            return false;
        }
        cff_lsubrs_.resize(fdicts.size());
        for (size_t i = 0; i < fdicts.size(); i++) {
            std::vector<CFFDictEntry> fd;
            cff_parse_dict(fdicts[i], fd);
            const CFFDictEntry *ep = cff_find(fd, 18); // Private [size offset]
            if (ep && ep->ops.size() >= 2)
                read_private((size_t)ep->ops[0], (size_t)ep->ops[1], cff_lsubrs_[i]);
        }
        size_t fs = (size_t)e_fds->ops[0];
        size_t nglyphs = cff_charstrings_.size();
        cff_fdselect_.assign(nglyphs, 0);
        if (fs < cff.size()) {
            uint8_t fmt = (uint8_t)cff[fs];
            if (fmt == 0) {
                for (size_t g = 0; g < nglyphs && fs + 1 + g < cff.size(); g++)
                    cff_fdselect_[g] = (uint8_t)cff[fs + 1 + g];
            } else if (fmt == 3) {
                uint16_t nR = cff_u16(cff, fs + 1);
                size_t p = fs + 3;
                for (uint16_t r = 0; r < nR && p + 4 < cff.size(); r++, p += 3) {
                    uint16_t first = cff_u16(cff, p);
                    uint8_t fd2 = (uint8_t)cff[p + 2];
                    uint16_t next = cff_u16(cff, p + 3); // 下一段首 gid 或末尾 sentinel
                    for (uint32_t g = first; g < next && g < nglyphs; g++)
                        cff_fdselect_[g] = fd2;
                }
            }
        }
    } else {
        // 非 CID: Top DICT 的 Private 直接给出局部子程序
        cff_lsubrs_.resize(1);
        const CFFDictEntry *ep = cff_find(top, 18);
        if (ep && ep->ops.size() >= 2)
            read_private((size_t)ep->ops[0], (size_t)ep->ops[1], cff_lsubrs_[0]);
    }
    is_cff_ = true;
    return true;
}

bool ttffont::parseCFFGlyph(uint16_t gid, GlyphOutline &out) {
    if ((size_t)gid >= cff_charstrings_.size()) return false;
    const std::string &cs = cff_charstrings_[gid];
    if (cs.empty()) return true; // 空字形
    size_t fd = 0;
    if (!cff_fdselect_.empty() && (size_t)gid < cff_fdselect_.size()) fd = cff_fdselect_[gid];
    if (fd >= cff_lsubrs_.size()) fd = 0;
    static const std::vector<std::string> no_subrs;
    CSCtx ctx;
    ctx.gsubrs = &cff_gsubrs_;
    ctx.lsubrs = cff_lsubrs_.empty() ? &no_subrs : &cff_lsubrs_[fd];
    ctx.gbias = cff_bias(cff_gsubrs_.size());
    ctx.lbias = cff_bias(ctx.lsubrs->size());
    ctx.out = &out;
    ctx.seg = (float)unitsPerEm / 50.0f; // 展平段长≈ 1/50 em
    cs_run(ctx, cs, 0);
    ctx.closeContour();
    if (!ctx.ok) return false;
    return true;
}

// 解析单个字形轮廓 (支持复合字形递归)
bool ttffont::parseGlyf(uint16_t gid, GlyphOutline &out, int depth) {
    if (depth > 5) return false; // 复合嵌套限制
    if (gid >= numGlyphs || (size_t)gid + 1 >= loca_.size()) return false;
    uint32_t start = loca_[gid], end = loca_[gid + 1];
    if (end <= start) return true; // 空字形 (如空格)
    if (end > glyf_len_ || end - start < 10) return true;
    size_t g = (size_t)glyf_off_ + start;

    int16_t numberOfContours = (int16_t)getUShort(g);
    if (numberOfContours >= 0) {
        // ---- 简单字形 ----
        int nc = numberOfContours;
        std::vector<uint16_t> endPts((size_t)nc);
        size_t p = g + 10;
        for (int i = 0; i < nc; i++, p += 2) endPts[(size_t)i] = getUShort(p);
        if (nc == 0) return true;
        size_t npts = (size_t)endPts[(size_t)nc - 1] + 1;
        if (npts > 10000) return false;
        uint16_t insLen = getUShort(p);
        p += 2 + insLen;

        // flags (带 REPEAT)
        std::vector<uint8_t> flags;
        flags.reserve(npts);
        while (flags.size() < npts && p < data_.size()) {
            uint8_t fl = (uint8_t)data_[p++];
            flags.push_back(fl);
            if (fl & 0x08) { // REPEAT_FLAG
                uint8_t rep = (p < data_.size()) ? (uint8_t)data_[p++] : 0;
                for (uint8_t r = 0; r < rep && flags.size() < npts; r++) flags.push_back(fl);
            }
        }
        if (flags.size() < npts) return false;

        // x 坐标增量
        std::vector<float> xs(npts), ys(npts);
        int32_t v = 0;
        for (size_t i = 0; i < npts; i++) {
            uint8_t fl = flags[i];
            if (fl & 0x02) { // X_SHORT
                uint8_t d = (p < data_.size()) ? (uint8_t)data_[p++] : 0;
                v += (fl & 0x10) ? (int32_t)d : -(int32_t)d;
            } else if (!(fl & 0x10)) {
                v += (int16_t)getUShort(p);
                p += 2;
            }
            xs[i] = (float)v;
        }
        // y 坐标增量
        v = 0;
        for (size_t i = 0; i < npts; i++) {
            uint8_t fl = flags[i];
            if (fl & 0x04) { // Y_SHORT
                uint8_t d = (p < data_.size()) ? (uint8_t)data_[p++] : 0;
                v += (fl & 0x20) ? (int32_t)d : -(int32_t)d;
            } else if (!(fl & 0x20)) {
                v += (int16_t)getUShort(p);
                p += 2;
            }
            ys[i] = (float)v;
        }

        size_t pt = 0;
        for (int cidx = 0; cidx < nc; cidx++) {
            size_t last = endPts[(size_t)cidx];
            if (last >= npts) break;
            std::vector<GlyphPoint> contour;
            contour.reserve(last - pt + 1);
            for (; pt <= last; pt++) {
                GlyphPoint gp;
                gp.x = xs[pt];
                gp.y = ys[pt];
                gp.oncurve = (flags[pt] & 0x01) != 0;
                contour.push_back(gp);
            }
            if (contour.size() >= 2) out.contours.push_back(std::move(contour));
        }
        return true;
    }

    // ---- 复合字形 ----
    size_t p = g + 10;
    for (int comp = 0; comp < 16; comp++) {
        uint16_t flags = getUShort(p);
        uint16_t glyphIndex = getUShort(p + 2);
        p += 4;
        float dx = 0, dy = 0;
        if (flags & 0x0001) { // ARG_1_AND_2_ARE_WORDS
            if (flags & 0x0002) { // ARGS_ARE_XY_VALUES
                dx = (float)(int16_t)getUShort(p);
                dy = (float)(int16_t)getUShort(p + 2);
            }
            p += 4;
        } else {
            if (flags & 0x0002) {
                dx = (float)(int8_t)(uint8_t)(p < data_.size() ? data_[p] : 0);
                dy = (float)(int8_t)(uint8_t)(p + 1 < data_.size() ? data_[p + 1] : 0);
            }
            p += 2;
        }
        // 变换矩阵 (F2Dot14)
        float a = 1, b = 0, c2 = 0, d2 = 1;
        if (flags & 0x0008) { // WE_HAVE_A_SCALE
            a = d2 = (float)(int16_t)getUShort(p) / 16384.0f;
            p += 2;
        } else if (flags & 0x0040) { // X_AND_Y_SCALE
            a = (float)(int16_t)getUShort(p) / 16384.0f;
            d2 = (float)(int16_t)getUShort(p + 2) / 16384.0f;
            p += 4;
        } else if (flags & 0x0080) { // TWO_BY_TWO
            a = (float)(int16_t)getUShort(p) / 16384.0f;
            b = (float)(int16_t)getUShort(p + 2) / 16384.0f;
            c2 = (float)(int16_t)getUShort(p + 4) / 16384.0f;
            d2 = (float)(int16_t)getUShort(p + 6) / 16384.0f;
            p += 8;
        }
        GlyphOutline child;
        if (glyphIndex != gid && parseGlyf(glyphIndex, child, depth + 1)) {
            for (auto &contour : child.contours) {
                for (auto &gp : contour) {
                    float nx = a * gp.x + c2 * gp.y + dx;
                    float ny = b * gp.x + d2 * gp.y + dy;
                    gp.x = nx;
                    gp.y = ny;
                }
                out.contours.push_back(std::move(contour));
            }
        }
        if (!(flags & 0x0020)) break; // MORE_COMPONENTS
    }
    return true;
}

const GlyphOutline *ttffont::glyphByGid(uint16_t gid) {
    auto it = cache_.find(gid);
    if (it != cache_.end()) return &it->second;
    GlyphOutline g;
    bool okp = is_cff_ ? parseCFFGlyph(gid, g) : parseGlyf(gid, g, 0);
    if (!okp) g.contours.clear();
    if (gid < advance_.size()) g.advanceWidth = advance_[gid];
    if (gid < lsb_.size()) g.lsb = lsb_[gid];
    return &cache_.emplace(gid, std::move(g)).first->second;
}

const GlyphOutline *ttffont::getGlyph(uint32_t codepoint) {
    if (!loaded_) return nullptr;
    auto it = chars_.find(codepoint);
    if (it == chars_.end()) return nullptr;
    return glyphByGid(it->second);
}

// ==================== 文本工具 ====================

std::vector<uint32_t> ttffont::utf8ToCodepoints(const std::string &s) {
    std::vector<uint32_t> out;
    size_t i = 0, n = s.size();
    while (i < n) {
        uint8_t c = (uint8_t)s[i];
        uint32_t cp = 0;
        size_t len = 1;
        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
            cp = ((uint32_t)(c & 0x1F) << 6) | ((uint8_t)s[i + 1] & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
            cp = ((uint32_t)(c & 0x0F) << 12) | (((uint8_t)s[i + 1] & 0x3F) << 6) |
                 ((uint8_t)s[i + 2] & 0x3F);
            len = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
            cp = ((uint32_t)(c & 0x07) << 18) | (((uint8_t)s[i + 1] & 0x3F) << 12) |
                 (((uint8_t)s[i + 2] & 0x3F) << 6) | ((uint8_t)s[i + 3] & 0x3F);
            len = 4;
        } else {
            cp = 0xFFFD;
        }
        out.push_back(cp);
        i += len;
    }
    return out;
}

float ttffont::textWidth(const std::string &utf8, float pixel_size) {
    if (!loaded_) return 0;
    float scale = pixel_size / (float)unitsPerEm;
    float w = 0;
    for (uint32_t cp : utf8ToCodepoints(utf8)) {
        auto it = chars_.find(cp);
        if (it != chars_.end() && it->second < advance_.size())
            w += (float)advance_[it->second] * scale;
        else
            w += pixel_size * 0.25f; // 未映射字符占 1/4 em
    }
    return w;
}

// ==================== 绘制 ====================

namespace {
// 单字符绘制公共流程: 取轮廓 -> 像素空间折线 -> 描边或填充, 返回步进宽度
template <typename Canvas>
float drawGlyphImpl(Canvas &cv, ttffont &font, const GlyphOutline *g, float x, float y,
                    float pixel_size, float rotate, bool fill) {
    if (!g) return pixel_size * 0.25f;
    float scale = pixel_size / (float)font.unitsPerEm;
    float adv = (float)g->advanceWidth * scale;
    if (g->empty()) return adv; // 空格等
    std::vector<std::vector<PtF>> polys;
    glyphToPolys(*g, x, y, scale, rotate, polys);
    if (fill)
        fillPolys(cv, polys);
    else
        strokePolys(cv, polys);
    return adv;
}

template <typename Canvas>
void drawTextImpl(Canvas &cv, ttffont &font, float x, float y, float pixel_size,
                  const std::string &utf8, float rotate, bool fill) {
    for (uint32_t cp : ttffont::utf8ToCodepoints(utf8)) {
        const GlyphOutline *g = font.getGlyph(cp);
        x += drawGlyphImpl(cv, font, g, x, y, pixel_size, rotate, fill);
    }
}
} // namespace

void ttffont::drawTextOutline(png &img, float x, float y, float pixel_size,
                              const std::string &utf8, const ColorRGBA &c, float rotate) {
    if (!loaded_) return;
    PngCanvas cv(img, c);
    drawTextImpl(cv, *this, x, y, pixel_size, utf8, rotate, false);
}

void ttffont::drawTextOutline(jpg &img, float x, float y, float pixel_size,
                              const std::string &utf8, const ColorRGB &c, float rotate) {
    if (!loaded_) return;
    JpgCanvas cv(img, c);
    drawTextImpl(cv, *this, x, y, pixel_size, utf8, rotate, false);
}

float ttffont::drawCharOutline(png &img, float x, float y, float pixel_size,
                               uint32_t codepoint, const ColorRGBA &c, float rotate) {
    if (!loaded_) return 0;
    PngCanvas cv(img, c);
    return drawGlyphImpl(cv, *this, getGlyph(codepoint), x, y, pixel_size, rotate, false);
}

float ttffont::drawCharOutline(jpg &img, float x, float y, float pixel_size,
                               uint32_t codepoint, const ColorRGB &c, float rotate) {
    if (!loaded_) return 0;
    JpgCanvas cv(img, c);
    return drawGlyphImpl(cv, *this, getGlyph(codepoint), x, y, pixel_size, rotate, false);
}

void ttffont::drawText(png &img, float x, float y, float pixel_size,
                       const std::string &utf8, const ColorRGBA &c, float rotate) {
    if (!loaded_) return;
    PngCanvas cv(img, c);
    drawTextImpl(cv, *this, x, y, pixel_size, utf8, rotate, true);
}

void ttffont::drawText(jpg &img, float x, float y, float pixel_size,
                       const std::string &utf8, const ColorRGB &c, float rotate) {
    if (!loaded_) return;
    JpgCanvas cv(img, c);
    drawTextImpl(cv, *this, x, y, pixel_size, utf8, rotate, true);
}

float ttffont::drawChar(png &img, float x, float y, float pixel_size,
                        uint32_t codepoint, const ColorRGBA &c, float rotate) {
    if (!loaded_) return 0;
    PngCanvas cv(img, c);
    return drawGlyphImpl(cv, *this, getGlyph(codepoint), x, y, pixel_size, rotate, true);
}

float ttffont::drawChar(jpg &img, float x, float y, float pixel_size,
                        uint32_t codepoint, const ColorRGB &c, float rotate) {
    if (!loaded_) return 0;
    JpgCanvas cv(img, c);
    return drawGlyphImpl(cv, *this, getGlyph(codepoint), x, y, pixel_size, rotate, true);
}

} // namespace image
