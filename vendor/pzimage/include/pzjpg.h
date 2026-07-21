#pragma once
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include "image.h"


namespace image {

// JPEG 扩展标记数据
struct JPEGMetadata {
    std::vector<uint8_t> exif;
    std::vector<uint8_t> xmp;
    std::vector<uint8_t> icc_profile;
    std::vector<uint8_t> app0;
    std::vector<uint8_t> other_apps;
    
    int orientation = 0;
    std::string make;
    std::string model;
    
    void parseExif();
};

// JPEG 图像类
class jpg {
public:
    int width = 0;
    int height = 0;
    std::vector<ColorRGB> pixels;
    JPEGMetadata metadata;

    // 读取 JPEG 文件
    bool read(const std::string& filename);

    // 保存为 JPEG 文件
    // quality: 10~100
    // progressive: 是否使用渐进式编码 (false=baseline)
    // subsample420: 是否使用 4:2:0 色度下采样 (省约一半体积, 照片推荐开启)
    // bits_per_sample: 8 或 12 (默认8)
    bool save(const std::string& filename, int quality = 85,
              bool progressive = false, bool subsample420 = false, int bits_per_sample = 8) const;
    // 返回 JPEG 编码数据 (不写文件)
    std::vector<uint8_t> imshow(int quality = 85,
              bool progressive = false, bool subsample420 = false, int bits_per_sample = 8) const;
    // 保存到文件并返回 JPEG 编码数据
    std::vector<uint8_t> saveAndShow(const std::string& filename, int quality = 85,
              bool progressive = false, bool subsample420 = false, int bits_per_sample = 8) const;
    // 创建指定尺寸的图像
    void create(int w, int h);

    bool qrdata(std::vector<uint8_t> &qr_data,int q_width,int q_height,unsigned char scale = 1, int pos_x = 0,int pos_y = 0);

    // ---------- 图像缩放 (Bicubic 双三次插值) ----------
    bool resize(int newWidth, int newHeight);
    bool resizeScale(double scale);

    // ---------- 像素访问 (与 PNG 保持一致) ----------
    void fillColor(const ColorRGB &c);
    void setPixel(uint32_t x, uint32_t y, const ColorRGB &c);
    ColorRGB getPixel(uint32_t x, uint32_t y) const;
    ColorRGB &at(uint32_t x, uint32_t y) { return pixels[(size_t)y * width + x]; }
    const ColorRGB &at(uint32_t x, uint32_t y) const { return pixels[(size_t)y * width + x]; }
    ColorRGB *rowPtr(uint32_t y) {
        if (pixels.empty() || (int)y >= height) return nullptr;
        return pixels.data() + (size_t)y * width;
    }
    const ColorRGB *rowPtr(uint32_t y) const {
        if (pixels.empty() || (int)y >= height) return nullptr;
        return pixels.data() + (size_t)y * width;
    }
    // 兼容旧接口 (set 带越界检查; 读取可用 getPixel 带越界检查)
    void set(int x, int y, const ColorRGB &c);

    // ---------- 矩形填充 ----------
    void setRect(int x1, int y1, int x2, int y2, const ColorRGB &c);

    // ---------- 绘图 ----------
    void drawSinglePixelLine(int x1, int y1, int x2, int y2, const ColorRGB &c);
    void drawLine(int x1, int y1, int x2, int y2, const ColorRGB &c, int thickness = 1);
    void drawLine(int x, int y, int length, const ColorRGB &c); // 水平线快捷方式

    // ---------- Xiaolin Wu 抗锯齿画线 ----------
    int ipart(float x) { return (int)std::floor(x); }
    float fpart(float x) { return x - std::floor(x); }
    float rfpart(float x) { return 1.0f - fpart(x); }
    void plotPixel(int w, int h, int x, int y, float brightness, const ColorRGB &fg);
    void drawSoftLine(int w, int h, float x0, float y0, float x1, float y1, const ColorRGB &c);
    void drawThickLine(int w, int h, float x0, float y0, float x1, float y1,
                       const ColorRGB &color, int thickness = 10);

    // ---------- 圆 / 扇形 / 圆弧 / 圆角矩形 ----------
    void drawCircle(int w, int h, int cx, int cy, int radius, const ColorRGB &color);       // 圆轮廓
    void fillCircle(int w, int h, int cx, int cy, int radius, const ColorRGB &color);       // 实心圆(抗锯齿)
    void fillCircleHard(int w, int h, int cx, int cy, int radius, const ColorRGB &color);   // 实心圆(硬边缘)
    void fillSector(int w, int h, int cx, int cy, int radius,
                    float start_angle, float end_angle, const ColorRGB &color);             // 实心扇形
    void drawFilledCircleWithBorder(int w, int h, int cx, int cy, int radius,
                                    const ColorRGB &fill_color, const ColorRGB &border_color); // 带边框实心圆
    void drawArc(int w, int h, int cx, int cy, int radius,
                 float start_angle, float end_angle, const ColorRGB &color, float step = 0.02f); // 圆弧
    void drawRoundedRectOutline(int x, int y, int w, int h, int r, const ColorRGB &color);  // 圆角矩形轮廓
    void fillRoundedRect(int x, int y, int w, int h, int r, const ColorRGB &color);         // 实心圆角矩形

    // ---------- 图像拷贝 / 融合 (支持 RGB/RGBA 源, 可与 png 互拷) ----------
    // 从 RGB 数据拷贝: 源区域[x1,y1]-[x2,y2] 贴到本图 (pos_x,pos_y), 透明度 A 默认255
    void copyFrom(const std::vector<ColorRGB> &from_data, int src_w, int src_h,
                  int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A = 255);
    // 从 RGBA 数据拷贝: 按源 alpha 融合背景, A 为全局透明度(默认255)
    void copy(const std::vector<ColorRGBA> &from_data, int src_w, int src_h,
              int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A = 255);
};

} // namespace image
