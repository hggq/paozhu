// ttffont.h
// TrueType/OpenType 字体字形读取与绘制 (验证码用途)
// 解析逻辑参考 vendor/webpdf/src/ttfparser.cpp (offset table / cmap / hmtx / CFF INDEX-DICT),
// 并额外解析字形轮廓:
//   - TTF: loca/glyf 表, 二次贝塞尔曲线轮廓
//   - OTF: "CFF " 表, Type2 charstring 三次贝塞尔曲线轮廓 (含 CID-keyed 字体)
// 因 pzimage(ENABLE_IMAGE) 与 webpdf(ENABLE_PDF) 是相互独立的编译模块,
// 这里做成自包含实现, 不直接链接 pz::TTFParser。
//
// 提供两种画法:
//   drawTextOutline / drawCharOutline : 只描字形轮廓(贝塞尔曲线展平后抗锯齿描线),
//                                       不做填充, 计算量最小, 适合验证码;
//   drawText / drawChar               : 扫描线非零环绕填充(带抗锯齿), 实心文字。
// 支持 TrueType 轮廓(glyf/loca) 与 OpenType/CFF 轮廓(OTTO), 不支持 .ttc 合集。

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>
#include "image.h"
#include "pzpng.h"
#include "pzjpg.h"

namespace image {

// 字形轮廓点 (字体设计单位, 复合字形已应用分量变换)
// TTF: on/off 交替的二次贝塞尔控制点; CFF: 三次曲线已展平, 全部为 on-curve
struct GlyphPoint {
    float x = 0, y = 0;
    bool oncurve = true;
};

// 单个字形: 若干条闭合轮廓 + 水平度量
struct GlyphOutline {
    std::vector<std::vector<GlyphPoint>> contours;
    uint16_t advanceWidth = 0;
    int16_t lsb = 0;
    bool empty() const { return contours.empty(); }
};

class ttffont {
public:
    ttffont() = default;
    explicit ttffont(const std::string &file) { load(file); }

    // 读取整个字体文件并解析 head/maxp/hhea/hmtx/cmap 及轮廓表 (glyf/loca 或 CFF)
    // 失败时返回 false, 原因见 error
    bool load(const std::string &file);
    bool isLoaded() const { return loaded_; }

    // ---------- 简易接口 (验证码常用) ----------
    // 设置字体目录, setFont 按名字在该目录下查找
    void setFontPath(const std::string &dir) { font_path_ = dir; }
    // 按名字加载字体: 依次尝试 名字本身/名字.ttf/名字.otf/目录下模糊匹配,
    // 都找不到时回退加载系统默认字体; 全部失败返回 false
    bool setFont(const std::string &name);
    // 直接加载系统默认字体 (macOS/Linux/Windows 常见 .ttf 路径)
    bool loadSystemDefault();

    void setFontSize(float px) { font_size_ = px; }           // em 像素大小, 默认 24
    void setFontColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        color_ = {r, g, b, a};
    }
    void setFillMode(bool fill) { fill_mode_ = fill; }        // false = 只画轮廓
    void setRotate(float radians) { rotate_ = radians; }      // 每字符绕自身中心旋转

    // 把文字画到图片上, (x, y) 为文字左上角起始位置
    // 用法: bb.drawFont<image::png>(a, "文字", x, y); 或 bb.drawFont(a, "文字", x, y);
    template <typename IMG>
    void drawFont(IMG &img, const std::string &text, float x, float y) {
        static_assert(std::is_same_v<IMG, png> || std::is_same_v<IMG, jpg>,
                      "drawFont only supports image::png / image::jpg");
        if (!loaded_) return;
        // (x,y) 为左上角, 基线 = y + ascender 像素高度
        float baseline = y + (float)ascender * font_size_ / (float)unitsPerEm;
        if constexpr (std::is_same_v<IMG, png>) {
            if (fill_mode_) drawText(img, x, baseline, font_size_, text, color_, rotate_);
            else drawTextOutline(img, x, baseline, font_size_, text, color_, rotate_);
        } else {
            ColorRGB c{color_.r, color_.g, color_.b};
            if (fill_mode_) drawText(img, x, baseline, font_size_, text, c, rotate_);
            else drawTextOutline(img, x, baseline, font_size_, text, c, rotate_);
        }
    }

    // ---------- 字体度量 (字体设计单位) ----------
    uint16_t unitsPerEm = 1000;
    int16_t ascender = 0;   // hhea ascender
    int16_t descender = 0;  // hhea descender (负值)
    uint16_t numGlyphs = 0;
    std::string error;      // load 失败原因

    // unicode 码点取字形轮廓 (内部按 gid 缓存); 无对应字形返回 nullptr
    const GlyphOutline *getGlyph(uint32_t codepoint);

    // 文本像素宽度 (pixel_size 为 em 像素大小)
    float textWidth(const std::string &utf8, float pixel_size);

    // ---------- 只画轮廓 (不填充, 计算量小, 验证码推荐) ----------
    // (x, y) 为基线起点; rotate 为每个字符绕自身包围盒中心旋转的弧度
    void drawTextOutline(png &img, float x, float y, float pixel_size,
                         const std::string &utf8, const ColorRGBA &c, float rotate = 0.0f);
    void drawTextOutline(jpg &img, float x, float y, float pixel_size,
                         const std::string &utf8, const ColorRGB &c, float rotate = 0.0f);
    // 单字符版本, 返回步进宽度(像素), 方便验证码逐字符随机旋转/错位
    float drawCharOutline(png &img, float x, float y, float pixel_size,
                          uint32_t codepoint, const ColorRGBA &c, float rotate = 0.0f);
    float drawCharOutline(jpg &img, float x, float y, float pixel_size,
                          uint32_t codepoint, const ColorRGB &c, float rotate = 0.0f);

    // ---------- 填充画法 (扫描线非零环绕 + 抗锯齿) ----------
    void drawText(png &img, float x, float y, float pixel_size,
                  const std::string &utf8, const ColorRGBA &c, float rotate = 0.0f);
    void drawText(jpg &img, float x, float y, float pixel_size,
                  const std::string &utf8, const ColorRGB &c, float rotate = 0.0f);
    float drawChar(png &img, float x, float y, float pixel_size,
                   uint32_t codepoint, const ColorRGBA &c, float rotate = 0.0f);
    float drawChar(jpg &img, float x, float y, float pixel_size,
                   uint32_t codepoint, const ColorRGB &c, float rotate = 0.0f);

    // utf8 字符串转 unicode 码点序列
    static std::vector<uint32_t> utf8ToCodepoints(const std::string &s);

private:
    bool loaded_ = false;
    std::string font_path_;                                   // setFontPath 设置的字体目录
    float font_size_ = 24.0f;                                 // drawFont 使用的 em 像素
    ColorRGBA color_ = {0, 0, 0, 255};                        // drawFont 颜色, 默认黑
    bool fill_mode_ = true;                                   // drawFont 默认填充
    float rotate_ = 0.0f;                                     // drawFont 每字符旋转弧度
    std::string data_;                                        // 整个字体文件
    std::map<std::string, std::pair<uint32_t, uint32_t>> tables_; // tag -> (offset,length)
    std::map<uint32_t, uint16_t> chars_;                      // unicode -> gid
    std::vector<uint16_t> advance_;                           // gid -> advanceWidth
    std::vector<int16_t> lsb_;                                // gid -> lsb
    std::vector<uint32_t> loca_;                              // gid -> glyf 偏移 (numGlyphs+1 项, 仅 TTF)
    uint32_t glyf_off_ = 0, glyf_len_ = 0;
    // ---- CFF (OTF) 轮廓数据 ----
    bool is_cff_ = false;
    std::vector<std::string> cff_charstrings_;                // gid -> Type2 charstring
    std::vector<std::string> cff_gsubrs_;                     // 全局子程序
    std::vector<std::vector<std::string>> cff_lsubrs_;        // 每个 FD 一组局部子程序 (非 CID 只有组 0)
    std::vector<uint8_t> cff_fdselect_;                       // gid -> FD 索引 (CID 字体, 空表示全部用组 0)
    std::unordered_map<uint32_t, GlyphOutline> cache_;        // gid -> 轮廓缓存

    // 大端读取 (越界返回 0, 同 ttfparser.cpp 风格)
    uint16_t getUShort(size_t pos) const;
    uint32_t getULong(size_t pos) const;

    bool parseOffsetTable();
    bool parseCmap();
    void parseHmtx(uint16_t numberOfHMetrics);
    bool parseLoca(int16_t indexToLocFormat);
    // 解析 "CFF " 表 (Top DICT/CharStrings/Subrs/FDArray/FDSelect)
    bool parseCFF(size_t off, size_t len);

    // 解析 gid 字形 (递归处理复合字形, depth 限制嵌套深度)
    bool parseGlyf(uint16_t gid, GlyphOutline &out, int depth);
    // 解释 gid 的 Type2 charstring, 三次曲线展平为折线点
    bool parseCFFGlyph(uint16_t gid, GlyphOutline &out);
    const GlyphOutline *glyphByGid(uint16_t gid);
};

} // namespace image
