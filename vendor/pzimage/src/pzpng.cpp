// pzpng.cpp
#include "pzpng.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <zlib.h>

namespace image
{
// ---------- 构造/析构 ----------
png::png() : speedMode_(false), seek_(0), read_ok_(false) {}
png::png(const std::string &filename) : speedMode_(false), seek_(0), read_ok_(false)
{
    read(filename);
}

// ---------- 模式设置 ----------
void png::setSpeedMode(bool speed) { speedMode_ = speed; }

// 画圆轮廓（抗锯齿版本，利用你的 plotPixel）
void png::drawCircle(int w, int h, int cx, int cy, int radius, const ColorRGBA& c) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // 利用八分对称性画 8 个对称点
        plotPixel(w, h, cx + x, cy + y, 1.0f, c);
        plotPixel(w, h, cx + y, cy + x, 1.0f, c);
        plotPixel(w, h, cx - y, cy + x, 1.0f, c);
        plotPixel(w, h, cx - x, cy + y, 1.0f, c);
        plotPixel(w, h, cx - x, cy - y, 1.0f, c);
        plotPixel(w, h, cx - y, cy - x, 1.0f, c);
        plotPixel(w, h, cx + y, cy - x, 1.0f, c);
        plotPixel(w, h, cx + x, cy - y, 1.0f, c);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

// 画实心圆（利用你的 drawLine 或直接水平填充）
void png::fillCircle(int w, int h, int cx, int cy, int radius, const ColorRGBA& c) {
    for (int y = -radius; y <= radius; ++y) {
        int half_width = (int)std::sqrt(radius * radius - y * y);
        int x1 = cx - half_width;
        int x2 = cx + half_width;
        // 直接画一条水平实线（利用你现有的画线函数，或者纯循环）
        // 方法A：如果你有 drawLine，直接画一条不透明的粗线（但那是标准线，比较慢）
        // 方法B：直接暴力填充像素（速度最快，因为全是整数操作）
        for (int x = x1; x <= x2; ++x) {
            // 直接调用你带颜色的 plotPixel，亮度设为 1.0（完全不透明）
            plotPixel(w, h, x, cy + y, 1.0f, c);
        }
    }
}

// 画实心扇形（角度范围 0~2PI，0度指正右方，顺时针）
void png::fillSector(int w, int h, int cx, int cy, int radius, 
                     float start_angle, float end_angle, const ColorRGBA& c) {
    // 归一化角度
    while (start_angle < 0) start_angle += 2 * M_PI;
    while (end_angle < 0) end_angle += 2 * M_PI;
    if (end_angle < start_angle) end_angle += 2 * M_PI;
    float range = end_angle - start_angle;

    for (int y = -radius; y <= radius; ++y) {
        int half_width = (int)std::sqrt(radius * radius - y * y);
        for (int x = -half_width; x <= half_width; ++x) {
            // 判断当前点是否在扇形角度范围内
            float angle = std::atan2((float)y, (float)x); // -PI ~ PI
            if (angle < 0) angle += 2 * M_PI; // 转为 0~2PI
            
            // 判断角度是否落在 [start_angle, end_angle] 内
            bool inside_angle = false;
            if (range < 2 * M_PI) {
                inside_angle = (angle >= start_angle && angle <= end_angle);
            } else {
                inside_angle = true; // 如果范围超过360度，就是整圆
            }

            if (inside_angle) {
                plotPixel(w, h, cx + x, cy + y, 1.0f, c);
            }
        }
    }
}

// 画带边框的实心圆（先填色，再描边）
void png::drawFilledCircleWithBorder(int w, int h, int cx, int cy, int radius,
                                     const ColorRGBA& fill_color,
                                     const ColorRGBA& border_color) {
    // 1. 先画实心填充（无边框）
    fillCircle(w, h, cx, cy, radius, fill_color);
    
    // 2. 再画空心轮廓（边框），覆盖在边缘上
    // 注意：如果想边框完全压在填充上，半径保持不变即可
    drawCircle(w, h, cx, cy, radius, border_color);
}

// 硬边缘实心圆（不抗锯齿，无过渡带）
void png::fillCircleHard(int w, int h, int cx, int cy, int radius, const ColorRGBA& c) {
    for (int y = -radius; y <= radius; ++y) {
        int py = cy + y;
        if (py < 0 || py >= h) continue;
        int half_width = (int)std::sqrt(radius * radius - y * y);
        int x1 = std::max(cx - half_width, 0);
        int x2 = std::min(cx + half_width, w - 1);
        for (int x = x1; x <= x2; ++x) {
            pixels[(size_t)py * w + x] = c; // 直接写成员像素缓冲 (参数改名为 c 避免遮蔽)
        }
    }
}

void png::drawArc(int w, int h, int cx, int cy, int radius, 
                  float start_angle, float end_angle, 
                  const ColorRGBA& c, float step) {
    if (radius <= 0) return;
    // 归一化角度范围
    if (end_angle < start_angle) end_angle += 2.0f * M_PI;
    float range = end_angle - start_angle;
    if (range <= 0) return;
    
    // 步长自适应（保证至少画到每个像素）
    int num_steps = std::max(2, (int)(range / step));
    for (int i = 0; i <= num_steps; ++i) {
        float t = start_angle + range * (i / (float)num_steps);
        int px = cx + (int)(radius * std::cos(t) + 0.5f);
        int py = cy + (int)(radius * std::sin(t) + 0.5f);
        plotPixel(w, h, px, py, 1.0f, c); // 亮度1.0表示完全不透明
    }
}

void png::drawRoundedRectOutline(int x, int y, int w, int h, int r, const ColorRGBA& c) {
    int W = (int)info.width, H = (int)info.height; // 图像尺寸 (区别于矩形 w,h)
    if (w <= 0 || h <= 0 || r <= 0) {
        // 退化情况，直接画普通矩形
        drawSoftLine(W, H, x, y, x + w, y, c);
        drawSoftLine(W, H, x + w, y, x + w, y + h, c);
        drawSoftLine(W, H, x + w, y + h, x, y + h, c);
        drawSoftLine(W, H, x, y + h, x, y, c);
        return;
    }
    // 限制圆角半径不超过半宽/半高
    r = std::min(r, std::min(w / 2, h / 2));

    // 四条直线（避开圆角区域）
    drawSoftLine(W, H, x + r, y, x + w - r, y, c);               // 上边
    drawSoftLine(W, H, x + w, y + r, x + w, y + h - r, c);       // 右边
    drawSoftLine(W, H, x + r, y + h, x + w - r, y + h, c);       // 下边
    drawSoftLine(W, H, x, y + r, x, y + h - r, c);               // 左边

    // 四个 1/4 圆弧（角度定义：0°=右，顺时针）
    drawArc(W, H, x + r, y + r, r, (float)M_PI, 1.5f * (float)M_PI, c);       // 左上角
    drawArc(W, H, x + w - r, y + r, r, 1.5f * (float)M_PI, 2.0f * (float)M_PI, c); // 右上角
    drawArc(W, H, x + w - r, y + h - r, r, 0.0f, 0.5f * (float)M_PI, c);     // 右下角
    drawArc(W, H, x + r, y + h - r, r, 0.5f * (float)M_PI, (float)M_PI, c);   // 左下角
}

void png::fillRoundedRect(int x, int y, int w, int h, int r, const ColorRGBA& c) {
    int W = (int)info.width, H = (int)info.height; // 图像尺寸 (区别于矩形 w,h)
    if (w <= 0 || h <= 0 || r <= 0) {
        // 退化：填充普通矩形（可复用你已有的填充矩形函数，或直接循环）
        for (int row = y; row < y + h; ++row) {
            for (int col = x; col < x + w; ++col) {
                plotPixel(W, H, col, row, 1.0f, c);
            }
        }
        return;
    }
    r = std::min(r, std::min(w / 2, h / 2));

    // 对每一行
    for (int row = y; row < y + h; ++row) {
        int dy_top = row - y;           // 距顶部距离
        int dy_bot = y + h - 1 - row;   // 距底部距离
        int left_bound = x;
        int right_bound = x + w - 1;

        // 处理顶部圆角（row 在 y ~ y+r-1）
        if (dy_top < r) {
            // 计算该行的左圆弧边界（左上角）
            int arc_x = (int)(std::sqrt(r * r - (r - dy_top) * (r - dy_top)) + 0.5f);
            left_bound = x + r - arc_x;
            // 右圆弧边界（右上角）对称
            right_bound = x + w - 1 - (r - arc_x);
        }
        // 处理底部圆角（row 在 y+h-r ~ y+h-1）
        else if (dy_bot < r) {
            int arc_x = (int)(std::sqrt(r * r - (r - dy_bot) * (r - dy_bot)) + 0.5f);
            left_bound = x + r - arc_x;
            right_bound = x + w - 1 - (r - arc_x);
        }
        // 中间区域无需调整

        // 填充该行像素（从 left_bound 到 right_bound）
        for (int col = left_bound; col <= right_bound; ++col) {
            // 对于边界像素（col == left_bound 或 col == right_bound），可做抗锯齿
            // 更精确的方法是计算该像素到实际边界的距离，但这里简化为边界像素亮度0.8~1.0
            // 由于我们用了 r 是整数，边界像素刚好在圆弧上，可直接用 plotPixel 的亮度=1.0
            // 但为了平滑，我们检查是否在圆弧边界附近（距离<1像素）
            // 这里为了简单，直接 plotPixel(..., 1.0) 填充，轮廓再由 drawRoundedRectOutline 覆盖
            // 这样会使轮廓更平滑，但填充内部是实的。
            // 推荐：先填充（硬），再用轮廓描边，这样轮廓抗锯齿，填充实心，视觉上没问题。
            plotPixel(W, H, col, row, 1.0f, c);
        }
    }
}

// 画带抗锯齿的点 (设置像素的亮度)
void png::plotPixel(int w, int h, int x, int y, float brightness, const ColorRGBA& fg) {
    if (x < 0 || x >= w || y < 0 || y >= h) return;
    float alpha = std::max(0.0f, std::min(1.0f, brightness));
    if (alpha <= 0.0f) return;
    float inv = 1.0f - alpha;
    // pixels 是 vector<ColorRGBA>, 按像素索引 (假定调用者传入的 w == info.width)
    ColorRGBA& bg = pixels[(size_t)y * w + x];
    bg.r = (uint8_t)(fg.r * alpha + bg.r * inv + 0.5f);
    bg.g = (uint8_t)(fg.g * alpha + bg.g * inv + 0.5f);
    bg.b = (uint8_t)(fg.b * alpha + bg.b * inv + 0.5f);
    bg.a = 255;
}

// Xiaolin Wu 画线算法 (灰度画板示例)
void png::drawSoftLine(int w, int h, float x0, float y0, float x1, float y1, const ColorRGBA &c) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
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
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

    int xpxl1 = (int)x0;
    int ypxl1 = (int)y0;
    // 修改点：传递颜色 c
    plotPixel(w, h, steep ? ypxl1 : xpxl1, steep ? xpxl1 : ypxl1, rfpart(y0), c);
    plotPixel(w, h, steep ? ypxl1 : xpxl1, steep ? xpxl1 : (ypxl1 + 1), fpart(y0), c);

    for (int x = xpxl1 + 1; x < (int)x1; x++) {
        float y = y0 + gradient * (x - x0);
        int ypxl = (int)y;
        plotPixel(w, h, steep ? ypxl : x, steep ? x : ypxl, rfpart(y), c);
        plotPixel(w, h, steep ? (ypxl + 1) : x, steep ? x : (ypxl + 1), fpart(y), c);
    }
}

void png::drawThickLine(int w, int h,
                        float x0, float y0, float x1, float y1,
                        const ColorRGBA& c, int thickness) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 0.0001f) return;
    float nx = -dy / len;
    float ny =  dx / len;
    int half = thickness / 2;
    for (int i = -half; i <= half; ++i) {
        drawSoftLine(w, h,
                     x0 + nx * i, y0 + ny * i,
                     x1 + nx * i, y1 + ny * i,
                     c);
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

void png::copyFrom(const std::vector<ColorRGB> &from_data, int width, int height,
                   int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A) {
    if (from_data.empty() || width <= 0 || height <= 0) return;
    if (pixels.empty() || info.width == 0 || info.height == 0) return;
    int sx0, sy0, dx0, dy0, cw, ch;
    if (!calcCopyRange(width, height, x1, y1, x2, y2, pos_x, pos_y,
                       (int)info.width, (int)info.height, sx0, sy0, dx0, dy0, cw, ch)) return;
    int ea = A, inv = 255 - ea;
    for (int j = 0; j < ch; j++) {
        int sy = sy0 + j, dy = dy0 + j;
        for (int i = 0; i < cw; i++) {
            int sx = sx0 + i, dx = dx0 + i;
            const ColorRGB &s = from_data[(size_t)sy * width + sx];
            ColorRGBA &d = pixels[(size_t)dy * info.width + dx];
            if (ea >= 255) { d.r = s.r; d.g = s.g; d.b = s.b; d.a = 255; }
            else {
                d.r = (uint8_t)((s.r * ea + d.r * inv + 127) / 255);
                d.g = (uint8_t)((s.g * ea + d.g * inv + 127) / 255);
                d.b = (uint8_t)((s.b * ea + d.b * inv + 127) / 255);
                d.a = (uint8_t)(ea + d.a * inv / 255); // source-over alpha
            }
        }
    }
}

void png::copy(const std::vector<ColorRGBA> &from_data, int width, int height,
               int x1, int y1, int x2, int y2, int pos_x, int pos_y, uint8_t A) {
    if (from_data.empty() || width <= 0 || height <= 0) return;
    if (pixels.empty() || info.width == 0 || info.height == 0) return;
    int sx0, sy0, dx0, dy0, cw, ch;
    if (!calcCopyRange(width, height, x1, y1, x2, y2, pos_x, pos_y,
                       (int)info.width, (int)info.height, sx0, sy0, dx0, dy0, cw, ch)) return;
    for (int j = 0; j < ch; j++) {
        int sy = sy0 + j, dy = dy0 + j;
        for (int i = 0; i < cw; i++) {
            int sx = sx0 + i, dx = dx0 + i;
            const ColorRGBA &s = from_data[(size_t)sy * width + sx];
            ColorRGBA &d = pixels[(size_t)dy * info.width + dx];
            int ea = s.a * A / 255; // 全局透明度 × 源alpha
            if (ea >= 255) { d = s; }
            else if (ea > 0) {
                int inv = 255 - ea;
                d.r = (uint8_t)((s.r * ea + d.r * inv + 127) / 255);
                d.g = (uint8_t)((s.g * ea + d.g * inv + 127) / 255);
                d.b = (uint8_t)((s.b * ea + d.b * inv + 127) / 255);
                d.a = (uint8_t)(ea + d.a * inv / 255); // source-over alpha
            }
        }
    }
}
// ---------- 解码 ----------
bool png::read(const std::string &filename)
{
    filename_ = filename;
    file_.open(filename, std::ios::binary);
    if (!file_.is_open())
        return false;
    read_ok_ = true;

    if (readU8() != 0x89 || !read_ok_)
    {
        file_.close();
        return false;
    }
    char png_sig[3];
    file_.read(png_sig, 3);
    if (file_.gcount() != 3 || std::string(png_sig, 3) != "PNG")
    {
        file_.close();
        return false;
    }
    if (readU32BE() != 0x0D0A1A0A || !read_ok_)
    {
        file_.close();
        return false;
    }

    idat_accum_.clear();
    bool hasIHDR = false;
    bool hasIDAT = false;

    while (read_ok_ && !file_.eof())
    {
        Block b = readBlock();
        if (!read_ok_)
            break;
        if (!b.checkcrc)
        {
            if (b.name == "IHDR" || b.name == "IDAT" || b.name == "IEND" ||
                b.name == "PLTE")
            {
                file_.close();
                return false;
            }
        }
        if (b.name == "IHDR")
        {
            if (hasIHDR)
            {
                file_.close();
                return false;
            }
            processIHDR(b);
            if (info.width == 0 || info.height == 0)
            {
                file_.close();
                return false;
            }
            hasIHDR = true;
        }
        else if (!hasIHDR && b.name != "IHDR")
        {
            file_.close();
            return false;
        }
        else if (b.name == "sBIT")
            info.sbit = b.data;
        else if (b.name == "PLTE")
            processPLTE(b);
        else if (b.name == "tRNS")
            info.trns = b.data;
        else if (b.name == "pHYs")
            processPHYS(b);
        else if (b.name == "cHRM")
            processCHRM(b);
        else if (b.name == "tEXt")
            info.create = std::string(b.data.begin(), b.data.end());
        else if (b.name == "IDAT")
        {
            if (!hasIHDR)
            {
                file_.close();
                return false;
            }
            if (idat_accum_.size() + b.data.size() > MAX_DECOMPRESSED_SIZE * 2)
            {
                file_.close();
                return false;
            }
            idat_accum_.insert(idat_accum_.end(), b.data.begin(), b.data.end());
            hasIDAT = true;
        }
        else if (b.name == "IEND")
        {
            break;
        }
    }
    file_.close();
    if (!hasIHDR || !hasIDAT)
        return false;
    if (!inflateIDAT())
        return false;
    decode();
    return true;
}

void png::decode()
{
    if (data_.empty())
        return;
    if (static_cast<size_t>(info.width) * info.height > MAX_PIXELS)
        return;
    switch (info.colortype)
    {
    case 0: decodeGrayscale(false); break;
    case 2: decodeTruecolor(3); break;
    case 3: indexrow(); break;
    case 4: decodeGrayscale(true); break;
    case 6: decodeTruecolor(4); break;
    default: break;
    }
}

// ---------- 创建空白图像 ----------
bool png::create(uint32_t width, uint32_t height, int colortype, int bitdepth)
{
    if (width == 0 || height == 0)
        return false;
    if (colortype != 2 && colortype != 6)
    {
        std::cerr << "仅支持 RGB (2) 或 RGBA (6) 创建" << std::endl;
        return false;
    }
    info.width       = width;
    info.height      = height;
    info.bitdepth    = static_cast<uint8_t>(bitdepth);
    info.colortype   = static_cast<uint8_t>(colortype);
    info.compression = 0;
    info.filter      = 0;
    info.interlace   = 0;
    pixels.assign(static_cast<size_t>(width) * height, {0, 0, 0, 255});
    return true;
}

// 将二维码模块数据(qrcode库输出, 非零=暗模块)缩放绘制到本图
bool png::qrdata(std::vector<uint8_t>& qr_data, int q_width, int q_height,
                 unsigned char scale, int pos_x, int pos_y) {
    if (q_width <= 0 || q_height <= 0) return false;
    if (pixels.empty() || info.width == 0 || info.height == 0) return false;
    if (scale == 0) scale = 1;
    if ((size_t)q_width * q_height > qr_data.size()) return false;
    int W = (int)info.width, H = (int)info.height;

    for (int y = 0; y < q_height; ++y) {
        for (int x = 0; x < q_width; ++x) {
            if (!qr_data[(size_t)y * q_width + x]) continue; // 只画暗模块(非零=黑)
            int bx = pos_x + x * scale;
            int by = pos_y + y * scale;
            for (int dy = 0; dy < scale; ++dy) {
                int py = by + dy;
                if (py < 0 || py >= H) continue;
                for (int dx = 0; dx < scale; ++dx) {
                    int px = bx + dx;
                    if (px < 0 || px >= W) continue;
                    pixels[(size_t)py * W + px] = ColorRGBA{0, 0, 0, 255}; // 暗模块=黑
                }
            }
        }
    }
    return true;
}

void png::fillColor(const ColorRGBA &c)
{
    std::fill(pixels.begin(), pixels.end(), c);
}

void png::setPixel(uint32_t x, uint32_t y, const ColorRGBA &c)
{
    if (x < info.width && y < info.height)
        at(x, y) = c;
}

ColorRGBA png::getPixel(uint32_t x, uint32_t y) const
{
    if (x < info.width && y < info.height)
        return at(x, y);
    return ColorRGBA{0, 0, 0, 0};
}

// ---------- 矩形填充 ----------
void png::setRect(int x1, int y1, int x2, int y2, const ColorRGBA &c)
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return;
    if (x1 > x2)
        std::swap(x1, x2);
    if (y1 > y2)
        std::swap(y1, y2);
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 >= static_cast<int>(info.width))
        x2 = static_cast<int>(info.width) - 1;
    if (y2 >= static_cast<int>(info.height))
        y2 = static_cast<int>(info.height) - 1;
    if (x1 > x2 || y1 > y2)
        return;
    for (int y = y1; y <= y2; ++y)
    {
        ColorRGBA *r = rowPtr(static_cast<uint32_t>(y));
        std::fill(r + x1, r + x2 + 1, c);
    }
}

// ---------- 单像素直线 ----------
void png::drawSinglePixelLine(int x1, int y1, int x2, int y2, const ColorRGBA &c)
{
    int dx  = abs(x2 - x1);
    int dy  = abs(y2 - y1);
    int sx  = (x1 < x2) ? 1 : -1;
    int sy  = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true)
    {
        if (x1 >= 0 && x1 < static_cast<int>(info.width) &&
            y1 >= 0 && y1 < static_cast<int>(info.height))
            setPixel(static_cast<uint32_t>(x1), static_cast<uint32_t>(y1), c);
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

// ---------- 画线 ----------
void png::drawLine(int x1, int y1, int x2, int y2, const ColorRGBA &c, int thickness)
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return;
    if (thickness <= 0)
        thickness = 1;

    if (y1 == y2)
    {
        int half = thickness / 2;
        for (int offY = -half; offY <= thickness - half - 1; ++offY)
            drawSinglePixelLine(x1, y1 + offY, x2, y1 + offY, c);
        return;
    }
    if (x1 == x2)
    {
        int half = thickness / 2;
        for (int offX = -half; offX <= thickness - half - 1; ++offX)
            drawSinglePixelLine(x1 + offX, y1, x1 + offX, y2, c);
        return;
    }

    if (thickness == 1)
    {
        drawSinglePixelLine(x1, y1, x2, y2, c);
        return;
    }

    double dx  = static_cast<double>(x2 - x1);
    double dy  = static_cast<double>(y2 - y1);
    double len = sqrt(dx * dx + dy * dy);
    if (len == 0)
    {
        setPixel(x1, y1, c);
        return;
    }
    double nx = -dy / len;
    double ny = dx / len;
    int half  = thickness / 2;
    for (int offset = -half; offset <= thickness - half - 1; ++offset)
    {
        int ox = static_cast<int>(nx * offset + 0.5);
        int oy = static_cast<int>(ny * offset + 0.5);
        drawSinglePixelLine(x1 + ox, y1 + oy, x2 + ox, y2 + oy, c);
    }
}

void png::drawLine(int x, int y, int length, const ColorRGBA &c)
{
    if (length <= 0)
        return;
    drawLine(x, y, x + length - 1, y, c, 1);
}

// ---------- 图像缩放 (Bicubic 双三次插值) ----------
namespace
{
// Catmull-Rom 三次插值核 (a = -0.5)
double bicubicW(double x)
{
    const double a = -0.5;
    double ax = std::fabs(x);
    if (ax <= 1.0)
        return (a + 2.0) * ax * ax * ax - (a + 3.0) * ax * ax + 1.0;
    if (ax < 2.0)
        return a * ax * ax * ax - 5.0 * a * ax * ax + 8.0 * a * ax - 4.0 * a;
    return 0.0;
}
uint8_t toByte(double v)
{
    if (v < 0.0)
        return 0;
    if (v > 255.0)
        return 255;
    return static_cast<uint8_t>(v + 0.5);
}
} // namespace

bool png::resize(uint32_t newWidth, uint32_t newHeight)
{
    if (newWidth == 0 || newHeight == 0)
        return false;
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return false;
    if (newWidth == info.width && newHeight == info.height)
        return true;
    // 防止超大分配
    if (static_cast<uint64_t>(newWidth) * newHeight > MAX_PIXELS)
        return false;

    const uint32_t sw = info.width, sh = info.height;
    std::vector<ColorRGBA> dst(static_cast<size_t>(newWidth) * newHeight);
    double xRatio = static_cast<double>(sw) / newWidth;
    double yRatio = static_cast<double>(sh) / newHeight;

    for (uint32_t oy = 0; oy < newHeight; oy++)
    {
        double sy = (oy + 0.5) * yRatio - 0.5;
        int y0 = static_cast<int>(std::floor(sy)) - 1;
        for (uint32_t ox = 0; ox < newWidth; ox++)
        {
            double sx = (ox + 0.5) * xRatio - 0.5;
            int x0 = static_cast<int>(std::floor(sx)) - 1;

            double sumR = 0, sumG = 0, sumB = 0, sumA = 0, sumW = 0;
            for (int j = 0; j < 4; j++)
            {
                int yy = y0 + j;
                if (yy < 0)
                    yy = 0;
                else if (yy >= static_cast<int>(sh))
                    yy = static_cast<int>(sh) - 1;
                double wy = bicubicW(sy - (y0 + j));
                const ColorRGBA *row = pixels.data() + static_cast<size_t>(yy) * sw;
                for (int i = 0; i < 4; i++)
                {
                    int xx = x0 + i;
                    if (xx < 0)
                        xx = 0;
                    else if (xx >= static_cast<int>(sw))
                        xx = static_cast<int>(sw) - 1;
                    double w = wy * bicubicW(sx - (x0 + i));
                    const ColorRGBA &c = row[xx];
                    sumR += c.r * w;
                    sumG += c.g * w;
                    sumB += c.b * w;
                    sumA += c.a * w;
                    sumW += w;
                }
            }
            ColorRGBA &out = dst[static_cast<size_t>(oy) * newWidth + ox];
            out.r = toByte(sumR / sumW);
            out.g = toByte(sumG / sumW);
            out.b = toByte(sumB / sumW);
            out.a = toByte(sumA / sumW);
        }
    }

    pixels = std::move(dst);
    info.width = newWidth;
    info.height = newHeight;
    return true;
}

bool png::resizeScale(double scale)
{
    if (scale <= 0.0)
        return false;
    uint32_t nw = static_cast<uint32_t>(std::llround(info.width * scale));
    uint32_t nh = static_cast<uint32_t>(std::llround(info.height * scale));
    if (nw < 1)
        nw = 1;
    if (nh < 1)
        nh = 1;
    return resize(nw, nh);
}

// ---------- 保存 ----------
bool png::save(const std::string &filename)
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return false;
    int ct = info.colortype;
    if (ct == 2 || ct == 6)
    {
        if (!encode(ct, info.bitdepth))
            return false;
        std::ofstream f(filename, std::ios::binary);
        if (!f.is_open())
            return false;
        const std::string& data = oss_.str();  // 唯一一次拷贝（str内部拷贝）
        f.write(data.data(), data.size());
        f.close();
        return true;
    }
    else if (ct == 3)
    {
        std::cerr << "索引色保存暂未实现" << std::endl;
        return false;
    }
    else
    {
        std::cerr << "不支持的颜色类型" << std::endl;
        return false;
    }
}

std::string png::saveAndShow(const std::string& filename)
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return {};
    int ct = info.colortype;
    if (ct == 2 || ct == 6)
    {
        // 一次编码，填充 oss_
        if (!encode(ct, info.bitdepth))
            return {};

        // 取出数据（只拷贝一次）
        std::string data = oss_.str();

        // 写入文件
        std::ofstream f(filename, std::ios::binary);
        if (!f.is_open())
            return {};   // 保存失败，但数据依然可以返回（可根据需求调整）
        f.write(data.data(), data.size());
        f.close();

        // 返回数据（移动语义，通常无额外拷贝）
        return data;
    }
    else
    {
        // 其他颜色类型暂不支持，返回空
        return {};
    }
}
// ---------- imshow ----------
std::string png::imshow()
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return {};
    int ct = info.colortype;
    if (ct == 2 || ct == 6)
    {
        if (!encode(ct, info.bitdepth))
            return {};
        return oss_.str();
    }
    else
    {
        // 可扩展其他颜色类型
        return {};
    }
}

// ---------- 工具 ----------
std::string png::rgbHex(uint8_t v)
{
    const char *hex = "0123456789ABCDEF";
    return std::string{hex[v >> 4], hex[v & 0x0F]};
}

// ==================== 私有实现 ====================
// ---------- 底层读取 ----------
uint8_t png::readU8()
{
    if (!read_ok_)
        return 0;
    char c;
    file_.read(&c, 1);
    if (file_.gcount() != 1)
    {
        read_ok_ = false;
        return 0;
    }
    return static_cast<uint8_t>(c);
}
uint16_t png::readU16BE()
{
    if (!read_ok_)
        return 0;
    uint8_t b[2] = {0};
    file_.read(reinterpret_cast<char *>(b), 2);
    if (file_.gcount() != 2)
    {
        read_ok_ = false;
        return 0;
    }
    return (static_cast<uint16_t>(b[0]) << 8) | static_cast<uint16_t>(b[1]);
}
uint32_t png::readU32BE()
{
    if (!read_ok_)
        return 0;
    uint8_t b[4] = {0};
    file_.read(reinterpret_cast<char *>(b), 4);
    if (file_.gcount() != 4)
    {
        read_ok_ = false;
        return 0;
    }
    return (static_cast<uint32_t>(b[0]) << 24) |
           (static_cast<uint32_t>(b[1]) << 16) |
           (static_cast<uint32_t>(b[2]) << 8) |
           static_cast<uint32_t>(b[3]);
}
std::string png::readBName()
{
    if (!read_ok_)
        return std::string(4, '\0');
    char n[5] = {0};
    file_.read(n, 4);
    if (file_.gcount() != 4)
    {
        read_ok_ = false;
        return std::string(4, '\0');
    }
    return std::string(n, 4);
}
std::vector<uint8_t> png::readSize(uint32_t len)
{
    if (!read_ok_ || len == 0)
        return {};
    if (len > MAX_BLOCK_LENGTH)
    {
        read_ok_ = false;
        return {};
    }
    std::vector<uint8_t> buf(len);
    file_.read(reinterpret_cast<char *>(buf.data()), len);
    if (static_cast<uint32_t>(file_.gcount()) != len)
    {
        read_ok_ = false;
        return {};
    }
    return buf;
}
uint32_t png::readCRC() { return readU32BE(); }

png::Block png::readBlock()
{
    Block b;
    b.length = readU32BE();
    b.name   = readBName();
    b.data   = readSize(b.length);
    b.crc    = readCRC();
    if (!read_ok_)
    {
        b.checkcrc = false;
        return b;
    }
    uint32_t crc = crc32(0, reinterpret_cast<const Bytef *>(b.name.c_str()), 4);
    if (!b.data.empty())
    {
        crc = crc32(crc, b.data.data(), b.data.size());
    }
    b.checkcrc = (crc == b.crc);
    return b;
}

// ---------- 块处理 ----------
void png::processIHDR(const Block &b)
{
    if (b.data.size() < 13)
        return;
    uint32_t w = (static_cast<uint32_t>(b.data[0]) << 24) |
                 (static_cast<uint32_t>(b.data[1]) << 16) |
                 (static_cast<uint32_t>(b.data[2]) << 8) |
                 static_cast<uint32_t>(b.data[3]);
    uint32_t h = (static_cast<uint32_t>(b.data[4]) << 24) |
                 (static_cast<uint32_t>(b.data[5]) << 16) |
                 (static_cast<uint32_t>(b.data[6]) << 8) |
                 static_cast<uint32_t>(b.data[7]);
    if (w == 0 || h == 0 || w > MAX_WIDTH || h > MAX_HEIGHT)
        return;

    uint8_t bd   = b.data[8];
    uint8_t ct   = b.data[9];
    bool validBD = false;
    switch (ct)
    {
    case 0: validBD = (bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16); break;
    case 2: validBD = (bd == 8 || bd == 16); break;
    case 3: validBD = (bd == 1 || bd == 2 || bd == 4 || bd == 8); break;
    case 4: validBD = (bd == 8 || bd == 16); break;
    case 6: validBD = (bd == 8 || bd == 16); break;
    default: break;
    }
    if (!validBD)
        return;

    info.width       = w;
    info.height      = h;
    info.bitdepth    = bd;
    info.colortype   = ct;
    info.compression = b.data[10];
    info.filter      = b.data[11];
    info.interlace   = b.data[12];
}
void png::processPLTE(const Block &b)
{
    info.plte.clear();
    for (size_t i = 0; i + 2 < b.data.size(); i += 3)
        info.plte.push_back({b.data[i], b.data[i + 1], b.data[i + 2]});
}
void png::processPHYS(const Block &b)
{
    if (b.data.size() < 9)
        return;
    info.phys.x    = (static_cast<uint32_t>(b.data[0]) << 24) | (static_cast<uint32_t>(b.data[1]) << 16) | (static_cast<uint32_t>(b.data[2]) << 8) | b.data[3];
    info.phys.y    = (static_cast<uint32_t>(b.data[4]) << 24) | (static_cast<uint32_t>(b.data[5]) << 16) | (static_cast<uint32_t>(b.data[6]) << 8) | b.data[7];
    info.phys.unit = b.data[8];
}
void png::processCHRM(const Block &b)
{
    if (b.data.size() < 32)
        return;
    auto g = [&](size_t o)
    { return (static_cast<uint32_t>(b.data[o]) << 24) | (static_cast<uint32_t>(b.data[o + 1]) << 16) | (static_cast<uint32_t>(b.data[o + 2]) << 8) | static_cast<uint32_t>(b.data[o + 3]); };
    info.chrm.wx = g(0);
    info.chrm.wy = g(4);
    info.chrm.rx = g(8);
    info.chrm.ry = g(12);
    info.chrm.gx = g(16);
    info.chrm.gy = g(20);
    info.chrm.bx = g(24);
    info.chrm.by = g(28);
}

// ---------- 解压 IDAT ----------
bool png::inflateIDAT()
{
    if (idat_accum_.empty())
        return false;
    if (info.width == 0 || info.height == 0)
        return false;

    int channels            = (info.colortype == 0) ? 1 :
                              (info.colortype == 2) ? 3 :
                              (info.colortype == 3) ? 1 :
                              (info.colortype == 4) ? 2 :
                                                      4;
    uint32_t bpp            = (info.bitdepth == 16) ? 2 : 1;
    size_t expectedRowBytes = (info.width * channels * bpp + 7) / 8 + 1;
    size_t expectedSize     = static_cast<size_t>(expectedRowBytes) * info.height;
    if (expectedSize > MAX_DECOMPRESSED_SIZE)
        return false;

    z_stream strm{};
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = static_cast<uInt>(idat_accum_.size());
    strm.next_in  = idat_accum_.data();
    if (inflateInit(&strm) != Z_OK)
        return false;

    std::vector<uint8_t> out;
    out.reserve(expectedSize);
    const size_t CHUNK = 16384;
    int ret;
    do
    {
        if (out.size() + CHUNK > MAX_DECOMPRESSED_SIZE)
        {
            inflateEnd(&strm);
            return false;
        }
        out.resize(out.size() + CHUNK);
        strm.avail_out = static_cast<uInt>(CHUNK);
        strm.next_out  = out.data() + out.size() - CHUNK;
        ret            = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR ||
            ret == Z_MEM_ERROR || ret == Z_NEED_DICT)
        {
            inflateEnd(&strm);
            return false;
        }
        if (ret == Z_BUF_ERROR && strm.avail_in > 0)
        {
            inflateEnd(&strm);
            return false;
        }
        out.resize(out.size() - strm.avail_out);
    } while (ret != Z_STREAM_END);
    inflateEnd(&strm);

    if (out.size() != expectedSize)
    {
        if (out.size() > MAX_DECOMPRESSED_SIZE)
            return false;
    }
    data_.swap(out);
    seek_ = 0;
    return true;
}

// ---------- 行解码辅助 ----------
uint8_t png::rRowHeader() { return (seek_ < data_.size()) ? data_[seek_++] : 0; }
uint8_t png::rBits() { return (seek_ < data_.size()) ? data_[seek_++] : 0; }

// ---------- 反滤波公共逻辑 ----------
void png::unfilterRow(uint8_t filter,
                      const uint8_t *raw,
                      const std::vector<uint8_t> &prevRow,
                      std::vector<uint8_t> &currRow,
                      uint32_t rowBytes,
                      uint32_t bpp)
{
    switch (filter)
    {
    case 1: // Sub
        for (uint32_t i = 0; i < rowBytes; ++i)
        {
            uint8_t left = (i >= bpp) ? currRow[i - bpp] : 0;
            currRow[i]   = static_cast<uint8_t>(raw[i] + left);
        }
        break;
    case 2: // Up
        for (uint32_t i = 0; i < rowBytes; ++i)
            currRow[i] = static_cast<uint8_t>(raw[i] + prevRow[i]);
        break;
    case 3: // Average
        for (uint32_t i = 0; i < rowBytes; ++i)
        {
            uint8_t left = (i >= bpp) ? currRow[i - bpp] : 0;
            currRow[i]   = static_cast<uint8_t>(raw[i] + ((prevRow[i] + left) / 2));
        }
        break;
    case 4: // Paeth
        for (uint32_t i = 0; i < rowBytes; ++i)
        {
            uint8_t left   = (i >= bpp) ? currRow[i - bpp] : 0;
            uint8_t up     = prevRow[i];
            uint8_t upleft = (i >= bpp) ? prevRow[i - bpp] : 0;
            currRow[i]     = static_cast<uint8_t>(raw[i] + paethPredictor(left, up, upleft));
        }
        break;
    case 0:  // None
    default:
        std::copy(raw, raw + rowBytes, currRow.begin());
        break;
    }
}

// ---------- 真彩色解码 ----------
void png::decodeTruecolor(uint32_t channels)
{
    if (info.width == 0 || info.height == 0)
        return;
    const uint32_t sampleBytes = (info.bitdepth == 16) ? 2 : 1;
    const uint32_t bpp         = channels * sampleBytes;
    const uint32_t rowBytes    = info.width * bpp;
    const size_t expected      = static_cast<size_t>(info.height) * (1 + rowBytes);
    if (data_.size() < expected)
        return;
    pixels.assign(static_cast<size_t>(info.width) * info.height, {0, 0, 0, 255});

    std::vector<uint8_t> prevRow(rowBytes, 0);
    std::vector<uint8_t> currRow(rowBytes);

    for (uint32_t y = 0; y < info.height; ++y)
    {
        uint8_t filter        = rRowHeader();
        const uint8_t *rawPtr = data_.data() + seek_;
        seek_ += rowBytes;

        unfilterRow(filter, rawPtr, prevRow, currRow, rowBytes, bpp);

        ColorRGBA *dst         = pixels.data() + static_cast<size_t>(y) * info.width;
        const uint8_t *src = currRow.data();

        if (sampleBytes == 1 && channels == 4 && sizeof(ColorRGBA) == 4)
        {
            std::copy(currRow.begin(), currRow.end(),
                      reinterpret_cast<uint8_t *>(dst));
        }
        else if (sampleBytes == 1)
        {
            for (uint32_t x = 0; x < info.width; ++x)
            {
                const uint8_t *p = src + static_cast<size_t>(x) * channels;
                dst[x].r = p[0];
                dst[x].g = p[1];
                dst[x].b = p[2];
                dst[x].a = (channels == 4) ? p[3] : 255;
            }
        }
        else
        {
            for (uint32_t x = 0; x < info.width; ++x)
            {
                const uint8_t *p = src + static_cast<size_t>(x) * channels * 2;
                uint16_t r16 = (static_cast<uint16_t>(p[0]) << 8) | p[1];
                uint16_t g16 = (static_cast<uint16_t>(p[2]) << 8) | p[3];
                uint16_t b16 = (static_cast<uint16_t>(p[4]) << 8) | p[5];
                dst[x].r = static_cast<uint8_t>(r16 >> 8);
                dst[x].g = static_cast<uint8_t>(g16 >> 8);
                dst[x].b = static_cast<uint8_t>(b16 >> 8);
                dst[x].a = (channels == 4) ? static_cast<uint8_t>(((static_cast<uint16_t>(p[6]) << 8) | p[7]) >> 8) : 255;
            }
        }

        std::swap(currRow, prevRow);
    }
}

// ---------- 灰度解码 ----------
void png::decodeGrayscale(bool hasAlpha)
{
    if (info.width == 0 || info.height == 0)
        return;
    const uint32_t channels     = hasAlpha ? 2 : 1;
    const uint8_t bd            = info.bitdepth;
    const uint32_t bitsPerPixel = channels * bd;
    const uint32_t rowBytes     = static_cast<uint32_t>(
        (static_cast<uint64_t>(info.width) * bitsPerPixel + 7) / 8);
    const uint32_t bpp     = (bitsPerPixel >= 8) ? (bitsPerPixel / 8) : 1;
    const size_t expected  = static_cast<size_t>(info.height) * (1 + rowBytes);
    if (data_.size() < expected)
        return;
    pixels.assign(static_cast<size_t>(info.width) * info.height, {0, 0, 0, 255});

    std::vector<uint8_t> prevRow(rowBytes, 0);
    std::vector<uint8_t> currRow(rowBytes);
    const uint32_t maxval = (bd >= 16) ? 65535u : ((1u << bd) - 1);

    for (uint32_t y = 0; y < info.height; ++y)
    {
        uint8_t filter        = rRowHeader();
        const uint8_t *rawPtr = data_.data() + seek_;
        seek_ += rowBytes;

        unfilterRow(filter, rawPtr, prevRow, currRow, rowBytes, bpp);

        ColorRGBA *dst         = pixels.data() + static_cast<size_t>(y) * info.width;
        const uint8_t *src = currRow.data();

        if (bd == 16)
        {
            for (uint32_t x = 0; x < info.width; ++x)
            {
                const uint8_t *p = src + static_cast<size_t>(x) * channels * 2;
                uint16_t gray16 = (static_cast<uint16_t>(p[0]) << 8) | p[1];
                uint8_t gray = static_cast<uint8_t>(gray16 >> 8);
                dst[x].r = dst[x].g = dst[x].b = gray;
                dst[x].a = hasAlpha ? static_cast<uint8_t>(((static_cast<uint16_t>(p[2]) << 8) | p[3]) >> 8) : 255;
            }
        }
        else if (bd == 8)
        {
            for (uint32_t x = 0; x < info.width; ++x)
            {
                const uint8_t *p = src + static_cast<size_t>(x) * channels;
                uint8_t gray     = p[0];
                dst[x].r = dst[x].g = dst[x].b = gray;
                dst[x].a = hasAlpha ? p[1] : 255;
            }
        }
        else
        {
            const uint32_t spb = 8u / bd;
            const uint8_t mask = static_cast<uint8_t>((1u << bd) - 1);
            for (uint32_t x = 0; x < info.width; ++x)
            {
                uint32_t byteIdx = x / spb;
                uint32_t within  = x % spb;
                uint8_t shift    = static_cast<uint8_t>(8u - bd * (within + 1));
                uint8_t val      = (src[byteIdx] >> shift) & mask;
                uint8_t gray     = static_cast<uint8_t>(val * 255u / maxval);
                dst[x].r = dst[x].g = dst[x].b = gray;
                dst[x].a = 255;
            }
        }

        std::swap(currRow, prevRow);
    }
}

void png::indexrow()
{
    uint32_t w = info.width, h = info.height;
    uint8_t bd = info.bitdepth;
    if (bd == 0 || w == 0 || h == 0)
        return;
    uint32_t bytesPerRow = (w * bd + 7) / 8;
    pixels.assign(static_cast<size_t>(w) * h, {0, 0, 0, 255});

    std::vector<uint8_t> prevRow(bytesPerRow, 0);
    std::vector<uint8_t> currRow(bytesPerRow);

    for (uint32_t y = 0; y < h; ++y)
    {
        uint8_t filter = rRowHeader();
        if (seek_ + bytesPerRow > data_.size())
            break;
        const uint8_t *rawPtr = data_.data() + seek_;
        seek_ += bytesPerRow;

        unfilterRow(filter, rawPtr, prevRow, currRow, bytesPerRow, 1);

        ColorRGBA *dst         = pixels.data() + static_cast<size_t>(y) * w;
        const uint8_t *src = currRow.data();

        if (bd == 8)
        {
            for (uint32_t x = 0; x < w; ++x)
                dst[x] = paletteColor(src[x]);
        }
        else
        {
            const uint32_t spb = 8u / bd;
            const uint8_t mask = static_cast<uint8_t>((1u << bd) - 1);
            for (uint32_t x = 0; x < w; ++x)
            {
                uint32_t byteIdx = x / spb;
                uint32_t within  = x % spb;
                uint8_t shift    = static_cast<uint8_t>(8u - bd * (within + 1));
                uint8_t idx      = (src[byteIdx] >> shift) & mask;
                dst[x]           = paletteColor(idx);
            }
        }

        std::swap(currRow, prevRow);
    }
}

ColorRGBA png::paletteColor(uint32_t idx) const
{
    ColorRGBA pixel;
    if (idx < info.plte.size())
    {
        pixel.r = info.plte[idx].r;
        pixel.g = info.plte[idx].g;
        pixel.b = info.plte[idx].b;
    }
    if (idx < info.trns.size())
        pixel.a = info.trns[idx];
    return pixel;
}

// ==================== 编码实现 ====================
// ---------- 写入函数（直接写 oss_） ----------
void png::writeSignature()
{
    const uint8_t sig[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    oss_.write(reinterpret_cast<const char *>(sig), 8);
}

void png::writeU32BE(uint32_t v)
{
    uint8_t b[4] = {
        static_cast<uint8_t>((v >> 24) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>(v & 0xFF)};
    oss_.write(reinterpret_cast<const char *>(b), 4);
}

void png::writeChunk(const std::string &type, const std::vector<uint8_t> &data)
{
    uint32_t len = static_cast<uint32_t>(data.size());
    writeU32BE(len);
    oss_.write(type.c_str(), 4);
    if (!data.empty())
        oss_.write(reinterpret_cast<const char *>(data.data()), data.size());
    uint32_t crc = crc32(0, reinterpret_cast<const Bytef *>(type.c_str()), 4);
    if (!data.empty())
        crc = crc32(crc, data.data(), data.size());
    writeU32BE(crc);
}

void png::writeIHDR(uint32_t w, uint32_t h, uint8_t bd, uint8_t ct)
{
    std::vector<uint8_t> data(13);
    data[0]  = (w >> 24) & 0xFF;
    data[1]  = (w >> 16) & 0xFF;
    data[2]  = (w >> 8) & 0xFF;
    data[3]  = w & 0xFF;
    data[4]  = (h >> 24) & 0xFF;
    data[5]  = (h >> 16) & 0xFF;
    data[6]  = (h >> 8) & 0xFF;
    data[7]  = h & 0xFF;
    data[8]  = bd;
    data[9]  = ct;
    data[10] = 0;
    data[11] = 0;
    data[12] = 0;
    writeChunk("IHDR", data);
}

void png::writeIEND()
{
    writeChunk("IEND", {});
}

// ---------- 核心编码（仅写到 oss_） ----------
bool png::encode(int colortype, int bitdepth)
{
    if (pixels.empty() || info.width == 0 || info.height == 0)
        return false;
    uint32_t width = info.width, height = info.height;
    if (bitdepth != 8)
        return false;
    if (colortype != 0 && colortype != 2 && colortype != 4 && colortype != 6)
        return false;
    if (pixels.size() < static_cast<size_t>(width) * height)
        return false;

    int channels = (colortype == 0) ? 1 : (colortype == 2 ? 3 : (colortype == 4 ? 2 : 4));
    int bpp = channels;

    std::vector<std::vector<uint8_t>> rawRows;
    rawRows.reserve(height);
    for (uint32_t y = 0; y < height; ++y)
    {
        const ColorRGBA *rowPix = pixels.data() + static_cast<size_t>(y) * width;
        std::vector<uint8_t> row;
        row.reserve(static_cast<size_t>(width) * bpp);
        for (uint32_t x = 0; x < width; ++x)
        {
            const ColorRGBA &c = rowPix[x];
            if (colortype == 0)
            {
                uint8_t gray = static_cast<uint8_t>(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
                row.push_back(gray);
            }
            else if (colortype == 2)
            {
                row.push_back(c.r);
                row.push_back(c.g);
                row.push_back(c.b);
            }
            else if (colortype == 4)
            {
                uint8_t gray = static_cast<uint8_t>(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
                row.push_back(gray);
                row.push_back(c.a);
            }
            else // colortype == 6
            {
                row.push_back(c.r);
                row.push_back(c.g);
                row.push_back(c.b);
                row.push_back(c.a);
            }
        }
        rawRows.push_back(std::move(row));
    }

    oss_.str("");
    oss_.clear();
    writeSignature();
    writeIHDR(width, height, bitdepth, colortype);
    int bppCalc = (colortype == 0) ? 1 : (colortype == 2 ? 3 : (colortype == 4 ? 2 : 4));
    if (!encodeIDAT(rawRows, bppCalc, width, height, bitdepth))
        return false;
    writeIEND();
    return true;
}

// ---------- encodeIDAT ----------
bool png::encodeIDAT(const std::vector<std::vector<uint8_t>> &rows,
                     int bpp,
                     uint32_t w,
                     uint32_t h,
                     [[maybe_unused]] int bd,
                     int forceFilter)
{
    std::vector<uint8_t> filteredData;
    filteredData.reserve(static_cast<size_t>(h) * (static_cast<size_t>(w) * bpp + 1));

    std::vector<uint8_t> bestFiltered(w * bpp);
    std::vector<uint8_t> scratch(w * bpp);
    std::vector<uint8_t> bestOutput;

    for (uint32_t y = 0; y < h; ++y)
    {
        const auto &row = rows[y];
        int bestFilter  = 0;

        if (forceFilter >= 0)
        {
            bestFilter = forceFilter;
            applyFilter(row, bestFilter, bpp, w, y, rows, bestFiltered);
            bestOutput = bestFiltered;
        }
        else if (speedMode_)
        {
            uint32_t bestScore = UINT32_MAX;
            int filters[]      = {0, 4};
            for (int filter : filters)
            {
                uint32_t score = computeFilterScore(row, filter, bpp, w, y, rows, scratch);
                if (score < bestScore)
                {
                    bestScore  = score;
                    bestFilter = filter;
                    std::swap(bestFiltered, scratch);
                }
            }
            bestOutput = std::move(bestFiltered);
        }
        else
        {
            uint32_t bestScore = UINT32_MAX;
            for (int filter = 0; filter <= 4; ++filter)
            {
                uint32_t score = computeFilterScore(row, filter, bpp, w, y, rows, scratch);
                if (score < bestScore)
                {
                    bestScore  = score;
                    bestFilter = filter;
                    std::swap(bestFiltered, scratch);
                }
            }
            bestOutput = std::move(bestFiltered);
        }

        filteredData.push_back(static_cast<uint8_t>(bestFilter));
        filteredData.insert(filteredData.end(), bestOutput.begin(), bestOutput.end());
        bestFiltered.resize(w * bpp);
    }

    auto compressed = compressZlib(filteredData);
    if (compressed.empty())
        return false;
    writeChunk("IDAT", compressed);
    return true;
}

// ---------- 滤波辅助 ----------
void png::applyFilter(const std::vector<uint8_t> &row,
                      int filterType,
                      int bpp,
                      uint32_t width,
                      uint32_t y,
                      const std::vector<std::vector<uint8_t>> &rows,
                      std::vector<uint8_t> &out)
{
    const uint32_t rowBytes = width * bpp;
    if (out.size() < rowBytes)
        out.resize(rowBytes);

    const uint8_t *rowPtr = row.data();
    const uint8_t *upPtr  = (y > 0) ? rows[y - 1].data() : nullptr;
    uint8_t *outPtr       = out.data();

    switch (filterType)
    {
    case 0:
        std::copy(rowPtr, rowPtr + rowBytes, outPtr);
        break;
    case 1:
        for (uint32_t x = 0; x < rowBytes; ++x)
        {
            uint8_t left = (x >= static_cast<uint32_t>(bpp)) ? rowPtr[x - bpp] : 0;
            outPtr[x]    = static_cast<uint8_t>(rowPtr[x] - left);
        }
        break;
    case 2:
        if (upPtr)
        {
            for (uint32_t x = 0; x < rowBytes; ++x)
            {
                outPtr[x] = static_cast<uint8_t>(rowPtr[x] - upPtr[x]);
            }
        }
        else
        {
            std::copy(rowPtr, rowPtr + rowBytes, outPtr);
        }
        break;
    case 3:
        for (uint32_t x = 0; x < rowBytes; ++x)
        {
            uint8_t left = (x >= static_cast<uint32_t>(bpp)) ? rowPtr[x - bpp] : 0;
            uint8_t up   = upPtr ? upPtr[x] : 0;
            outPtr[x]    = static_cast<uint8_t>(rowPtr[x] - ((left + up) / 2));
        }
        break;
    case 4:
        for (uint32_t x = 0; x < rowBytes; ++x)
        {
            uint8_t left   = (x >= static_cast<uint32_t>(bpp)) ? rowPtr[x - bpp] : 0;
            uint8_t up     = upPtr ? upPtr[x] : 0;
            uint8_t upleft = (upPtr && x >= static_cast<uint32_t>(bpp)) ? upPtr[x - bpp] : 0;
            outPtr[x]      = static_cast<uint8_t>(rowPtr[x] - paethPredictor(left, up, upleft));
        }
        break;
    default:
        std::copy(rowPtr, rowPtr + rowBytes, outPtr);
        break;
    }
}

uint32_t png::computeFilterScore(const std::vector<uint8_t> &row,
                                 int filterType,
                                 int bpp,
                                 uint32_t width,
                                 uint32_t y,
                                 const std::vector<std::vector<uint8_t>> &rows,
                                 std::vector<uint8_t> &scratch)
{
    applyFilter(row, filterType, bpp, width, y, rows, scratch);
    uint32_t score = 0;
    const int8_t *p = reinterpret_cast<const int8_t *>(scratch.data());
    uint32_t n = width * bpp;
    for (uint32_t i = 0; i < n; ++i)
    {
        int32_t abs_val = static_cast<int32_t>(p[i]);
        score += static_cast<uint32_t>(abs_val < 0 ? -abs_val : abs_val);
    }
    return score;
}

uint8_t png::paethPredictor(uint8_t a, uint8_t b, uint8_t c)
{
    int p  = static_cast<int>(a) + static_cast<int>(b) - static_cast<int>(c);
    int pa = std::abs(p - static_cast<int>(a)), pb = std::abs(p - static_cast<int>(b)), pc = std::abs(p - static_cast<int>(c));
    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

std::vector<uint8_t> png::compressZlib(const std::vector<uint8_t> &data)
{
    if (data.empty())
        return {};
    uLongf destLen = compressBound(data.size());
    std::vector<uint8_t> compressed(destLen);
    int ret;
    if (speedMode_)
    {
        ret = compress2(compressed.data(), &destLen, data.data(), data.size(), Z_BEST_SPEED);
    }
    else
    {
        ret = compress(compressed.data(), &destLen, data.data(), data.size());
    }
    if (ret != Z_OK)
        return {};
    compressed.resize(destLen);
    return compressed;
}

} // namespace image