// pzpng.h
// png 解码/编码库（支持 RGBA、RGB、灰度、灰度+Alpha、索引色）
// 编译需链接 zlib：g++ -std=c++20 pzpng.cpp main.cpp -lz

#pragma once

#include <cstdint>
#include <cmath>
#include <fstream> // 必须包含，用于 std::ifstream / std::ofstream
#include <string>
#include <vector>
#include <sstream>
#include "image.h"

namespace image {

class png {
public:
  static constexpr uint32_t MAX_WIDTH = 32768;
  static constexpr uint32_t MAX_HEIGHT = 32768;
  static constexpr size_t MAX_PIXELS = 100ULL * 1024 * 1024;
  static constexpr size_t MAX_DECOMPRESSED_SIZE = 500ULL * 1024 * 1024;
  static constexpr uint32_t MAX_BLOCK_LENGTH = 100UL * 1024 * 1024;

  struct Info {
    uint32_t width = 0, height = 0;
    uint8_t bitdepth = 0, colortype = 0;
    uint8_t compression = 0, filter = 0, interlace = 0;
    std::vector<uint8_t> sbit;
    struct PLTEEntry {
      uint8_t r = 0, g = 0, b = 0;
    };
    std::vector<PLTEEntry> plte;
    std::vector<uint8_t> trns;
    struct {
      uint32_t x, y;
      uint8_t unit;
    } phys{};
    std::string create;
    struct {
      uint32_t wx, wy, rx, ry, gx, gy, bx, by;
    } chrm{};
  };

  Info info;
  // 像素数据: 扁平行主序存储, 索引 pixels[y * info.width + x]
  std::vector<ColorRGBA> pixels;

int ipart(float x) { return (int)std::floor(x); }
float fpart(float x) { return x - std::floor(x); }
float rfpart(float x) { return 1.0f - fpart(x); }
void plotPixel(int w, int h, int x, int y, float brightness, const ColorRGBA& fg);
void drawSoftLine(int w, int h, float x0, float y0, float x1, float y1, const ColorRGBA &c);
//Xiaolin Wu 画线算法 (灰度画板示例)
void drawThickLine(int w, int h,
                        float x0, float y0, float x1, float y1,
                        const ColorRGBA& c, int thickness = 10);

  // ---------- 圆 / 扇形 / 圆弧 / 圆角矩形 ----------
  void drawCircle(int w, int h, int cx, int cy, int radius, const ColorRGBA &c);       // 圆轮廓
  void fillCircle(int w, int h, int cx, int cy, int radius, const ColorRGBA &c);       // 实心圆(抗锯齿)
  void fillCircleHard(int w, int h, int cx, int cy, int radius, const ColorRGBA &c);   // 实心圆(硬边缘)
  void fillSector(int w, int h, int cx, int cy, int radius,
                  float start_angle, float end_angle, const ColorRGBA &c);             // 实心扇形
  void drawFilledCircleWithBorder(int w, int h, int cx, int cy, int radius,
                                  const ColorRGBA &fill_color, const ColorRGBA &border_color); // 带边框实心圆
  void drawArc(int w, int h, int cx, int cy, int radius,
               float start_angle, float end_angle, const ColorRGBA &c, float step = 0.02f); // 圆弧
  void drawRoundedRectOutline(int x, int y, int w, int h, int r, const ColorRGBA &c);  // 圆角矩形轮廓
  void fillRoundedRect(int x, int y, int w, int h, int r, const ColorRGBA &c);         // 实心圆角矩形

  // ---------- 图像拷贝 / 融合 (支持 RGB/RGBA 源, 可与 jpg 互拷) ----------
  // 从 RGB 数据拷贝: 源区域[x1,y1]-[x2,y2] 贴到本图 (pos_x,pos_y), 透明度 A 默认255
  void copyFrom(const std::vector<ColorRGB> &from_data, int width, int height,
                int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A = 255);
  // 从 RGBA 数据拷贝: 按源 alpha 融合背景, A 为全局透明度(默认255)
  void copy(const std::vector<ColorRGBA> &from_data, int width, int height,
            int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A = 255);

  png();
  explicit png(const std::string &filename);

  // ---------- 设置模式 ----------
  void setSpeedMode(bool speed); // true: 速度优先, false: 体积优先

  // ---------- 解码 ----------
  bool read(const std::string &filename);

  // ---------- 创建空白图像 ----------
  bool create(uint32_t width, uint32_t height, int colortype = 6,
              int bitdepth = 8);

  // 将二维码模块数据(qrcode库输出, 非零=暗模块)缩放绘制到本图
  bool qrdata(std::vector<uint8_t> &qr_data, int q_width, int q_height,
              unsigned char scale = 1, int pos_x = 0, int pos_y = 0);

  void fillColor(const ColorRGBA &c);
  void setPixel(uint32_t x, uint32_t y, const ColorRGBA &c);
  ColorRGBA getPixel(uint32_t x, uint32_t y) const;

  // ---------- 直接访问 (行主序扁平缓冲) ----------
  // 像素引用: 直接索引, 不做边界检查, 调用方需保证 x < width 且 y < height
  ColorRGBA &at(uint32_t x, uint32_t y)
  {
    return pixels[static_cast<size_t>(y) * info.width + x];
  }
  const ColorRGBA &at(uint32_t x, uint32_t y) const
  {
    return pixels[static_cast<size_t>(y) * info.width + x];
  }
  // 行首指针: y 越界或无像素数据时返回 nullptr
  ColorRGBA *rowPtr(uint32_t y)
  {
    if (pixels.empty() || y >= info.height)
      return nullptr;
    return pixels.data() + static_cast<size_t>(y) * info.width;
  }
  const ColorRGBA *rowPtr(uint32_t y) const
  {
    if (pixels.empty() || y >= info.height)
      return nullptr;
    return pixels.data() + static_cast<size_t>(y) * info.width;
  }

  // ---------- 矩形填充 ----------
  void setRect(int x1, int y1, int x2, int y2, const ColorRGBA &c);

  // ---------- 绘图 ----------
  void drawSinglePixelLine(int x1, int y1, int x2, int y2, const ColorRGBA &c);
  void drawLine(int x1, int y1, int x2, int y2, const ColorRGBA &c,
                int thickness = 1);
  void drawLine(int x, int y, int length, const ColorRGBA &c); // 水平线快捷方式

  // ---------- 图像缩放 (Bicubic 双三次插值) ----------
  // 缩放到指定尺寸，成功返回 true (保留 Alpha 通道)
  bool resize(uint32_t newWidth, uint32_t newHeight);
  // 按比例缩放 (scale=0.5 缩小一半, scale=2.0 放大两倍)
  bool resizeScale(double scale);

  // ---------- 保存 ----------
  bool save(const std::string &filename);
  std::string imshow();   // 返回 png 数据（不保存）
  std::string saveAndShow(const std::string& filename);  // 新增
  // ---------- 工具 ----------
  static std::string rgbHex(uint8_t v);

private:
  // ---------- 运行控制 ----------
  bool speedMode_;

  // ---------- 解码私有成员 ----------
  std::string filename_;
  std::ifstream file_;
  std::vector<uint8_t> data_, idat_accum_;
  size_t seek_;
  bool read_ok_;

  // 内存输出流（所有编码输出都写到这里）
  std::ostringstream oss_;

  struct Block {
    uint32_t length;
    std::string name;
    std::vector<uint8_t> data;
    uint32_t crc;
    bool checkcrc;
  };

  uint8_t readU8();
  uint16_t readU16BE();
  uint32_t readU32BE();
  std::string readBName();
  std::vector<uint8_t> readSize(uint32_t len);
  uint32_t readCRC();
  Block readBlock();

  void processIHDR(const Block &b);
  void processPLTE(const Block &b);
  void processPHYS(const Block &b);
  void processCHRM(const Block &b);

  bool inflateIDAT();
  void decode();

  // 行解码辅助
  uint8_t rRowHeader();
  uint8_t rBits();

  // 真彩色解码 (channels=3 -> RGB, channels=4 -> RGBA, 支持 8/16 位)
  void decodeTruecolor(uint32_t channels);
  // 灰度解码 (hasAlpha=false -> 灰度, true -> 灰度+Alpha, 支持 1/2/4/8/16 位)
  void decodeGrayscale(bool hasAlpha);
  void indexrow();

  // 反滤波公共逻辑 (filter 0-4)
  void unfilterRow(uint8_t filter, const uint8_t *raw,
                   const std::vector<uint8_t> &prevRow,
                   std::vector<uint8_t> &currRow, uint32_t rowBytes,
                   uint32_t bpp);
  // 索引色调色板 + tRNS 查找
  ColorRGBA paletteColor(uint32_t idx) const;

  // ---------- 编码（仅编码到 oss_，不写文件） ----------
  bool encode(int colortype, int bitdepth);

  // 以下写入函数直接使用成员 oss_
  void writeSignature();
  void writeU32BE(uint32_t v);
  void writeChunk(const std::string &type, const std::vector<uint8_t> &data);
  void writeIHDR(uint32_t w, uint32_t h, uint8_t bd, uint8_t ct);
  void writeIEND();

  bool encodeIDAT(const std::vector<std::vector<uint8_t>> &rows, int bpp,
                  uint32_t w, uint32_t h, int bd, int forceFilter = -1);

  void applyFilter(const std::vector<uint8_t> &row, int filterType, int bpp,
                   uint32_t width, uint32_t y,
                   const std::vector<std::vector<uint8_t>> &rows,
                   std::vector<uint8_t> &out);

  uint32_t computeFilterScore(const std::vector<uint8_t> &row, int filterType,
                              int bpp, uint32_t width, uint32_t y,
                              const std::vector<std::vector<uint8_t>> &rows,
                              std::vector<uint8_t> &scratch);

  uint8_t paethPredictor(uint8_t a, uint8_t b, uint8_t c);
  std::vector<uint8_t> compressZlib(const std::vector<uint8_t> &data);
};
} // namespace image
