/*
 * SVG Chart Generator - Implementation
 * Algorithms extracted from ECharts 6.1.0
 */

#include "svg_chart.h"
#include <cmath>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// SvgChart 基类实现
// ============================================================

SvgChart::SvgChart(int width, int height)
    : width_(width), height_(height) {
    gridRect_ = ChartRect(
        kMarginLeft,
        kMarginTop,
        width_ - kMarginLeft - kMarginRight,
        height_ - kMarginTop - kMarginBottom
    );
}

void SvgChart::setSize(int width, int height) {
    width_ = width;
    height_ = height;
    gridRect_ = ChartRect(
        kMarginLeft,
        kMarginTop,
        width_ - kMarginLeft - kMarginRight,
        height_ - kMarginTop - kMarginBottom
    );
}

void SvgChart::setTitle(const std::string& title) {
    title_ = title;
}

void SvgChart::setFontFamily(const std::string& font) {
    fontFamily_ = font;
}

std::string SvgChart::svgHeader() const {
    std::ostringstream oss;
    oss << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << width_ << "\" height=\"" << height_ << "\" "
        << "viewBox=\"0 0 " << width_ << " " << height_ << "\">\n";
    oss << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    return oss.str();
}

std::string SvgChart::svgFooter() const {
    return "</svg>\n";
}

std::string SvgChart::svgRect(double x, double y, double w, double h,
                              const std::string& fill, double rx) const {
    std::ostringstream oss;
    oss << "<rect x=\"" << x << "\" y=\"" << y
        << "\" width=\"" << w << "\" height=\"" << h
        << "\" fill=\"" << fill << "\"";
    if (rx > 0) oss << " rx=\"" << rx << "\"";
    oss << "/>\n";
    return oss.str();
}

std::string SvgChart::svgCircle(double cx, double cy, double r,
                                const std::string& fill) const {
    std::ostringstream oss;
    oss << "<circle cx=\"" << cx << "\" cy=\"" << cy
        << "\" r=\"" << r << "\" fill=\"" << fill << "\"/>\n";
    return oss.str();
}

std::string SvgChart::svgLine(double x1, double y1, double x2, double y2,
                              const std::string& stroke, double width) const {
    std::ostringstream oss;
    oss << "<line x1=\"" << x1 << "\" y1=\"" << y1
        << "\" x2=\"" << x2 << "\" y2=\"" << y2
        << "\" stroke=\"" << stroke << "\" stroke-width=\"" << width << "\"/>\n";
    return oss.str();
}

std::string SvgChart::svgPolyline(const std::vector<SvgPoint>& pts,
                                  const std::string& stroke, double width,
                                  const std::string& fill) const {
    std::ostringstream oss;
    oss << "<polyline points=\"";
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i > 0) oss << " ";
        oss << pts[i].x << "," << pts[i].y;
    }
    oss << "\" fill=\"" << fill
        << "\" stroke=\"" << stroke
        << "\" stroke-width=\"" << width
        << "\" stroke-linejoin=\"round\"/>\n";
    return oss.str();
}

std::string SvgChart::svgPath(const std::string& d,
                              const std::string& stroke, double width,
                              const std::string& fill) const {
    std::ostringstream oss;
    oss << "<path d=\"" << d
        << "\" fill=\"" << fill
        << "\" stroke=\"" << stroke
        << "\" stroke-width=\"" << width
        << "\" stroke-linejoin=\"round\"/>\n";
    return oss.str();
}

std::string SvgChart::svgText(double x, double y, const std::string& text,
                              const std::string& anchor, const std::string& fill,
                              int fontSize) const {
    std::string escaped = escapeXml(text);

    std::ostringstream oss;
    oss << "<text x=\"" << x << "\" y=\"" << y
        << "\" text-anchor=\"" << anchor
        << "\" fill=\"" << fill
        << "\" font-size=\"" << fontSize
        << "\" font-family=\"" << fontFamily_ << "\">"
        << escaped << "</text>\n";
    return oss.str();
}

std::string SvgChart::svgTitleText(const std::string& text) const {
    std::string escaped = escapeXml(text);

    std::ostringstream oss;
    oss << "<text x=\"" << (width_ / 2.0)
        << "\" y=\"" << (kMarginTop / 2.0 + 5)
        << "\" text-anchor=\"middle\""
        << " fill=\"#333\" font-size=\"16\""
        << " font-weight=\"bold\""
        << " font-family=\"" << fontFamily_ << "\">"
        << escaped << "</text>\n";
    return oss.str();
}

std::string SvgChart::escapeXml(const std::string& text) {
    std::string result = text;
    for (size_t pos = 0; (pos = result.find('&', pos)) != std::string::npos; pos += 5)
        result.replace(pos, 1, "&amp;");
    for (size_t pos = 0; (pos = result.find('<', pos)) != std::string::npos; pos += 4)
        result.replace(pos, 1, "&lt;");
    for (size_t pos = 0; (pos = result.find('>', pos)) != std::string::npos; pos += 4)
        result.replace(pos, 1, "&gt;");
    return result;
}

std::string SvgChart::drawAxes(const std::vector<std::string>& xLabels,
                               const std::vector<std::string>& yLabels) const {
    std::ostringstream oss;

    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    // Y 轴
    oss << svgLine(gx, gy, gx, gy + gh, "#333", 1);
    // X 轴
    oss << svgLine(gx, gy + gh, gx + gw, gy + gh, "#333", 1);

    // Y 轴刻度线和标签
    if (!yLabels.empty()) {
        int n = static_cast<int>(yLabels.size());
        for (int i = 0; i < n; ++i) {
            double ratio = (n == 1) ? 0.5 : static_cast<double>(i) / (n - 1);
            double y = gy + gh * (1.0 - ratio);
            // 背景横线
            oss << svgLine(gx, y, gx + gw, y, "#e0e0e0", 1);
            // 刻度短线
            oss << svgLine(gx - 4, y, gx, y, "#999", 1);
            oss << svgText(gx - 8, y + 4, yLabels[i], "end", "#666", 11);
        }
    }

    // X 轴刻度线和标签
    if (!xLabels.empty()) {
        int n = static_cast<int>(xLabels.size());
        for (int i = 0; i < n; ++i) {
            double ratio = (n == 1) ? 0.5 : static_cast<double>(i) / (n - 1);
            double x = gx + gw * ratio;
            oss << svgLine(x, gy + gh, x, gy + gh + 4, "#999", 1);
            oss << svgText(x, gy + gh + 20, xLabels[i], "middle", "#666", 11);
        }
    }

    return oss.str();
}

std::string SvgChart::fmt(double val, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << val;
    return oss.str();
}

int SvgChart::tickPrecision(double span, int ticks) {
    if (ticks < 1) ticks = 1;
    // 取能区分相邻刻度的最少位数：刻度间距比分辨率大一倍以上就够了
    double step = std::fabs(span) / ticks;
    if (step >= 5.0) return 0;
    if (step >= 0.5) return 1;
    if (step >= 0.05) return 2;
    return 3;
}

// ============================================================
// SvgLineChart 实现
// ============================================================

SvgLineChart::SvgLineChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgLineChart::addSeries(const std::vector<double>& data,
                             const std::string& name,
                             const SvgColor& color) {
    Series s;
    s.data = data;
    s.name = name;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        // 使用默认颜色（如果用户没有设置）
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

void SvgLineChart::setXLabels(const std::vector<std::string>& labels) {
    xLabels_ = labels;
}

std::string SvgLineChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    // 标题
    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    int maxLen = 0;

    for (const auto& s : series_) {
        maxLen = std::max(maxLen, static_cast<int>(s.data.size()));
        for (double v : s.data) {
            if (std::isfinite(v)) {
                yMin = std::min(yMin, v);
                yMax = std::max(yMax, v);
            }
        }
    }

    // 扩展 Y 轴范围
    if (yMin == yMax) {
        yMin -= 1;
        yMax += 1;
    }
    double yPadding = (yMax - yMin) * 0.1;
    yMax += yPadding;
    if (yMin >= 0) {
        yMin = 0;
    } else {
        yMin -= yPadding;
    }

    // 坐标变换
    LinearScale xScale, yScale;
    xScale.setDomain(0, maxLen > 1 ? maxLen - 1.0 : 1.0);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    // Y 轴标签
    std::vector<std::string> yLabels;
    int yTicks = 5;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }

    // X 轴标签
    std::vector<std::string> xLabels = xLabels_;
    if (xLabels.empty()) {
        for (int i = 0; i < maxLen; ++i) {
            xLabels.push_back(std::to_string(i + 1));
        }
    }

    // 绘制坐标轴
    oss << drawAxes(xLabels, yLabels);

    // 绘制每条折线
    for (const auto& s : series_) {
        std::vector<SvgPoint> points;
        for (size_t i = 0; i < s.data.size(); ++i) {
            if (!std::isfinite(s.data[i])) continue;
            double x = xScale.map(static_cast<double>(i));
            double y = yScale.map(s.data[i]);
            points.push_back(SvgPoint(x, y));
        }

        if (points.empty()) continue;

        // 折线
        oss << svgPolyline(points, s.color.toHex(), 2, "none");

        // 数据点圆点
        for (const auto& pt : points) {
            oss << svgCircle(pt.x, pt.y, 4, s.color.toHex());
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgSmoothLineChart 实现
// 核心平滑算法来自 ECharts poly.ts 的 drawSegment 函数
// ============================================================

SvgSmoothLineChart::SvgSmoothLineChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgSmoothLineChart::addSeries(const std::vector<double>& data,
                                   const std::string& name,
                                   const SvgColor& color,
                                   double smooth) {
    Series s;
    s.data = data;
    s.name = name;
    s.smooth = std::max(0.0, std::min(1.0, smooth));
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

void SvgSmoothLineChart::setXLabels(const std::vector<std::string>& labels) {
    xLabels_ = labels;
}

// 平滑曲线核心算法
// 来自 ECharts poly.ts drawSegment:
//   使用 Catmull-Rom 风格的三次贝塞尔曲线平滑点序列
//   控制点计算基于相邻点之间的距离和方向，并施加约束避免超调
std::string SvgSmoothLineChart::buildSmoothPath(
    const std::vector<SvgPoint>& points, double smooth) const {

    if (points.empty()) return "";
    if (points.size() == 1) {
        return "M" + std::to_string(points[0].x) + "," + std::to_string(points[0].y);
    }

    std::ostringstream d;
    d << "M" << points[0].x << "," << points[0].y;

    // 如果 smooth == 0，直接使用折线
    if (smooth <= 0.0) {
        for (size_t i = 1; i < points.size(); ++i) {
            d << "L" << points[i].x << "," << points[i].y;
        }
        return d.str();
    }

    // 上一个控制点（当前段切出方向）
    double cpx0 = points[0].x;
    double cpy0 = points[0].y;

    for (size_t i = 1; i < points.size(); ++i) {
        double prevX = (i >= 1) ? points[i - 1].x : points[0].x;
        double prevY = (i >= 1) ? points[i - 1].y : points[0].y;
        double x = points[i].x;
        double y = points[i].y;

        double dx = x - prevX;
        double dy = y - prevY;

        // 忽略太短的段
        if ((dx * dx + dy * dy) < 0.5) {
            continue;
        }

        // 下一个点
        bool hasNext = (i + 1 < points.size());
        double nextX = hasNext ? points[i + 1].x : x;
        double nextY = hasNext ? points[i + 1].y : y;

        double cpx1, cpy1;
        double nextCpx0, nextCpy0;

        if (!hasNext) {
            // 最后一个点：控制点在同一点
            cpx1 = x;
            cpy1 = y;
            nextCpx0 = x;
            nextCpy0 = y;
        } else {
            double vx = nextX - prevX;
            double vy = nextY - prevY;

            double dx0 = x - prevX;
            double dx1 = nextX - x;
            double dy0 = y - prevY;
            double dy1 = nextY - y;

            // 使用 Catmull-Rom 风格计算控制点
            double lenPrevSeg = std::sqrt(dx0 * dx0 + dy0 * dy0);
            double lenNextSeg = std::sqrt(dx1 * dx1 + dy1 * dy1);

            // 使用段长度比例
            double ratioNextSeg = lenNextSeg / (lenNextSeg + lenPrevSeg);

            cpx1 = x - vx * smooth * (1.0 - ratioNextSeg);
            cpy1 = y - vy * smooth * (1.0 - ratioNextSeg);

            nextCpx0 = x + vx * smooth * ratioNextSeg;
            nextCpy0 = y + vy * smooth * ratioNextSeg;

            // 约束 nextCpx0 在 [nextX, x] 范围内，避免超调
            nextCpx0 = std::min(nextCpx0, std::max(nextX, x));
            nextCpy0 = std::min(nextCpy0, std::max(nextY, y));
            nextCpx0 = std::max(nextCpx0, std::min(nextX, x));
            nextCpy0 = std::max(nextCpy0, std::min(nextY, y));

            // 根据调整后的 nextCpx0 重新计算 cpx1
            vx = nextCpx0 - x;
            vy = nextCpy0 - y;

            cpx1 = x - vx * lenPrevSeg / lenNextSeg;
            cpy1 = y - vy * lenPrevSeg / lenNextSeg;

            // 约束 cpx1 在 [prevX, x] 范围内
            cpx1 = std::min(cpx1, std::max(prevX, x));
            cpy1 = std::min(cpy1, std::max(prevY, y));
            cpx1 = std::max(cpx1, std::min(prevX, x));
            cpy1 = std::max(cpy1, std::min(prevY, y));

            // 再次调整 nextCpx0
            vx = x - cpx1;
            vy = y - cpy1;
            nextCpx0 = x + vx * lenNextSeg / lenPrevSeg;
            nextCpy0 = y + vy * lenNextSeg / lenPrevSeg;
        }

        d << "C" << cpx0 << "," << cpy0 << " "
          << cpx1 << "," << cpy1 << " "
          << x << "," << y;

        cpx0 = nextCpx0;
        cpy0 = nextCpy0;
    }

    return d.str();
}

std::string SvgSmoothLineChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    int maxLen = 0;

    for (const auto& s : series_) {
        maxLen = std::max(maxLen, static_cast<int>(s.data.size()));
        for (double v : s.data) {
            if (std::isfinite(v)) {
                yMin = std::min(yMin, v);
                yMax = std::max(yMax, v);
            }
        }
    }

    if (yMin == yMax) {
        yMin -= 1;
        yMax += 1;
    }
    double yPadding = (yMax - yMin) * 0.1;
    yMax += yPadding;
    if (yMin >= 0) {
        yMin = 0;
    } else {
        yMin -= yPadding;
    }

    LinearScale xScale, yScale;
    xScale.setDomain(0, maxLen > 1 ? maxLen - 1.0 : 1.0);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    std::vector<std::string> yLabels;
    int yTicks = 5;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }

    std::vector<std::string> xLabels = xLabels_;
    if (xLabels.empty()) {
        for (int i = 0; i < maxLen; ++i) {
            xLabels.push_back(std::to_string(i + 1));
        }
    }

    oss << drawAxes(xLabels, yLabels);

    for (const auto& s : series_) {
        std::vector<SvgPoint> points;
        for (size_t i = 0; i < s.data.size(); ++i) {
            if (!std::isfinite(s.data[i])) continue;
            double x = xScale.map(static_cast<double>(i));
            double y = yScale.map(s.data[i]);
            points.push_back(SvgPoint(x, y));
        }

        if (points.empty()) continue;

        // 平滑曲线路径
        std::string pathD = buildSmoothPath(points, s.smooth);
        oss << svgPath(pathD, s.color.toHex(), 2, "none");

        // 数据点圆点
        for (const auto& pt : points) {
            oss << svgCircle(pt.x, pt.y, 4, s.color.toHex());
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgBarChart 实现
// ============================================================

SvgBarChart::SvgBarChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgBarChart::addSeries(const std::vector<double>& data,
                            const std::string& name,
                            const SvgColor& color) {
    Series s;
    s.data = data;
    s.name = name;
    s.usePerBarColor = false;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

void SvgBarChart::addSeries(const std::vector<double>& data,
                            const std::vector<SvgColor>& colors,
                            const std::string& name) {
    Series s;
    s.data = data;
    s.name = name;
    s.usePerBarColor = true;
    s.colors = colors;
    s.color = palette_[paletteIdx_ % palette_.size()];
    paletteIdx_++;
    series_.push_back(s);
}

void SvgBarChart::setXLabels(const std::vector<std::string>& labels) {
    xLabels_ = labels;
}

std::string SvgBarChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double yMin = 0;
    double yMax = std::numeric_limits<double>::lowest();
    int maxLen = 0;

    for (const auto& s : series_) {
        maxLen = std::max(maxLen, static_cast<int>(s.data.size()));
        for (double v : s.data) {
            if (std::isfinite(v)) {
                yMax = std::max(yMax, v);
                yMin = std::min(yMin, v);
            }
        }
    }

    if (yMax <= 0) yMax = 1;
    double yPadding = (yMax - yMin) * 0.1;
    yMax += yPadding;
    if (yMin >= 0) {
        yMin = 0;
    } else {
        yMin -= yPadding;
    }

    LinearScale yScale;
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    // 每个分类的宽度
    double categoryWidth = gridRect_.width / std::max(1, maxLen);

    std::vector<std::string> yLabels;
    int yTicks = 5;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }

    std::vector<std::string> xLabels = xLabels_;
    if (xLabels.empty()) {
        for (int i = 0; i < maxLen; ++i) {
            xLabels.push_back(std::to_string(i + 1));
        }
    }

    // 绘制坐标轴 Y轴 + 底部 X 轴线
    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    bool hasNegative = (yMin < 0);
    double zeroY = yScale.map(0);

    oss << svgLine(gx, gy, gx, gy + gh, "#333", 1);
    // X 轴零线（正负条形图：零线在中间，加粗）
    if (hasNegative) {
        oss << svgLine(gx, zeroY, gx + gw, zeroY, "#333", 1.5);
    } else {
        oss << svgLine(gx, gy + gh, gx + gw, gy + gh, "#333", 1);
    }

    // Y 轴刻度
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        double y = yScale.map(val);
        // 背景横线（零线用虚线突出）
        std::string gridColor = (hasNegative && std::abs(val) < 0.001) ? "#999" : "#e0e0e0";
        double gridWidth = (hasNegative && std::abs(val) < 0.001) ? 1.0 : 1.0;
        oss << svgLine(gx, y, gx + gw, y, gridColor, gridWidth);
        oss << svgLine(gx - 4, y, gx, y, "#999", 1);
        oss << svgText(gx - 8, y + 4, yLabels[i], "end", "#666", 11);
    }

    // X 轴刻度（分类居中）
    for (int i = 0; i < maxLen; ++i) {
        double cx = gx + (i + 0.5) * categoryWidth;
        oss << svgLine(cx, gy + gh, cx, gy + gh + 4, "#999", 1);
        if (i < static_cast<int>(xLabels.size())) {
            oss << svgText(cx, gy + gh + 20, xLabels[i], "middle", "#666", 11);
        }
    }

    // 添加裁剪区域，防止柱子超出网格
    std::string clipId = "barClip";
    oss << "<defs>\n";
    oss << "<clipPath id=\"" << clipId << "\">\n";
    oss << "<rect x=\"" << gx << "\" y=\"" << gy
        << "\" width=\"" << gw << "\" height=\"" << gh << "\"/>\n";
    oss << "</clipPath>\n";
    oss << "</defs>\n";
    oss << "<g clip-path=\"url(#" << clipId << ")\">\n";

    // 柱子组宽度 = 分类宽度的 60%
    double barGroupWidth = categoryWidth * 0.6;
    double barWidth = barGroupWidth / std::max(1, static_cast<int>(series_.size()));
    double barGap = 0; // 柱子之间无间隙

    // 绘制每组柱子
    for (int i = 0; i < maxLen; ++i) {
        double centerX = gx + (i + 0.5) * categoryWidth;
        double startX = centerX - barGroupWidth / 2.0;

        for (size_t si = 0; si < series_.size(); ++si) {
            if (i >= static_cast<int>(series_[si].data.size())) continue;
            double val = series_[si].data[i];
            if (!std::isfinite(val)) continue;

            double valY = yScale.map(val);
            double x = startX + si * (barWidth + barGap);
            double y = std::min(valY, zeroY);
            double h = std::abs(valY - zeroY);

            // 使用独立颜色或系列默认颜色
            SvgColor barColor = series_[si].color;
            if (series_[si].usePerBarColor &&
                i < static_cast<int>(series_[si].colors.size())) {
                barColor = series_[si].colors[i];
            }
            oss << svgRect(x, y, barWidth, h, barColor.toHex(), 2);
        }
    }

    oss << "</g>\n";

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgScatterChart 实现
// ============================================================

SvgScatterChart::SvgScatterChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgScatterChart::addSeries(const std::vector<SvgPoint>& data,
                                const std::string& name,
                                const SvgColor& color,
                                double symbolSize) {
    Series s;
    s.data = data;
    s.name = name;
    s.symbolSize = symbolSize;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

std::string SvgScatterChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    for (const auto& s : series_) {
        for (const auto& pt : s.data) {
            if (std::isfinite(pt.x) && std::isfinite(pt.y)) {
                xMin = std::min(xMin, pt.x);
                xMax = std::max(xMax, pt.x);
                yMin = std::min(yMin, pt.y);
                yMax = std::max(yMax, pt.y);
            }
        }
    }

    if (xMin == xMax) { xMin -= 1; xMax += 1; }
    if (yMin == yMax) { yMin -= 1; yMax += 1; }

    double xPadding = (xMax - xMin) * 0.1;
    double yPadding = (yMax - yMin) * 0.1;
    xMin -= xPadding; xMax += xPadding;
    yMin -= yPadding; yMax += yPadding;

    LinearScale xScale, yScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    // 生成 Y 轴标签
    std::vector<std::string> yLabels;
    int yTicks = 5;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }

    // 生成 X 轴标签
    std::vector<std::string> xLabels;
    int xTicks = 5;
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        xLabels.push_back(fmt(val, 0));
    }

    oss << drawAxes(xLabels, yLabels);

    // 绘制散点
    for (const auto& s : series_) {
        for (const auto& pt : s.data) {
            if (!std::isfinite(pt.x) || !std::isfinite(pt.y)) continue;
            double sx = xScale.map(pt.x);
            double sy = yScale.map(pt.y);
            oss << svgCircle(sx, sy, s.symbolSize, s.color.toHex());
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgPieChart 实现
// 核心算法来自 ECharts pieLayout.ts
// ============================================================

SvgPieChart::SvgPieChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgPieChart::addData(const std::string& name, double value,
                          const SvgColor& color) {
    PieDataItem item;
    item.name = name;
    item.value = value;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        item.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        item.color = color;
    }
    data_.push_back(item);
}

void SvgPieChart::setInnerRadius(double ratio) {
    innerRadiusRatio_ = std::max(0.0, std::min(1.0, ratio));
    if (ratio > 0) isDonut_ = true;
}

void SvgPieChart::setDonut(bool donut) {
    isDonut_ = donut;
    if (donut && innerRadiusRatio_ == 0.0) {
        innerRadiusRatio_ = 0.5;
    }
    if (!donut) {
        innerRadiusRatio_ = 0.0;
    }
}

// 构建扇区 SVG 路径
// 算法来自 ECharts pieLayout.ts 的角度计算逻辑
std::string SvgPieChart::buildSectorPath(double cx, double cy, double r, double r0,
                                         double startAngle, double endAngle) const {
    // 将角度转换为弧度坐标
    auto angleToPoint = [&](double angle, double radius) -> SvgPoint {
        return SvgPoint(
            cx + radius * std::cos(angle),
            cy + radius * std::sin(angle)
        );
    };

    double delta = endAngle - startAngle;
    int largeArc = (std::abs(delta) > M_PI) ? 1 : 0;

    SvgPoint outerStart = angleToPoint(startAngle, r);
    SvgPoint outerEnd = angleToPoint(endAngle, r);

    std::ostringstream d;

    if (r0 <= 0) {
        // 饼图（无内半径）
        d << "M" << cx << "," << cy << " ";
        d << "L" << outerStart.x << "," << outerStart.y << " ";
        d << "A" << r << "," << r << " 0 " << largeArc << " 1 "
          << outerEnd.x << "," << outerEnd.y << " ";
        d << "Z";
    } else {
        // 环形图
        SvgPoint innerStart = angleToPoint(startAngle, r0);
        SvgPoint innerEnd = angleToPoint(endAngle, r0);

        d << "M" << outerStart.x << "," << outerStart.y << " ";
        d << "A" << r << "," << r << " 0 " << largeArc << " 1 "
          << outerEnd.x << "," << outerEnd.y << " ";
        d << "L" << innerEnd.x << "," << innerEnd.y << " ";
        d << "A" << r0 << "," << r0 << " 0 " << largeArc << " 0 "
          << innerStart.x << "," << innerStart.y << " ";
        d << "Z";
    }

    return d.str();
}

SvgPoint SvgPieChart::labelPosition(double cx, double cy, double r,
                                    double startAngle, double endAngle) const {
    double midAngle = (startAngle + endAngle) / 2.0;
    double labelR = r * 0.7; // 标签在半径的 70% 处
    return SvgPoint(
        cx + labelR * std::cos(midAngle),
        cy + labelR * std::sin(midAngle)
    );
}

std::string SvgPieChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算圆心和半径
    double cx = width_ / 2.0;
    double cy = (height_ + kMarginTop - kMarginBottom) / 2.0;
    double maxR = std::min(gridRect_.width, gridRect_.height) / 2.0 * 0.85;
    double r0 = maxR * innerRadiusRatio_;

    // 计算总和（来自 ECharts pieLayout 算法）
    double sum = 0;
    for (const auto& item : data_) {
        if (!std::isnan(item.value) && item.value > 0) {
            sum += item.value;
        }
    }

    if (sum <= 0) {
        oss << svgFooter();
        return oss.str();
    }

    // 起始角度（ECharts 默认从 -90° 开始，即 12 点钟方向）
    double startAngle = -M_PI / 2.0;
    double currentAngle = startAngle;

    // 为每个扇区生成路径
    for (size_t i = 0; i < data_.size(); ++i) {
        if (std::isnan(data_[i].value) || data_[i].value <= 0) continue;

        // 按比例计算角度（来自 ECharts pieLayout）
        double angle = (data_[i].value / sum) * 2.0 * M_PI;
        double endAngle = currentAngle + angle;

        // 构建并绘制扇区
        std::string pathD = buildSectorPath(cx, cy, maxR, r0, currentAngle, endAngle);
        oss << svgPath(pathD, "#fff", 1.5, data_[i].color.toHex());

        // 绘制标签：用黑色文字，白色标签在小扇区溢出到白色背景上会看不见
        SvgPoint labelPos = labelPosition(cx, cy, maxR, currentAngle, endAngle);
        oss << svgText(labelPos.x, labelPos.y, data_[i].name,
                       "middle", "#333", 11);

        currentAngle = endAngle;
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgRadarChart 实现
// 核心算法来自 ECharts:
//   - Radar.ts: coordToPoint (将坐标值转换为极坐标点)
//   - radarLayout.ts: 雷达图布局算法
// ============================================================

SvgRadarChart::SvgRadarChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgRadarChart::setIndicators(const std::vector<std::string>& names) {
    indicators_ = names;
}

void SvgRadarChart::addSeries(const std::vector<double>& data,
                              const std::string& name,
                              const SvgColor& color) {
    Series s;
    s.data = data;
    s.name = name;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

SvgRadarChart::RadarLayout SvgRadarChart::computeLayout() const {
    RadarLayout layout;

    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    layout.cx = gx + gw / 2.0;
    layout.cy = gy + gh / 2.0;
    layout.r = std::min(gw, gh) / 2.0 * 0.75;
    layout.r0 = 0;

    // 起始角度：12点钟方向（ECharts 默认）
    layout.startAngle = -M_PI / 2.0;

    int n = std::max(1, static_cast<int>(indicators_.size()));
    for (int i = 0; i < n; ++i) {
        double angle = layout.startAngle + i * 2.0 * M_PI / n;
        layout.axisAngles.push_back(angle);
    }

    return layout;
}

SvgPoint SvgRadarChart::coordToPoint(double cx, double cy, double r, double r0,
                                     double value, double min, double max,
                                     double angle) const {
    // 将值映射到 [r0, r] 范围
    double ratio = (max == min) ? 0.5 : (value - min) / (max - min);
    double radius = r0 + ratio * (r - r0);

    // 极坐标转直角坐标 (来自 ECharts Radar.ts coordToPoint)
    double x = cx + radius * std::cos(angle);
    double y = cy - radius * std::sin(angle);
    return SvgPoint(x, y);
}

void SvgRadarChart::setFillOpacity(double opacity) {
    fillOpacity_ = std::max(0.0, std::min(1.0, opacity));
}

void SvgRadarChart::setBorderWidth(double width) {
    borderWidth_ = std::max(0.0, width);
}

std::string SvgRadarChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty() || indicators_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    auto layout = computeLayout();
    int n = static_cast<int>(indicators_.size());

    // 计算数据范围
    double dataMin = std::numeric_limits<double>::max();
    double dataMax = std::numeric_limits<double>::lowest();
    for (const auto& s : series_) {
        for (double v : s.data) {
            if (std::isfinite(v)) {
                dataMin = std::min(dataMin, v);
                dataMax = std::max(dataMax, v);
            }
        }
    }
    if (dataMin == dataMax) {
        dataMin = 0;
        dataMax = dataMin + 1;
    }
    double padding = (dataMax - dataMin) * 0.1;
    dataMin -= padding;
    dataMax += padding;
    if (dataMin > 0) dataMin = 0;

    // ============================================================
    // 绘制背景网格（蜘蛛网线）
    // ============================================================

    int splitCount = 5;
    for (int level = 1; level <= splitCount; ++level) {
        double ratio = static_cast<double>(level) / splitCount;
        double levelR = layout.r0 + ratio * (layout.r - layout.r0);

        std::ostringstream polyD;
        for (int i = 0; i < n; ++i) {
            double x = layout.cx + levelR * std::cos(layout.axisAngles[i]);
            double y = layout.cy - levelR * std::sin(layout.axisAngles[i]);
            if (i == 0)
                polyD << "M" << x << "," << y;
            else
                polyD << "L" << x << "," << y;
        }
        polyD << "Z";

        oss << "<path d=\"" << polyD.str()
            << "\" fill=\"none\" stroke=\"#e0e0e0\" stroke-width=\"1\"/>\n";

        // 分割圈标签
        if (level == splitCount) {
            double val = dataMax;
            SvgPoint labelPt = coordToPoint(layout.cx, layout.cy, layout.r, layout.r0,
                                            val, dataMin, dataMax,
                                            layout.axisAngles[0]);
            oss << svgText(labelPt.x, labelPt.y - 6, fmt(val, 0), "middle", "#999", 10);
        }
    }

    // 绘制轴线和标签
    for (int i = 0; i < n; ++i) {
        // 轴线
        SvgPoint axisEnd = coordToPoint(layout.cx, layout.cy, layout.r, layout.r0,
                                        dataMax, dataMin, dataMax,
                                        layout.axisAngles[i]);
        oss << svgLine(layout.cx, layout.cy, axisEnd.x, axisEnd.y, "#d0d0d0", 1);

        // 轴标签
        double labelAngle = layout.axisAngles[i];
        double labelR = layout.r + 20;
        double lx = layout.cx + labelR * std::cos(labelAngle);
        double ly = layout.cy - labelR * std::sin(labelAngle);

        std::string anchor = "middle";
        if (lx < layout.cx - 5) anchor = "end";
        else if (lx > layout.cx + 5) anchor = "start";

        oss << svgText(lx, ly + 4, indicators_[i], anchor, "#333", 12);
    }

    // ============================================================
    // 绘制数据系列
    // ============================================================

    for (const auto& s : series_) {
        if (s.data.empty()) continue;

        std::vector<SvgPoint> points;
        for (int i = 0; i < n && i < static_cast<int>(s.data.size()); ++i) {
            double val = s.data[i];
            if (!std::isfinite(val)) {
                val = dataMin;
            }
            points.push_back(coordToPoint(layout.cx, layout.cy, layout.r, layout.r0,
                                          val, dataMin, dataMax,
                                          layout.axisAngles[i]));
        }

        // 闭合多边形（来自 ECharts radarLayout.ts: 将第一个点复制到最后）
        if (!points.empty()) {
            points.push_back(points[0]);
        }

        // 绘制填充区域（半透明，可通过 setFillOpacity(0) 禁用）
        std::ostringstream polyD;
        for (size_t i = 0; i < points.size(); ++i) {
            if (i == 0)
                polyD << "M" << points[i].x << "," << points[i].y;
            else
                polyD << "L" << points[i].x << "," << points[i].y;
        }
        polyD << "Z";

        if (fillOpacity_ > 0) {
            oss << "<path d=\"" << polyD.str()
                << "\" fill=\"rgba(" << s.color.r << "," << s.color.g << "," << s.color.b << "," << fillOpacity_ << ")\""
                << " stroke=\"none\"/>\n";
        }

        // 绘制轮廓线
        if (borderWidth_ > 0) {
            oss << svgPolyline(points, s.color.toHex(), borderWidth_, "none");
        }

        // 绘制数据点
        for (const auto& pt : points) {
            oss << svgCircle(pt.x, pt.y, 4, s.color.toHex());
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgGaugeChart 实现
// 核心算法来自 ECharts GaugeView.ts / GaugeSeries.ts / PointerPath.ts
// ============================================================

SvgGaugeChart::SvgGaugeChart(int width, int height)
    : SvgChart(width, height) {
    // 默认颜色分段
    colorStops_.push_back({0.2, SvgColor(103, 224, 227)});
    colorStops_.push_back({0.5, SvgColor(55, 162, 218)});
    colorStops_.push_back({0.8, SvgColor(250, 200, 88)});
    colorStops_.push_back({1.0, SvgColor(253, 102, 109)});
}

void SvgGaugeChart::setGaugeType(GaugeType type) {
    gaugeType_ = type;
    switch (type) {
        case TYPE_BASIC:
            showPointer_ = true;
            showProgress_ = false;
            showTicks_ = true;
            showLabels_ = true;
            break;
        case TYPE_PROGRESS:
            showPointer_ = false;
            showProgress_ = true;
            showTicks_ = true;
            showLabels_ = true;
            break;
        case TYPE_STAGE_SPEED:
            showPointer_ = true;
            showProgress_ = false;
            showTicks_ = true;
            showLabels_ = true;
            break;
        case TYPE_LEVEL:
            showPointer_ = true;
            showProgress_ = false;
            showTicks_ = false;
            showLabels_ = true;
            splitNumber_ = 4;
            break;
    }
}

void SvgGaugeChart::setValue(double value) {
    value_ = value;
}

void SvgGaugeChart::setMinMax(double minVal, double maxVal) {
    minVal_ = minVal;
    maxVal_ = maxVal;
}

void SvgGaugeChart::setAngleRange(double startAngle, double endAngle) {
    startAngle_ = startAngle;
    endAngle_ = endAngle;
}

void SvgGaugeChart::setColorStops(const std::vector<GaugeColorStop>& stops) {
    colorStops_ = stops;
}

void SvgGaugeChart::setGaugeTitle(const std::string& title) {
    gaugeTitle_ = title;
}

void SvgGaugeChart::setTitleOffset(double xPercent, double yPercent) {
    titleOffsetX_ = xPercent;
    titleOffsetY_ = yPercent;
}

void SvgGaugeChart::setDetailOffset(double xPercent, double yPercent) {
    detailOffsetX_ = xPercent;
    detailOffsetY_ = yPercent;
}

void SvgGaugeChart::setSplitNumber(int n) {
    splitNumber_ = std::max(1, n);
}

void SvgGaugeChart::setShowPointer(bool show) {
    showPointer_ = show;
}

void SvgGaugeChart::setShowProgress(bool show) {
    showProgress_ = show;
}

void SvgGaugeChart::setShowTicks(bool show) {
    showTicks_ = show;
}

void SvgGaugeChart::setShowLabels(bool show) {
    showLabels_ = show;
}

void SvgGaugeChart::setLabelDistance(double distance) {
    labelDistance_ = distance;
}

void SvgGaugeChart::setLabelRotate(const std::string& rotate) {
    labelRotate_ = rotate;
}

void SvgGaugeChart::setLabelFormatter(const std::vector<std::string>& labels) {
    labelFormatter_ = labels;
}

void SvgGaugeChart::setPointerLength(double length) {
    pointerLength_ = std::max(0.1, std::min(1.0, length));
}

void SvgGaugeChart::setTickInheritColor(bool inherit) {
    tickInheritColor_ = inherit;
}

void SvgGaugeChart::setSplitLineWidth(double width) {
    splitLineWidth_ = width;
}

void SvgGaugeChart::setProgressColor(const SvgColor& color) {
    progressColor_ = color;
    progressColorSet_ = true;
}

SvgColor SvgGaugeChart::getColor(double percent) const {
    if (colorStops_.empty()) return SvgColor(84, 112, 198);
    percent = std::max(0.0, std::min(1.0, percent));
    if (percent <= 0) return colorStops_[0].color;
    for (size_t i = 0; i < colorStops_.size(); ++i) {
        if (colorStops_[i].percent >= percent &&
            (i == 0 || colorStops_[i - 1].percent < percent)) {
            return colorStops_[i].color;
        }
    }
    return colorStops_.back().color;
}

std::string SvgGaugeChart::arcPath(double cx, double cy, double r, double r0,
                                   double startAngle, double endAngle,
                                   bool clockwise) const {
    double startRad = startAngle * M_PI / 180.0;
    double endRad = endAngle * M_PI / 180.0;

    double x1 = cx + r * std::cos(startRad);
    double y1 = cy + r * std::sin(startRad);
    double x2 = cx + r * std::cos(endRad);
    double y2 = cy + r * std::sin(endRad);

    double sweepAngle = endRad - startRad;
    if (!clockwise) sweepAngle = -sweepAngle;
    int largeArc = (std::abs(sweepAngle) > M_PI) ? 1 : 0;
    int sweep = (sweepAngle >= 0) ? 1 : 0;

    std::ostringstream d;
    if (r0 <= 0 || r0 >= r) {
        // 扇形
        d << "M" << cx << "," << cy << " ";
        d << "L" << x1 << "," << y1 << " ";
        d << "A" << r << "," << r << " 0 " << largeArc << " " << sweep << " "
          << x2 << "," << y2 << " ";
        d << "Z";
    } else {
        // 环形
        double x3 = cx + r0 * std::cos(endRad);
        double y3 = cy + r0 * std::sin(endRad);
        double x4 = cx + r0 * std::cos(startRad);
        double y4 = cy + r0 * std::sin(startRad);

        d << "M" << x1 << "," << y1 << " ";
        d << "A" << r << "," << r << " 0 " << largeArc << " " << sweep << " "
          << x2 << "," << y2 << " ";
        d << "L" << x3 << "," << y3 << " ";
        d << "A" << r0 << "," << r0 << " 0 " << largeArc << " " << (1 - sweep) << " "
          << x4 << "," << y4 << " ";
        d << "Z";
    }
    return d.str();
}

std::string SvgGaugeChart::drawPointer(double cx, double cy, double r,
                                       double angleDeg, double pointerLen) const {
    // 来自 ECharts PointerPath.ts 的 buildPath 算法
    double angle = -angleDeg * M_PI / 180.0;
    double pointerWidth = 6;
    double pointerLength = r * pointerLen;

    double baseAngle = angle - M_PI / 2.0;
    double x = cx - std::cos(angle) * pointerWidth * (pointerWidth >= pointerLength / 3 ? 1 : 2);
    double y = cy - std::sin(angle) * pointerWidth * (pointerWidth >= pointerLength / 3 ? 1 : 2);

    std::ostringstream oss;
    oss << "<polygon points=\"";
    oss << x << "," << y << " ";
    oss << (cx + std::cos(baseAngle) * pointerWidth) << ","
        << (cy + std::sin(baseAngle) * pointerWidth) << " ";
    oss << (cx + std::cos(angle) * pointerLength) << ","
        << (cy + std::sin(angle) * pointerLength) << " ";
    oss << (cx - std::cos(baseAngle) * pointerWidth) << ","
        << (cy - std::sin(baseAngle) * pointerWidth) << " ";
    oss << "\" fill=\"#464646\" stroke=\"none\"/>\n";
    return oss.str();
}

std::string SvgGaugeChart::drawAnchor(double cx, double cy) const {
    std::ostringstream oss;
    double anchorSize = 8;
    oss << svgCircle(cx, cy, anchorSize / 2.0, "#fff");
    oss << "<circle cx=\"" << cx << "\" cy=\"" << cy
        << "\" r=\"" << (anchorSize / 2.0)
        << "\" fill=\"none\" stroke=\"#5470c6\" stroke-width=\"2\"/>\n";
    return oss.str();
}

std::string SvgGaugeChart::drawTicks(double cx, double cy, double r,
                                     double startAngle, double endAngle) const {
    std::ostringstream oss;

    double angleRange = endAngle - startAngle;
    double step = angleRange / splitNumber_;
    double axisLineWidth = 12.0;

    // 分割线长度
    double splitLineLen = r * 0.1;
    double splitLineDist = axisLineWidth + 5;

    for (int i = 0; i <= splitNumber_; ++i) {
        double angleDeg = startAngle + i * step;
        double angleRad = -angleDeg * M_PI / 180.0;
        double unitX = std::cos(angleRad);
        double unitY = std::sin(angleRad);

        // 分割线颜色（继承）
        std::string splitColor = "#999";
        if (tickInheritColor_ && !colorStops_.empty()) {
            double percent = static_cast<double>(i) / splitNumber_;
            splitColor = getColor(percent).toHex();
        }

        // 分割线
        double x1 = unitX * (r - splitLineDist) + cx;
        double y1 = unitY * (r - splitLineDist) + cy;
        double x2 = unitX * (r - splitLineLen - splitLineDist) + cx;
        double y2 = unitY * (r - splitLineLen - splitLineDist) + cy;

        oss << svgLine(x1, y1, x2, y2, splitColor, splitLineWidth_);

        // 标签
        if (showLabels_) {
            // 标签文本
            std::string label;
            if (!labelFormatter_.empty() && i < static_cast<int>(labelFormatter_.size())) {
                label = labelFormatter_[i];
            } else {
                double val = minVal_ + (maxVal_ - minVal_) * i / splitNumber_;
                label = fmt(val, 0);
            }

            if (!label.empty()) {
                double totalDist = splitLineDist + splitLineLen + labelDistance_;
                double lx = unitX * (r - totalDist) + cx;
                double ly = unitY * (r - totalDist) + cy;

                // 标签旋转（来自 ECharts GaugeView._renderTicks）
                // ECharts zrender SVG 输出时 rotation 符号取反
                // 故 rotateDeg = 90 - angleDeg（而非 angleDeg - 90）
                double rotateDeg = 0;
                if (labelRotate_ == "tangential") {
                    rotateDeg = 90 - angleDeg;
                } else if (labelRotate_ == "radial") {
                    rotateDeg = -angleDeg;
                    if (rotateDeg > 90) rotateDeg += 180;
                }

                if (rotateDeg == 0) {
                    // 无旋转时的对齐
                    std::string anchor = "middle";
                    std::string valign = "middle";
                    if (unitY < -0.8) valign = "top";
                    else if (unitY > 0.8) valign = "bottom";
                    if (unitX < -0.4) anchor = "end";
                    else if (unitX > 0.4) anchor = "start";

                    double dy = 4;
                    if (valign == "top") dy = -4;
                    else if (valign == "bottom") dy = 12;

                    oss << svgText(lx, ly + dy, label, anchor, splitColor, 11);
                } else {
                    // 旋转标签
                    oss << "<text x=\"" << lx << "\" y=\"" << ly
                        << "\" text-anchor=\"middle\""
                        << " dominant-baseline=\"central\""
                        << " fill=\"" << splitColor
                        << "\" font-size=\"11\""
                        << " font-family=\"" << fontFamily_ << "\""
                        << " transform=\"rotate(" << rotateDeg << " " << lx << " " << ly << ")\">"
                        << escapeXml(label) << "</text>\n";
                }
            }
        }

        // 刻度线（短刻度）
        if (showTicks_ && i < splitNumber_) {
            int subSplit = 5;
            double subStep = step / subSplit;
            double tickLen = r * 0.06;
            double tickDist = axisLineWidth + 5;

            for (int j = 1; j <= subSplit; ++j) {
                double subAngleDeg = angleDeg + j * subStep;
                double subAngleRad = -subAngleDeg * M_PI / 180.0;
                double sux = std::cos(subAngleRad);
                double suy = std::sin(subAngleRad);

                double tx1 = sux * (r - tickDist) + cx;
                double ty1 = suy * (r - tickDist) + cy;
                double tx2 = sux * (r - tickLen - tickDist) + cx;
                double ty2 = suy * (r - tickLen - tickDist) + cy;

                std::string tickColor = "#ccc";
                if (tickInheritColor_ && !colorStops_.empty()) {
                    double subPercent = (i + static_cast<double>(j) / subSplit) / splitNumber_;
                    tickColor = getColor(subPercent).toHex();
                }

                oss << svgLine(tx1, ty1, tx2, ty2, tickColor, 1);
            }
        }
    }

    return oss.str();
}

std::string SvgGaugeChart::drawTitleAndDetail(double cx, double cy, double r) const {
    std::ostringstream oss;

    if (!gaugeTitle_.empty()) {
        double titleX = cx + r * titleOffsetX_ / 100.0;
        double titleY = cy + r * titleOffsetY_ / 100.0;
        oss << svgText(titleX, titleY, gaugeTitle_, "middle", "#333", 14);
    }

    // 详情（当前值）
    double detailX = cx + r * detailOffsetX_ / 100.0;
    double detailY = cy + r * detailOffsetY_ / 100.0;
    oss << "<text x=\"" << detailX << "\" y=\"" << detailY
        << "\" text-anchor=\"middle\""
        << " fill=\"#333\" font-size=\"28\" font-weight=\"bold\""
        << " font-family=\"" << fontFamily_ << "\">"
        << fmt(value_, 1) << "</text>\n";

    return oss.str();
}

std::string SvgGaugeChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    // 计算圆心和半径 (来自 ECharts parsePosition)
    double cx = width_ / 2.0;
    double cy = height_ / 2.0 + 20; // 略微下移
    double gaugeR = std::min(width_, height_) / 2.0 * 0.75;

    double axisLineWidth = 12.0;

    // 角度标准化（弧度制）
    // 来自 ECharts GaugeView: startAngle = -seriesModel.get('startAngle') / 180 * Math.PI
    double startRad = -startAngle_ * M_PI / 180.0;
    double endRad = -endAngle_ * M_PI / 180.0;
    double angleRange = endRad - startRad;

    // ============================================================
    // 绘制轴线段（彩色分段弧）
    // 来自 ECharts GaugeView._renderMain
    // ============================================================

    if (!colorStops_.empty()) {
        double prevEndAngle = startRad;
        for (size_t i = 0; i < colorStops_.size(); ++i) {
            double percent = std::min(std::max(colorStops_[i].percent, 0.0), 1.0);
            double segEndRad = startRad + angleRange * percent;
            double segEndDeg = segEndRad * 180.0 / M_PI;
            double prevEndDeg = prevEndAngle * 180.0 / M_PI;

            std::string pathD = arcPath(cx, cy, gaugeR, gaugeR - axisLineWidth,
                                        prevEndDeg, segEndDeg, true);
            oss << svgPath(pathD, "none", 0, colorStops_[i].color.toHex());

            prevEndAngle = segEndRad;
        }
    }

    // ============================================================
    // 绘制刻度
    // 来自 ECharts GaugeView._renderTicks
    // ============================================================
    oss << drawTicks(cx, cy, gaugeR, startAngle_, endAngle_);

    // ============================================================
    // 绘制进度条
    // 来自 ECharts GaugeView._renderPointer (progress 部分)
    // ============================================================
    if (showProgress_) {
        double valuePercent = (maxVal_ == minVal_) ? 0.5 :
            (value_ - minVal_) / (maxVal_ - minVal_);
        valuePercent = std::max(0.0, std::min(1.0, valuePercent));
        double valueRad = startRad + angleRange * valuePercent;
        double valueDeg = valueRad * 180.0 / M_PI;

        // 来自 ECharts: progressWidth = isOverlap ? progressModel.get('width') : axisLineWidth / data.count()
        // 进度条比轴线略宽(向内延伸)，覆盖轴线上方
        double progressWidth = axisLineWidth + 2;  // 向内延伸2px，确保覆盖轴线
        std::string pathD = arcPath(cx, cy, gaugeR, gaugeR - progressWidth,
                                    -startAngle_, valueDeg, true);
        // 进度条颜色：来自 ECharts 全局调色板 (visualStyleAccessPath='itemStyle')
        // 默认使用调色板第一色 #5470c6，可通过 setProgressColor 自定义
        SvgColor progressColor = progressColorSet_
            ? progressColor_
            : SvgColor(84, 112, 198);   // 默认调色板色 #5470c6
        oss << svgPath(pathD, "none", 0, progressColor.toHex());
    }

    // ============================================================
    // 绘制指针
    // 来自 ECharts GaugeView._renderPointer (pointer 部分)
    // ============================================================
    if (showPointer_) {
        double valuePercent = (maxVal_ == minVal_) ? 0.5 :
            (value_ - minVal_) / (maxVal_ - minVal_);
        valuePercent = std::max(0.0, std::min(1.0, valuePercent));
        double pointerAngleDeg = startAngle_ + (endAngle_ - startAngle_) * valuePercent;

        oss << drawPointer(cx, cy, gaugeR, pointerAngleDeg, pointerLength_);
        oss << drawAnchor(cx, cy);
    }

    // ============================================================
    // 绘制标题和详情
    // 来自 ECharts GaugeView._renderTitleAndDetail
    // ============================================================
    oss << drawTitleAndDetail(cx, cy, gaugeR);

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgFunnelChart 实现
// 核心算法来自 ECharts funnelLayout.ts
// ============================================================

SvgFunnelChart::SvgFunnelChart(int width, int height)
    : SvgChart(width, height), minSize_(20), maxSize_(100),
      palette_(defaultPalette()) {}

void SvgFunnelChart::addData(const std::string& name, double value,
                             const SvgColor& color) {
    FunnelDataItem item;
    item.name = name;
    item.value = value;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        item.color = palette_[data_.size() % palette_.size()];
    } else {
        item.color = color;
    }
    data_.push_back(item);
}

void SvgFunnelChart::setSort(const std::string& sort) {
    sort_ = sort;
}

void SvgFunnelChart::setFunnelAlign(const std::string& align) {
    funnelAlign_ = align;
}

void SvgFunnelChart::setGap(int gap) {
    gap_ = std::max(0, gap);
}

void SvgFunnelChart::setMinSize(double percent) {
    minSize_ = std::max(0.0, std::min(100.0, percent));
}

void SvgFunnelChart::setMaxSize(double percent) {
    maxSize_ = std::max(0.0, std::min(100.0, percent));
}

void SvgFunnelChart::setOrient(const std::string& orient) {
    orient_ = orient;
}

SvgFunnelChart::FunnelLayout SvgFunnelChart::computeLayout(
    const ChartRect& viewRect) const {
    FunnelLayout layout;
    int count = static_cast<int>(data_.size());
    if (count == 0) return layout;

    bool isHorizontal = (orient_ == "horizontal");

    // 排序数据索引
    std::vector<int> indices(count);
    for (int i = 0; i < count; ++i) indices[i] = i;

    if (sort_ == "ascending") {
        std::sort(indices.begin(), indices.end(), [this](int a, int b) {
            return data_[a].value < data_[b].value;
        });
    } else if (sort_ == "descending") {
        std::sort(indices.begin(), indices.end(), [this](int a, int b) {
            return data_[a].value > data_[b].value;
        });
    }

    // 计算数据范围
    double minVal = data_[0].value;
    double maxVal = data_[0].value;
    for (const auto& d : data_) {
        minVal = std::min(minVal, d.value);
        maxVal = std::max(maxVal, d.value);
    }

    double viewWidth = viewRect.width;
    double viewHeight = viewRect.height;
    double viewSize = isHorizontal ? viewWidth : viewHeight;
    double itemSize = (viewSize - gap_ * (count - 1)) / count;

    double minSizePx = (isHorizontal ? viewHeight : viewWidth) * minSize_ / 100.0;
    double maxSizePx = (isHorizontal ? viewHeight : viewWidth) * maxSize_ / 100.0;

    double x = viewRect.x;
    double y = viewRect.y;

    // 来自 ECharts funnelLayout: linearMap
    auto linearMap = [](double val, double dMin, double dMax, double rMin, double rMax) {
        if (dMax == dMin) return (rMin + rMax) / 2.0;
        return rMin + (val - dMin) / (dMax - dMin) * (rMax - rMin);
    };

    // 获取每个 item 的端点线 [point1, point2]
    auto getLinePoints = [&](int idx, double offset) -> std::pair<SvgPoint, SvgPoint> {
        double val = data_[idx].value;
        double itemLen = linearMap(val, minVal, maxVal, minSizePx, maxSizePx);

        if (isHorizontal) {
            double y0;
            if (funnelAlign_ == "top") {
                y0 = y;
            } else if (funnelAlign_ == "center") {
                y0 = y + (viewHeight - itemLen) / 2.0;
            } else if (funnelAlign_ == "right" || funnelAlign_ == "bottom") {
                y0 = y + (viewHeight - itemLen);
            } else {
                y0 = y + (viewHeight - itemLen) / 2.0;
            }
            return {SvgPoint(offset, y0), SvgPoint(offset, y0 + itemLen)};
        } else {
            double x0;
            if (funnelAlign_ == "left") {
                x0 = x;
            } else if (funnelAlign_ == "center") {
                x0 = x + (viewWidth - itemLen) / 2.0;
            } else if (funnelAlign_ == "right") {
                x0 = x + viewWidth - itemLen;
            } else {
                x0 = x + (viewWidth - itemLen) / 2.0;
            }
            return {SvgPoint(x0, offset), SvgPoint(x0 + itemLen, offset)};
        }
    };

    // 构建每个 item 的四个顶点
    // 来自 ECharts funnelLayout: start.concat(end.slice().reverse())
    layout.points.resize(count);
    for (int i = 0; i < count; ++i) {
        int idx = indices[i];
        int nextIdx = (i + 1 < count) ? indices[i + 1] : -1;

        if (isHorizontal) {
            auto start = getLinePoints(idx, x);
            double segWidth = itemSize;
            std::pair<SvgPoint, SvgPoint> end;
            if (nextIdx >= 0) {
                end = getLinePoints(nextIdx, x + segWidth);
            } else {
                // 最后一个 item：底部与顶部同宽
                double itemLen = linearMap(data_[idx].value, minVal, maxVal, minSizePx, maxSizePx);
                double y0;
                if (funnelAlign_ == "top") {
                    y0 = y;
                } else if (funnelAlign_ == "center") {
                    y0 = y + (viewHeight - itemLen) / 2.0;
                } else if (funnelAlign_ == "right" || funnelAlign_ == "bottom") {
                    y0 = y + (viewHeight - itemLen);
                } else {
                    y0 = y + (viewHeight - itemLen) / 2.0;
                }
                end = {SvgPoint(x + segWidth, y0), SvgPoint(x + segWidth, y0 + itemLen)};
            }

            layout.points[idx] = {
                start.first,
                start.second,
                end.second,
                end.first
            };
            x += segWidth + gap_;
        } else {
            auto start = getLinePoints(idx, y);
            double segHeight = itemSize;
            std::pair<SvgPoint, SvgPoint> end;
            if (nextIdx >= 0) {
                end = getLinePoints(nextIdx, y + segHeight);
            } else {
                // 最后一个 item：底部与顶部同宽
                double itemLen = linearMap(data_[idx].value, minVal, maxVal, minSizePx, maxSizePx);
                double x0;
                if (funnelAlign_ == "left") {
                    x0 = x;
                } else if (funnelAlign_ == "center") {
                    x0 = x + (viewWidth - itemLen) / 2.0;
                } else if (funnelAlign_ == "right") {
                    x0 = x + viewWidth - itemLen;
                } else {
                    x0 = x + (viewWidth - itemLen) / 2.0;
                }
                end = {SvgPoint(x0, y + segHeight), SvgPoint(x0 + itemLen, y + segHeight)};
            }

            layout.points[idx] = {
                start.first,
                start.second,
                end.second,
                end.first
            };
            y += segHeight + gap_;
        }
    }

    return layout;
}

std::string SvgFunnelChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 漏斗图使用居中布局
    double margin = 60;
    double left = margin;
    double top = margin;
    if (!title_.empty()) top = kMarginTop + 10;

    double w = width_ - 2 * margin;
    double h = height_ - top - margin;
    ChartRect viewRect(left, top, w, h);

    auto layout = computeLayout(viewRect);

    if (layout.points.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 绘制每个漏斗段
    for (size_t i = 0; i < data_.size(); ++i) {
        const auto& pts = layout.points[i];
        if (pts.size() < 4) continue;

        // 多边形路径
        std::ostringstream polyD;
        polyD << "M" << pts[0].x << "," << pts[0].y << " ";
        for (size_t j = 1; j < pts.size(); ++j) {
            polyD << "L" << pts[j].x << "," << pts[j].y << " ";
        }
        polyD << "Z";

        oss << "<path d=\"" << polyD.str()
            << "\" fill=\"" << data_[i].color.toHex()
            << "\" fill-opacity=\"0.85\""
            << " stroke=\"#fff\" stroke-width=\"2\""
            << " stroke-linejoin=\"round\"/>\n";

        // 计算标签位置（中心）
        double cx = 0, cy = 0;
        for (const auto& p : pts) {
            cx += p.x;
            cy += p.y;
        }
        cx /= pts.size();
        cy /= pts.size();

        // 标签：名称 + 值
        oss << svgText(cx, cy - 4, data_[i].name, "middle", "#fff", 13);
        oss << svgText(cx, cy + 14, fmt(data_[i].value, 0),
                       "middle", "#fff", 12);
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgHorizontalBarChart 实现
// 核心算法来自 ECharts BarView.ts (isHorizontal 模式)
// ============================================================

SvgHorizontalBarChart::SvgHorizontalBarChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgHorizontalBarChart::addSeries(const std::vector<double>& data,
                                      const std::string& name,
                                      const SvgColor& color) {
    Series s;
    s.data = data;
    s.name = name;
    s.usePerBarColor = false;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

void SvgHorizontalBarChart::addSeries(const std::vector<double>& data,
                                      const std::vector<SvgColor>& colors,
                                      const std::string& name) {
    Series s;
    s.data = data;
    s.name = name;
    s.usePerBarColor = true;
    s.colors = colors;
    s.color = palette_[paletteIdx_ % palette_.size()];
    paletteIdx_++;
    series_.push_back(s);
}

void SvgHorizontalBarChart::setYLabels(const std::vector<std::string>& labels) {
    yLabels_ = labels;
}

std::string SvgHorizontalBarChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double xMin = 0;
    double xMax = std::numeric_limits<double>::lowest();
    int maxLen = 0;

    for (const auto& s : series_) {
        maxLen = std::max(maxLen, static_cast<int>(s.data.size()));
        for (double v : s.data) {
            if (std::isfinite(v)) {
                xMax = std::max(xMax, v);
                xMin = std::min(xMin, v);
            }
        }
    }

    if (xMax <= 0) xMax = 1;
    double xPadding = (xMax - xMin) * 0.1;
    xMax += xPadding;
    if (xMin >= 0) {
        xMin = 0;
    } else {
        xMin -= xPadding;
    }

    LinearScale xScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);

    // 每个分类的高度
    double categoryHeight = gridRect_.height / std::max(1, maxLen);

    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    // X 轴标签
    int xTicks = 5;
    std::vector<std::string> xLabels;
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        xLabels.push_back(fmt(val, 0));
    }

    std::vector<std::string> yLabels = yLabels_;
    if (yLabels.empty()) {
        for (int i = 0; i < maxLen; ++i) {
            yLabels.push_back(std::to_string(i + 1));
        }
    }

    // 绘制坐标轴
    oss << svgLine(gx, gy, gx, gy + gh, "#333", 1);       // Y 轴
    oss << svgLine(gx, gy + gh, gx + gw, gy + gh, "#333", 1); // X 轴

    // X 轴刻度和背景竖线
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        double x = xScale.map(val);
        // 背景竖线
        oss << svgLine(x, gy, x, gy + gh, "#e0e0e0", 1);
        // 刻度
        oss << svgLine(x, gy + gh, x, gy + gh + 4, "#999", 1);
        oss << svgText(x, gy + gh + 20, xLabels[i], "middle", "#666", 11);
    }

    // Y 轴标签（类别居中）
    for (int i = 0; i < maxLen; ++i) {
        double cy = gy + (i + 0.5) * categoryHeight;
        if (i < static_cast<int>(yLabels.size())) {
            oss << "<text x=\"" << (gx - 8)
                << "\" y=\"" << (cy + 4)
                << "\" text-anchor=\"end\""
                << " fill=\"#666\" font-size=\"11\""
                << " font-family=\"" << fontFamily_ << "\">"
                << escapeXml(yLabels[i]) << "</text>\n";
        }
    }

    // 添加裁剪区域
    std::string clipId = "hbarClip";
    oss << "<defs>\n";
    oss << "<clipPath id=\"" << clipId << "\">\n";
    oss << "<rect x=\"" << gx << "\" y=\"" << gy
        << "\" width=\"" << gw << "\" height=\"" << gh << "\"/>\n";
    oss << "</clipPath>\n";
    oss << "</defs>\n";
    oss << "<g clip-path=\"url(#" << clipId << ")\">\n";

    // 柱子组高度 = 分类高度的 60%
    double barGroupHeight = categoryHeight * 0.6;
    double barHeight = barGroupHeight / std::max(1, static_cast<int>(series_.size()));
    double barGap = 0;

    double zeroX = xScale.map(0);

    // 绘制每组柱子
    for (int i = 0; i < maxLen; ++i) {
        double centerY = gy + (i + 0.5) * categoryHeight;
        double startY = centerY - barGroupHeight / 2.0;

        for (size_t si = 0; si < series_.size(); ++si) {
            if (i >= static_cast<int>(series_[si].data.size())) continue;
            double val = series_[si].data[i];
            if (!std::isfinite(val)) continue;

            double valX = xScale.map(val);
            double y = startY + si * (barHeight + barGap);
            double x = std::min(valX, zeroX);
            double w = std::abs(valX - zeroX);

            SvgColor barColor = series_[si].color;
            if (series_[si].usePerBarColor &&
                i < static_cast<int>(series_[si].colors.size())) {
                barColor = series_[si].colors[i];
            }
            oss << svgRect(x, y, w, barHeight, barColor.toHex(), 2);
        }
    }

    oss << "</g>\n";

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgDensityChart 实现
// 核密度估计曲线，用于对比多组数据的分布形态
// 不填充（适配 PDF 输出），多组数据叠加对比
// ============================================================

SvgDensityChart::SvgDensityChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgDensityChart::addSeries(const std::vector<double>& data,
                                const std::string& name,
                                const SvgColor& color) {
    Series s;
    s.kde = KernelDensity(data);
    s.name = name;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        s.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        s.color = color;
    }
    series_.push_back(s);
}

void SvgDensityChart::setShowRug(bool show) {
    showRug_ = show;
}

std::string SvgDensityChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (series_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // ============================================================
    // Step 1: Find global x range and y range across all series
    // ============================================================
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMax = 0.0;

    for (auto& s : series_) {
        xMin = std::min(xMin, s.kde.dataMin() - s.kde.bandwidth());
        xMax = std::max(xMax, s.kde.dataMax() + s.kde.bandwidth());
    }

    // Generate curves and compute yMax
    std::vector<std::vector<SvgPoint>> allCurves;
    for (auto& s : series_) {
        auto curve = s.kde.generateCurve(200);
        std::vector<SvgPoint> points;
        for (auto& [x, y] : curve) {
            points.push_back(SvgPoint(x, y));
            yMax = std::max(yMax, y);
        }
        allCurves.push_back(points);
    }

    double yPadding = yMax * 0.1;
    yMax += yPadding;

    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    // ============================================================
    // Step 2: Set up scales
    // ============================================================
    LinearScale xScale, yScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gx, gx + gw);
    yScale.setDomain(0, yMax);
    yScale.setRange(gy + gh, gy);

    // ============================================================
    // Step 3: Draw axes
    // ============================================================
    oss << svgLine(gx, gy + gh, gx + gw, gy + gh, "#333", 1); // X axis
    oss << svgLine(gx, gy, gx, gy + gh, "#333", 1);            // Y axis

    // Y-axis ticks and labels
    int yTicks = 5;
    // Density values are often tiny (e.g. 0.002); fixed 2-decimal labels would
    // collapse into identical "0.01" ticks, so widen precision until the tick
    // step becomes representable.
    int yDec = 2;
    double yStep = yMax / yTicks;
    while (yDec < 6 && yStep * std::pow(10.0, yDec) < 1.0) {
        yDec++;
    }
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMax * i / yTicks;
        double y = yScale.map(val);
        oss << svgLine(gx, y, gx + gw, y, "#e8e8e8", 1); // grid line
        oss << svgLine(gx - 4, y, gx, y, "#999", 1);      // tick
        oss << svgText(gx - 8, y + 4, fmt(val, yDec), "end", "#666", 11);
    }

    // X-axis ticks and labels
    int xTicks = 5;
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        double x = xScale.map(val);
        oss << svgLine(x, gy + gh, x, gy + gh + 4, "#999", 1);
        oss << svgText(x, gy + gh + 18, fmt(val, 1), "middle", "#666", 11);
    }

    // ============================================================
    // Step 4: Draw curves (no fill, PDF-safe)
    // ============================================================
    // Build smooth paths using the same Catmull-Rom algorithm as SvgSmoothLineChart
    for (size_t si = 0; si < series_.size(); ++si) {
        auto& s = series_[si];
        auto& curve = allCurves[si];

        if (curve.size() < 2) continue;

        // Transform curve points to SVG coordinates
        std::vector<SvgPoint> svgPts;
        for (auto& pt : curve) {
            svgPts.push_back(SvgPoint(xScale.map(pt.x), yScale.map(pt.y)));
        }

        // Build smooth path (Catmull-Rom style)
        std::ostringstream pathD;
        double smooth = 0.5;
        pathD << "M" << svgPts[0].x << "," << svgPts[0].y;

        double cpx0 = svgPts[0].x;
        double cpy0 = svgPts[0].y;

        for (size_t i = 1; i < svgPts.size(); ++i) {
            double prevX = svgPts[i - 1].x;
            double prevY = svgPts[i - 1].y;
            double x = svgPts[i].x;
            double y = svgPts[i].y;

            double dx = x - prevX;
            double dy = y - prevY;
            if ((dx * dx + dy * dy) < 0.5) continue;

            bool hasNext = (i + 1 < svgPts.size());
            double nextX = hasNext ? svgPts[i + 1].x : x;
            double nextY = hasNext ? svgPts[i + 1].y : y;

            double cpx1, cpy1, nextCpx0, nextCpy0;

            if (!hasNext) {
                cpx1 = x; cpy1 = y;
                nextCpx0 = x; nextCpy0 = y;
            } else {
                double vx = nextX - prevX;
                double vy = nextY - prevY;
                double dx0 = x - prevX, dx1 = nextX - x;
                double dy0 = y - prevY, dy1 = nextY - y;
                double lenPrevSeg = std::sqrt(dx0 * dx0 + dy0 * dy0);
                double lenNextSeg = std::sqrt(dx1 * dx1 + dy1 * dy1);
                double ratioNextSeg = lenNextSeg / (lenNextSeg + lenPrevSeg);

                cpx1 = x - vx * smooth * (1.0 - ratioNextSeg);
                cpy1 = y - vy * smooth * (1.0 - ratioNextSeg);
                nextCpx0 = x + vx * smooth * ratioNextSeg;
                nextCpy0 = y + vy * smooth * ratioNextSeg;

                nextCpx0 = std::min(nextCpx0, std::max(nextX, x));
                nextCpy0 = std::min(nextCpy0, std::max(nextY, y));
                nextCpx0 = std::max(nextCpx0, std::min(nextX, x));
                nextCpy0 = std::max(nextCpy0, std::min(nextY, y));

                vx = nextCpx0 - x; vy = nextCpy0 - y;
                cpx1 = x - vx * lenPrevSeg / lenNextSeg;
                cpy1 = y - vy * lenPrevSeg / lenNextSeg;
                cpx1 = std::min(cpx1, std::max(prevX, x));
                cpy1 = std::min(cpy1, std::max(prevY, y));
                cpx1 = std::max(cpx1, std::min(prevX, x));
                cpy1 = std::max(cpy1, std::min(prevY, y));

                vx = x - cpx1; vy = y - cpy1;
                nextCpx0 = x + vx * lenNextSeg / lenPrevSeg;
                nextCpy0 = y + vy * lenNextSeg / lenPrevSeg;
            }

            pathD << "C" << cpx0 << "," << cpy0 << " "
                  << cpx1 << "," << cpy1 << " "
                  << x << "," << y;
            cpx0 = nextCpx0;
            cpy0 = nextCpy0;
        }

        double lineWidth = 2.5;
        oss << svgPath(pathD.str(), s.color.toHex(), lineWidth, "none");

        // Legend (top-right)
        double legendX = gx + gw - 120;
        double legendY = gy - 30 + si * 18;
        oss << svgLine(legendX, legendY + 4, legendX + 25, legendY + 4,
                       s.color.toHex(), lineWidth);
        oss << svgText(legendX + 30, legendY + 8, s.name, "start", "#333", 12);
    }

    // ============================================================
    // Step 5: Rug plot (optional)
    // ============================================================
    if (showRug_) {
        double rugBase = gy + gh;
        double rugHeight = 15;
        for (size_t si = 0; si < series_.size(); ++si) {
            auto& s = series_[si];
            const auto& data = s.kde.data();
            // Offset each series' rug slightly to avoid overlap
            double rugY = rugBase + si * 3;
            for (double xi : data) {
                double rx = xScale.map(xi);
                oss << svgLine(rx, rugY, rx, rugY + rugHeight,
                               s.color.toHex(), 1);
            }
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgNegativeBarChart 实现
// 交错正负轴标签 - 算法来自 ECharts bar-negative2 示例
// 特征：水平条形图、X轴在上方、虚线分割线、标签显示在柱子上
// ============================================================

SvgNegativeBarChart::SvgNegativeBarChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgNegativeBarChart::addData(double value, const std::string& label,
                                   const SvgColor& color) {
    DataItem item;
    item.value = value;
    item.label = label;
    if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 1.0) {
        item.color = palette_[paletteIdx_ % palette_.size()];
        paletteIdx_++;
    } else {
        item.color = color;
    }
    data_.push_back(item);
}

std::string SvgNegativeBarChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    // 计算数据范围
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    for (const auto& item : data_) {
        if (std::isfinite(item.value)) {
            xMax = std::max(xMax, item.value);
            xMin = std::min(xMin, item.value);
        }
    }
    if (xMax <= 0) xMax = 1;
    if (xMin >= 0) xMin = -1;
    double xPadding = (xMax - xMin) * 0.1;
    xMax += xPadding;
    xMin -= xPadding;

    double gx = gridRect_.x + 60;  // 给标签留空间
    double gy = gridRect_.y + 20;  // X轴在上方
    double gw = gridRect_.width - 60;
    double gh = gridRect_.height - 20;

    LinearScale xScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gx, gx + gw);

    double zeroX = xScale.map(0);

    // 每个分类的高度
    int n = static_cast<int>(data_.size());
    double categoryHeight = gh / std::max(1, n);
    double barHeight = categoryHeight * 0.6;

    // X 轴刻度（在上方）
    int xTicks = 5;
    std::vector<std::string> xLabels;
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        xLabels.push_back(fmt(val, 1));
    }

    // X 轴（顶部，虚线分割线）
    // 零线
    oss << svgLine(zeroX, gy, zeroX, gy + gh, "#333", 1.5);

    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        double x = xScale.map(val);
        // 虚线背景
        oss << "<line x1=\"" << x << "\" y1=\"" << gy
            << "\" x2=\"" << x << "\" y2=\"" << (gy + gh)
            << "\" stroke=\"#e0e0e0\" stroke-width=\"1\""
            << " stroke-dasharray=\"4,4\"/>\n";
        // 刻度
        oss << svgLine(x, gy - 4, x, gy, "#999", 1);
        oss << svgText(x, gy - 10, xLabels[i], "middle", "#666", 11);
    }

    // 裁剪区域
    std::string clipId = "negBarClip";
    oss << "<defs>\n";
    oss << "<clipPath id=\"" << clipId << "\">\n";
    oss << "<rect x=\"" << gx << "\" y=\"" << gy
        << "\" width=\"" << gw << "\" height=\"" << gh << "\"/>\n";
    oss << "</clipPath>\n";
    oss << "</defs>\n";
    oss << "<g clip-path=\"url(#" << clipId << ")\">\n";

    // 绘制柱子
    for (int i = 0; i < n; ++i) {
        const auto& item = data_[i];
        if (!std::isfinite(item.value)) continue;

        double valX = xScale.map(item.value);
        double centerY = gy + (i + 0.5) * categoryHeight;
        double barY = centerY - barHeight / 2.0;

        double x = std::min(valX, zeroX);
        double w = std::abs(valX - zeroX);

        oss << svgRect(x, barY, w, barHeight, item.color.toHex(), 2);

        // 标签：正值在左侧（柱子左端），负值在右侧（柱子右端）
        std::string labelAnchor = (item.value >= 0) ? "end" : "start";
        double labelX = (item.value >= 0) ? valX - 5 : valX + 5;
        double labelY = centerY + 4;

        oss << "<text x=\"" << labelX << "\" y=\"" << labelY
            << "\" text-anchor=\"" << labelAnchor
            << "\" fill=\"#333\" font-size=\"12\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(item.label) << "</text>\n";
    }

    oss << "</g>\n";

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgCandlestickChart 实现
// 核心算法来自 ECharts candlestickLayout.ts / CandlestickView.ts
// ============================================================

SvgCandlestickChart::SvgCandlestickChart(int width, int height)
    : SvgChart(width, height) {}

void SvgCandlestickChart::addData(const std::vector<CandlestickData>& data,
                                  const std::string& /*name*/) {
    data_ = data;
}

void SvgCandlestickChart::setXLabels(const std::vector<std::string>& labels) {
    xLabels_ = labels;
}

void SvgCandlestickChart::setColorUp(const SvgColor& color) {
    colorUp_ = color;
    borderUp_ = color;
}

void SvgCandlestickChart::setColorDown(const SvgColor& color) {
    colorDown_ = color;
    borderDown_ = color;
}

void SvgCandlestickChart::setCandleWidth(double width) {
    candleWidth_ = width;
}

void SvgCandlestickChart::setDojiColor(const SvgColor& color) {
    dojiColor_ = color;
}

int SvgCandlestickChart::getSign(size_t index) const {
    if (index >= data_.size()) return 0;
    const auto& item = data_[index];
    if (item.open > item.close) {
        return -1;  // 阴线
    } else if (item.open < item.close) {
        return 1;   // 阳线
    } else {
        // 十字星：比较前一根收盘价
        if (dojiColor_.r != 0 || dojiColor_.g != 0 || dojiColor_.b != 0) {
            return 0;  // 设置了十字星颜色，使用特殊颜色
        }
        if (index > 0 && data_[index - 1].close <= item.close) {
            return 1;
        }
        return 1;
    }
}

std::string SvgCandlestickChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    size_t n = data_.size();

    // 计算数据范围
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    for (const auto& item : data_) {
        if (std::isfinite(item.low))  yMin = std::min(yMin, item.low);
        if (std::isfinite(item.high)) yMax = std::max(yMax, item.high);
    }

    if (yMin == yMax) { yMin -= 1; yMax += 1; }
    // K 线图不强制 Y 轴从 0 开始，只加 padding 显示价格波动区间
    double yPadding = (yMax - yMin) * 0.1;
    yMax += yPadding;
    yMin -= yPadding;

    double gx = gridRect_.x;
    double gy = gridRect_.y;
    double gw = gridRect_.width;
    double gh = gridRect_.height;

    // Y 轴比例尺
    LinearScale yScale;
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gy + gh, gy);

    // X 轴比例尺：每个数据点占一个分类位置
    LinearScale xScale;
    xScale.setDomain(0, n > 1 ? static_cast<double>(n - 1) : 1.0);
    xScale.setRange(gx, gx + gw);

    // 蜡烛宽度
    double categoryWidth = gw / std::max(static_cast<size_t>(1), n);
    double cw = candleWidth_;
    if (cw <= 0) {
        cw = std::min(categoryWidth * 0.6, 20.0);  // 自动宽度：60% 分类宽度，最大 20px
    }
    double halfCw = cw / 2.0;

    // Y 轴标签
    int yTicks = 5;
    std::vector<std::string> yLabels;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }

    std::vector<std::string> xLabels = xLabels_;
    if (xLabels.empty()) {
        for (size_t i = 0; i < n; ++i) {
            xLabels.push_back(std::to_string(i + 1));
        }
    }

    // 绘制坐标轴
    oss << svgLine(gx, gy, gx, gy + gh, "#333", 1);           // Y 轴
    oss << svgLine(gx, gy + gh, gx + gw, gy + gh, "#333", 1); // X 轴

    // Y 轴刻度及背景横线
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        double y = yScale.map(val);
        oss << svgLine(gx, y, gx + gw, y, "#e0e0e0", 1);
        oss << svgLine(gx - 4, y, gx, y, "#999", 1);
        oss << svgText(gx - 8, y + 4, yLabels[i], "end", "#666", 11);
    }

    // X 轴刻度（每隔一定间距显示）
    int xLabelStep = std::max(1, static_cast<int>(n) / 10);
    for (size_t i = 0; i < n; ++i) {
        double cx = xScale.map(static_cast<double>(i));
        bool showLabel = (i % xLabelStep == 0) || (i == n - 1);
        oss << svgLine(cx, gy + gh, cx, gy + gh + (showLabel ? 4 : 2), "#999", 1);
        if (showLabel && i < xLabels.size()) {
            oss << svgText(cx, gy + gh + 18, xLabels[i], "middle", "#666", 10);
        }
    }

    // 裁剪区域
    std::string clipId = "candleClip";
    oss << "<defs>\n";
    oss << "<clipPath id=\"" << clipId << "\">\n";
    oss << "<rect x=\"" << gx << "\" y=\"" << gy
        << "\" width=\"" << gw << "\" height=\"" << gh << "\"/>\n";
    oss << "</clipPath>\n";
    oss << "</defs>\n";
    oss << "<g clip-path=\"url(#" << clipId << ")\">\n";

    // 绘制每根 K 线
    for (size_t i = 0; i < n; ++i) {
        const auto& item = data_[i];
        if (!std::isfinite(item.open) || !std::isfinite(item.close) ||
            !std::isfinite(item.low) || !std::isfinite(item.high)) {
            continue;
        }

        double cx = xScale.map(static_cast<double>(i));
        int sign = getSign(i);

        // 阳线/阴线/十字星的颜色
        SvgColor fillColor, strokeColor;
        if (sign == 0) {
            fillColor = dojiColor_;
            strokeColor = dojiColor_;
        } else if (sign > 0) {
            fillColor = colorUp_;
            strokeColor = borderUp_;
        } else {
            fillColor = colorDown_;
            strokeColor = borderDown_;
        }

        // 实体上下边界
        double ocHigh = std::max(item.open, item.close);
        double ocLow  = std::min(item.open, item.close);

        double yHigh = yScale.map(ocHigh);
        double yLow  = yScale.map(ocLow);
        double yTop  = yScale.map(item.high);
        double yBottom = yScale.map(item.low);

        double bodyH = yLow - yHigh;

        // 绘制影线（上影线 + 下影线）
        oss << svgLine(cx, yTop, cx, yHigh, strokeColor.toHex(), 1);
        oss << svgLine(cx, yLow, cx, yBottom, strokeColor.toHex(), 1);

        // 绘制实体
        if (bodyH < 0.5) {
            // 十字星：实体太窄，画一条横线
            oss << svgLine(cx - halfCw, yHigh, cx + halfCw, yHigh, strokeColor.toHex(), 1);
        } else {
            std::string fill = (sign > 0) ? fillColor.toHex() : fillColor.toHex();
            oss << svgRect(cx - halfCw, yHigh, cw, bodyH, fill, 0);
            // 边框
            oss << "<rect x=\"" << (cx - halfCw) << "\" y=\"" << yHigh
                << "\" width=\"" << cw << "\" height=\"" << bodyH
                << "\" fill=\"none\" stroke=\"" << strokeColor.toHex()
                << "\" stroke-width=\"1\"/>\n";
        }
    }

    oss << "</g>\n";

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgJohariWindowChart 实现 - 乔哈里视窗 (2x2 象限图)
// ============================================================

SvgJohariWindowChart::SvgJohariWindowChart(int width, int height)
    : SvgChart(width, height) {}

void SvgJohariWindowChart::setQuadrant(Quadrant q, const std::string& title,
                                       const std::string& content,
                                       const SvgColor& bgColor,
                                       const SvgColor& titleColor,
                                       const SvgColor& contentColor,
                                       int titleFontSize, int contentFontSize) {
    if (q < 0 || q > 3) return;
    quadrants_[q].title = title;
    quadrants_[q].content = content;
    if (bgColor.r != 0 || bgColor.g != 0 || bgColor.b != 0 || bgColor.a != 1.0) {
        quadrants_[q].bgColor = bgColor;
    }
    if (titleColor.r != 0 || titleColor.g != 0 || titleColor.b != 0 || titleColor.a != 1.0) {
        quadrants_[q].titleColor = titleColor;
    }
    if (contentColor.r != 0 || contentColor.g != 0 || contentColor.b != 0 || contentColor.a != 1.0) {
        quadrants_[q].contentColor = contentColor;
    }
    quadrants_[q].titleFontSize = titleFontSize;
    quadrants_[q].contentFontSize = contentFontSize;
}

void SvgJohariWindowChart::setXLabels(const std::string& left, const std::string& right) {
    xLabelLeft_ = left;
    xLabelRight_ = right;
}

void SvgJohariWindowChart::setYLabels(const std::string& top, const std::string& bottom) {
    yLabelTop_ = top;
    yLabelBottom_ = bottom;
}

void SvgJohariWindowChart::setCellBorder(const std::string& color, double width) {
    cellBorderColor_ = color;
    cellBorderWidth_ = width;
}

void SvgJohariWindowChart::setCellBorderRadius(double radius) {
    cellBorderRadius_ = std::max(0.0, radius);
}

void SvgJohariWindowChart::setContentOffset(double offset) {
    contentOffset_ = std::max(0.0, offset);
}

void SvgJohariWindowChart::setTitleAlign(const std::string& align) {
    titleAlign_ = align;
}

void SvgJohariWindowChart::setShowBackground(bool show) {
    showBackground_ = show;
}

std::string SvgJohariWindowChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    // 布局
    double marginLeft = 120;
    double marginRight = 40;
    double marginTop = 60;
    double marginBottom = 60;

    double gridW = width_ - marginLeft - marginRight;
    double gridH = height_ - marginTop - marginBottom;
    double cellW = gridW / 2.0;
    double cellH = gridH / 2.0;

    double gridX = marginLeft;
    double gridY = marginTop;

    // Y 轴标签（左侧，旋转90度）
    for (int r = 0; r < 2; ++r) {
        const std::string& label = (r == 0) ? yLabelTop_ : yLabelBottom_;
        double cy = gridY + cellH * r + cellH / 2.0 + 4;
        double lx = gridX - 10;
        oss << "<text x=\"" << lx << "\" y=\"" << cy
            << "\" text-anchor=\"end\""
            << " fill=\"#333\" font-size=\"13\" font-weight=\"bold\""
            << " font-family=\"" << fontFamily_ << "\""
            << " transform=\"rotate(-90 " << lx << " " << cy << ")\">"
            << escapeXml(label) << "</text>\n";
    }

    // X 轴标签（底部）
    for (int c = 0; c < 2; ++c) {
        const std::string& label = (c == 0) ? xLabelLeft_ : xLabelRight_;
        double cx = gridX + cellW * c + cellW / 2.0;
        double ly = gridY + gridH + 25;
        oss << "<text x=\"" << cx << "\" y=\"" << ly
            << "\" text-anchor=\"middle\""
            << " fill=\"#333\" font-size=\"13\" font-weight=\"bold\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(label) << "</text>\n";
    }

    // 绘制 4 个象限
    double padding = 12;
    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 2; ++c) {
            int idx = r * 2 + c;
            const auto& qd = quadrants_[idx];
            double cx = gridX + cellW * c;
            double cy = gridY + cellH * r;

            // 背景矩形（仅在 showBackground_ 为 true 时绘制填充）
            // 边框始终绘制
            if (showBackground_) {
                std::string bg = qd.bgColor.toHex();
                if (cellBorderRadius_ > 0) {
                    oss << "<rect x=\"" << cx << "\" y=\"" << cy
                        << "\" width=\"" << cellW << "\" height=\"" << cellH
                        << "\" rx=\"" << cellBorderRadius_ << "\""
                        << " fill=\"" << bg << "\""
                        << " stroke=\"" << cellBorderColor_ << "\""
                        << " stroke-width=\"" << cellBorderWidth_ << "\"/>\n";
                } else {
                    oss << "<rect x=\"" << cx << "\" y=\"" << cy
                        << "\" width=\"" << cellW << "\" height=\"" << cellH
                        << "\" fill=\"" << bg << "\""
                        << " stroke=\"" << cellBorderColor_ << "\""
                        << " stroke-width=\"" << cellBorderWidth_ << "\"/>\n";
                }
            } else {
                if (cellBorderRadius_ > 0) {
                    oss << "<rect x=\"" << cx << "\" y=\"" << cy
                        << "\" width=\"" << cellW << "\" height=\"" << cellH
                        << "\" rx=\"" << cellBorderRadius_ << "\""
                        << " fill=\"none\""
                        << " stroke=\"" << cellBorderColor_ << "\""
                        << " stroke-width=\"" << cellBorderWidth_ << "\"/>\n";
                } else {
                    oss << "<rect x=\"" << cx << "\" y=\"" << cy
                        << "\" width=\"" << cellW << "\" height=\"" << cellH
                        << " fill=\"none\""
                        << " stroke=\"" << cellBorderColor_ << "\""
                        << " stroke-width=\"" << cellBorderWidth_ << "\"/>\n";
                }
            }

            if (qd.title.empty() && qd.content.empty()) continue;

            bool hasContent = !qd.content.empty();
            bool isCenter = (titleAlign_ == "center");

            double titleX;
            std::string anchor;
            if (isCenter) {
                titleX = cx + cellW / 2.0;
                anchor = "middle";
            } else {
                titleX = cx + padding;
                anchor = "start";
            }
            double titleY = cy + padding + qd.titleFontSize;

            if (hasContent) {
                // 标题
                oss << "<text x=\"" << titleX << "\" y=\"" << titleY
                    << "\" text-anchor=\"" << anchor << "\""
                    << " fill=\"" << qd.titleColor.toHex() << "\""
                    << " font-size=\"" << qd.titleFontSize << "\""
                    << " font-weight=\"bold\""
                    << " font-family=\"" << fontFamily_ << "\">"
                    << escapeXml(qd.title) << "</text>\n";

                // 内容文本（多行，始终左对齐）
                double contentY = titleY + contentOffset_;
                double contentX = cx + padding;
                std::istringstream stream(qd.content);
                std::string line;
                while (std::getline(stream, line)) {
                    oss << "<text x=\"" << contentX << "\" y=\"" << contentY
                        << "\" text-anchor=\"start\""
                        << " fill=\"" << qd.contentColor.toHex() << "\""
                        << " font-size=\"" << qd.contentFontSize << "\""
                        << " font-family=\"" << fontFamily_ << "\">"
                        << escapeXml(line) << "</text>\n";
                    contentY += qd.contentFontSize + 4;
                }
            } else {
                // 只有标题
                double ty = cy + cellH / 2.0 + qd.titleFontSize / 3.0;
                oss << "<text x=\"" << titleX << "\" y=\"" << ty
                    << "\" text-anchor=\"" << anchor << "\""
                    << " fill=\"" << qd.titleColor.toHex() << "\""
                    << " font-size=\"" << qd.titleFontSize << "\""
                    << " font-weight=\"bold\""
                    << " font-family=\"" << fontFamily_ << "\">"
                    << escapeXml(qd.title) << "</text>\n";
            }
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgGridChart 实现 - 九宫格图
// ============================================================

SvgGridChart::SvgGridChart(int width, int height)
    : SvgChart(width, height) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            cells_[r][c].title = "";
            cells_[r][c].content = "";
            cells_[r][c].bgColor = SvgColor(255, 255, 255);
            cells_[r][c].titleColor = SvgColor(51, 51, 51);
            cells_[r][c].contentColor = SvgColor(102, 102, 102);
            cells_[r][c].titleFontSize = 12;
            cells_[r][c].contentFontSize = 10;
            cells_[r][c].borderColor = SvgColor(0, 0, 0, 0);  // 未设置
            cells_[r][c].borderWidth = -1;
            cells_[r][c].titleAlign = "";
            cells_[r][c].contentOffset = 10;
        }
    }
}

void SvgGridChart::setCell(int row, int col, const std::string& title,
                           const SvgColor& bgColor,
                           const SvgColor& titleColor,
                           int titleFontSize) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return;
    cells_[row][col].title = title;
    cells_[row][col].content = "";
    if (bgColor.r != 0 || bgColor.g != 0 || bgColor.b != 0 || bgColor.a != 1.0) {
        cells_[row][col].bgColor = bgColor;
    }
    if (titleColor.r != 0 || titleColor.g != 0 || titleColor.b != 0 || titleColor.a != 1.0) {
        cells_[row][col].titleColor = titleColor;
    }
    cells_[row][col].titleFontSize = titleFontSize;
}

void SvgGridChart::setCell(int row, int col, const std::string& title, const std::string& content,
                           const SvgColor& bgColor,
                           const SvgColor& titleColor,
                           const SvgColor& contentColor,
                           int titleFontSize, int contentFontSize) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return;
    cells_[row][col].title = title;
    cells_[row][col].content = content;
    if (bgColor.r != 0 || bgColor.g != 0 || bgColor.b != 0 || bgColor.a != 1.0) {
        cells_[row][col].bgColor = bgColor;
    }
    if (titleColor.r != 0 || titleColor.g != 0 || titleColor.b != 0 || titleColor.a != 1.0) {
        cells_[row][col].titleColor = titleColor;
    }
    if (contentColor.r != 0 || contentColor.g != 0 || contentColor.b != 0 || contentColor.a != 1.0) {
        cells_[row][col].contentColor = contentColor;
    }
    cells_[row][col].titleFontSize = titleFontSize;
    cells_[row][col].contentFontSize = contentFontSize;
}

void SvgGridChart::setCellBorder(int row, int col, const std::string& color, double width) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return;
    cells_[row][col].borderColor = SvgColor::fromHex(color);
    cells_[row][col].borderWidth = width;
}

void SvgGridChart::setCellTitleAlign(int row, int col, const std::string& align) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return;
    cells_[row][col].titleAlign = align;
}

void SvgGridChart::setCellContentOffset(int row, int col, double offset) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return;
    cells_[row][col].contentOffset = std::max(0.0, offset);
}

void SvgGridChart::setXLabels(const std::string& l0, const std::string& l1, const std::string& l2) {
    xLabels_[0] = l0;
    xLabels_[1] = l1;
    xLabels_[2] = l2;
    xLabelsSet_ = true;
}

void SvgGridChart::setYLabels(const std::string& l0, const std::string& l1, const std::string& l2) {
    yLabels_[0] = l0;
    yLabels_[1] = l1;
    yLabels_[2] = l2;
    yLabelsSet_ = true;
}

void SvgGridChart::setXLabel(const std::string& label) {
    xLabels_[0] = label;
    xLabels_[1] = "";
    xLabels_[2] = "";
    xLabelsSet_ = true;
}

void SvgGridChart::setYLabel(const std::string& label) {
    yLabels_[0] = label;
    yLabels_[1] = "";
    yLabels_[2] = "";
    yLabelsSet_ = true;
}

void SvgGridChart::setYLabelPosition(const std::string& pos) {
    yLabelPos_ = pos;
}

void SvgGridChart::setXLabelPosition(const std::string& pos) {
    xLabelPos_ = pos;
}

void SvgGridChart::setYLabelRotate(bool rotate) {
    yLabelRotate_ = rotate;
}

void SvgGridChart::setCellBorder(const std::string& color, double width) {
    cellBorderColor_ = color;
    cellBorderWidth_ = width;
}

void SvgGridChart::setCellBorderRadius(double radius) {
    cellBorderRadius_ = std::max(0.0, radius);
}

std::string SvgGridChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    // 布局计算
    double marginLeft = 120;
    double marginRight = 40;
    double marginTop = 60;
    double marginBottom = 60;

    bool hasYLabels = yLabelsSet_ && (!yLabels_[0].empty() || !yLabels_[1].empty() || !yLabels_[2].empty());
    bool hasXLabels = xLabelsSet_ && (!xLabels_[0].empty() || !xLabels_[1].empty() || !xLabels_[2].empty());

    if (yLabelPos_ == "right") {
        marginLeft = 40;
        marginRight = 120;
    }

    if (xLabelPos_ == "top") {
        marginTop = 80;
        marginBottom = 40;
    }

    double gridW = width_ - marginLeft - marginRight;
    double gridH = height_ - marginTop - marginBottom;
    double cellW = gridW / 3.0;
    double cellH = gridH / 3.0;

    double gridX = (yLabelPos_ == "right") ? 40 : marginLeft;
    double gridY = (xLabelPos_ == "top") ? marginTop : 60;

    // 绘制 Y 轴标签（左侧）
    if (hasYLabels) {
        double labelX = (yLabelPos_ == "right")
            ? gridX + gridW + 10
            : gridX - 10;
        std::string anchor = (yLabelPos_ == "right") ? "start" : "end";

        bool isSingleY = yLabels_[1].empty() && yLabels_[2].empty();

        if (isSingleY) {
            double cy = gridY + gridH / 2.0 + 4;
            oss << "<text x=\"" << labelX << "\" y=\"" << cy
                << "\" text-anchor=\"" << anchor
                << "\" fill=\"#333\" font-size=\"14\" font-weight=\"bold\""
                << " font-family=\"" << fontFamily_ << "\"";
            if (yLabelRotate_) {
                oss << " transform=\"rotate(" << ((yLabelPos_ == "right") ? 90 : -90)
                    << " " << labelX << " " << cy << ")\"";
            }
            oss << ">" << escapeXml(yLabels_[0]) << "</text>\n";
        } else {
            for (int i = 0; i < 3; ++i) {
                if (yLabels_[i].empty()) continue;
                double cy = gridY + cellH * i + cellH / 2.0 + 4;
                oss << "<text x=\"" << labelX << "\" y=\"" << cy
                    << "\" text-anchor=\"" << anchor
                    << "\" fill=\"#333\" font-size=\"13\""
                    << " font-family=\"" << fontFamily_ << "\"";
                if (yLabelRotate_) {
                    double rotDeg = (yLabelPos_ == "right") ? 90 : -90;
                    oss << " transform=\"rotate(" << rotDeg
                        << " " << labelX << " " << cy << ")\"";
                }
                oss << ">" << escapeXml(yLabels_[i]) << "</text>\n";
            }
        }
    }

    // 绘制 X 轴标签（底部）
    if (hasXLabels) {
        double labelY = (xLabelPos_ == "top")
            ? gridY - 10
            : gridY + gridH + 25;
        std::string anchor = "middle";

        bool isSingleX = xLabels_[1].empty() && xLabels_[2].empty();

        if (isSingleX) {
            double cx = gridX + gridW / 2.0;
            oss << "<text x=\"" << cx << "\" y=\"" << labelY
                << "\" text-anchor=\"" << anchor
                << "\" fill=\"#333\" font-size=\"14\" font-weight=\"bold\""
                << " font-family=\"" << fontFamily_ << "\">"
                << escapeXml(xLabels_[0]) << "</text>\n";
        } else {
            for (int i = 0; i < 3; ++i) {
                if (xLabels_[i].empty()) continue;
                double cx = gridX + cellW * i + cellW / 2.0;
                oss << "<text x=\"" << cx << "\" y=\"" << labelY
                    << "\" text-anchor=\"" << anchor
                    << "\" fill=\"#333\" font-size=\"13\""
                    << " font-family=\"" << fontFamily_ << "\">"
                    << escapeXml(xLabels_[i]) << "</text>\n";
            }
        }
    }

    // 绘制 3x3 格子
    double padding = 10;  // 文字内边距
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            double cx = gridX + cellW * c;
            double cy = gridY + cellH * r;

            const auto& cell = cells_[r][c];
            std::string bg = cell.bgColor.toHex();

            // 边框：优先用格子独立边框，否则用全局边框
            std::string borderColor = cellBorderColor_;
            double borderWidth = cellBorderWidth_;
            if (cell.borderWidth >= 0) {
                borderColor = cell.borderColor.toHex();
                borderWidth = cell.borderWidth;
            }

            // 背景矩形
            if (cellBorderRadius_ > 0) {
                oss << "<rect x=\"" << cx << "\" y=\"" << cy
                    << "\" width=\"" << cellW << "\" height=\"" << cellH
                    << "\" rx=\"" << cellBorderRadius_ << "\""
                    << " fill=\"" << bg << "\""
                    << " stroke=\"" << borderColor << "\""
                    << " stroke-width=\"" << borderWidth << "\"/>\n";
            } else {
                oss << "<rect x=\"" << cx << "\" y=\"" << cy
                    << "\" width=\"" << cellW << "\" height=\"" << cellH
                    << "\" fill=\"" << bg << "\""
                    << " stroke=\"" << borderColor << "\""
                    << " stroke-width=\"" << borderWidth << "\"/>\n";
            }

            if (cell.title.empty() && cell.content.empty()) continue;

            bool hasContent = !cell.content.empty();

            // 确定标题对齐方式：优先使用格子设置，否则用默认（纯标题居中，有内容左对齐）
            std::string align = cell.titleAlign;
            std::string anchor;
            double titleX;
            if (align.empty()) {
                align = hasContent ? "start" : "center";
            }
            if (align == "center") {
                anchor = "middle";
                titleX = cx + cellW / 2.0;
            } else if (align == "end") {
                anchor = "end";
                titleX = cx + cellW - padding;
            } else {
                anchor = "start";
                titleX = cx + padding;
            }

            if (hasContent) {
                // 有标题+内容
                double titleY = cy + padding + cell.titleFontSize;
                oss << "<text x=\"" << titleX << "\" y=\"" << titleY
                    << "\" text-anchor=\"" << anchor << "\""
                    << " fill=\"" << cell.titleColor.toHex() << "\""
                    << " font-size=\"" << cell.titleFontSize << "\""
                    << " font-weight=\"bold\""
                    << " font-family=\"" << fontFamily_ << "\">"
                    << escapeXml(cell.title) << "</text>\n";

                // 内容文本（支持多行，用 \n 分隔）
                double contentY = titleY + cell.contentOffset;
                double contentX = cx + padding;  // 内容始终左对齐
                std::istringstream stream(cell.content);
                std::string line;
                while (std::getline(stream, line)) {
                    oss << "<text x=\"" << contentX << "\" y=\"" << contentY
                        << "\" text-anchor=\"start\""
                        << " fill=\"" << cell.contentColor.toHex() << "\""
                        << " font-size=\"" << cell.contentFontSize << "\""
                        << " font-family=\"" << fontFamily_ << "\">"
                        << escapeXml(line) << "</text>\n";
                    contentY += cell.contentFontSize + 4;
                }
            } else {
                // 只有标题
                double ty = cy + cellH / 2.0 + cell.titleFontSize / 3.0;
                oss << "<text x=\"" << titleX << "\" y=\"" << ty
                    << "\" text-anchor=\"" << anchor << "\""
                    << " fill=\"" << cell.titleColor.toHex() << "\""
                    << " font-size=\"" << cell.titleFontSize << "\""
                    << " font-weight=\"bold\""
                    << " font-family=\"" << fontFamily_ << "\">"
                    << escapeXml(cell.title) << "</text>\n";
            }
        }
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgRegressionChart 实现
// 散点图 + 回归曲线 + 公式标签
// 使用 LinearRegression 类进行回归计算
// ============================================================

SvgRegressionChart::SvgRegressionChart(int width, int height)
    : SvgChart(width, height) {}

void SvgRegressionChart::setData(const std::vector<std::vector<double>>& data) {
    data_ = data;
}

void SvgRegressionChart::setMethod(LinearRegression::Method method) {
    method_ = method;
}

void SvgRegressionChart::setPolynomialOrder(int order) {
    polynomialOrder_ = std::max(1, order);
}

void SvgRegressionChart::setPointColor(const SvgColor& color) {
    pointColor_ = color;
}

void SvgRegressionChart::setLineColor(const SvgColor& color) {
    lineColor_ = color;
}

void SvgRegressionChart::setPointSize(double size) {
    pointSize_ = std::max(1.0, size);
}

std::string SvgRegressionChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.size() < 2) {
        oss << svgFooter();
        return oss.str();
    }

    // 运行回归
    LinearRegression::Config config;
    config.method = method_;
    config.order = polynomialOrder_;
    auto regResult = LinearRegression::run(data_, config);

    bool isMultivariate = (method_ == LinearRegression::MULTIVARIATE_LINEAR);

    // 计算数据范围
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    if (isMultivariate) {
        // 多元回归：x = 实际值, y = 预测值
        for (const auto& p : regResult.points) {
            xMin = std::min(xMin, p[0]);
            xMax = std::max(xMax, p[0]);
            yMin = std::min(yMin, p[1]);
            yMax = std::max(yMax, p[1]);
        }
        // 统一 x 和 y 的范围，使对角线居中
        double bothMin = std::min(xMin, yMin);
        double bothMax = std::max(xMax, yMax);
        double bothPadding = (bothMax - bothMin) * 0.1;
        if (bothPadding < 1) bothPadding = 1;
        xMin = yMin = bothMin - bothPadding;
        xMax = yMax = bothMax + bothPadding;
    } else {
        // 单变量回归：x = 自变量, y = 因变量
        for (const auto& row : data_) {
            xMin = std::min(xMin, row[0]);
            xMax = std::max(xMax, row[0]);
            yMin = std::min(yMin, row[1]);
            yMax = std::max(yMax, row[1]);
        }
        if (xMin == xMax) { xMin -= 1; xMax += 1; }
        if (yMin == yMax) { yMin -= 1; yMax += 1; }
        double xPadding = (xMax - xMin) * 0.1;
        double yPadding = (yMax - yMin) * 0.1;
        xMin -= xPadding; xMax += xPadding;
        yMin -= yPadding; yMax += yPadding;
    }

    LinearScale xScale, yScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    // 轴标签
    std::vector<std::string> yLabels;
    int yTicks = 5;
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, 0));
    }
    std::vector<std::string> xLabels;
    int xTicks = 5;
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        xLabels.push_back(fmt(val, 0));
    }
    oss << drawAxes(xLabels, yLabels);

    if (isMultivariate) {
        // 多元回归：预测值 vs 实际值散点图
        // 绘制对角线（完美预测参考线）
        SvgPoint diagStart(xScale.map(xMin), yScale.map(yMin));
        SvgPoint diagEnd(xScale.map(xMax), yScale.map(yMax));
        oss << "<line x1=\"" << diagStart.x << "\" y1=\"" << diagStart.y
            << "\" x2=\"" << diagEnd.x << "\" y2=\"" << diagEnd.y
            << "\" stroke=\"#999\" stroke-width=\"1\" stroke-dasharray=\"5,5\"/>\n";

        // 绘制散点（实际值, 预测值）
        for (const auto& p : regResult.points) {
            double sx = xScale.map(p[0]); // 实际值
            double sy = yScale.map(p[1]); // 预测值
            oss << svgCircle(sx, sy, pointSize_, pointColor_.toHex());
        }

        // 计算 R²
        double ssTot = 0, ssRes = 0;
        double meanY = 0;
        for (const auto& p : regResult.points) meanY += p[0];
        meanY /= regResult.points.size();
        for (const auto& p : regResult.points) {
            ssTot += (p[0] - meanY) * (p[0] - meanY);
            ssRes += (p[0] - p[1]) * (p[0] - p[1]);
        }
        double rSquared = (ssTot > 0) ? 1.0 - ssRes / ssTot : 0;

        // 公式 + R² 标签
        double labelX = gridRect_.x + gridRect_.width - 10;
        double labelY = gridRect_.y + 20;
        oss << "<text x=\"" << labelX << "\" y=\"" << labelY
            << "\" text-anchor=\"end\""
            << " fill=\"" << lineColor_.toHex() << "\""
            << " font-size=\"13\" font-weight=\"bold\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(regResult.expression) << "</text>\n";
        labelY += 18;
        std::ostringstream r2Str;
        r2Str << "R² = " << std::fixed << std::setprecision(4) << rSquared;
        oss << "<text x=\"" << labelX << "\" y=\"" << labelY
            << "\" text-anchor=\"end\""
            << " fill=\"#666\" font-size=\"12\""
            << " font-family=\"" << fontFamily_ << "\">"
            << r2Str.str() << "</text>\n";
    } else {
        // 单变量回归：x-y 散点 + 回归曲线
        for (const auto& row : data_) {
            double sx = xScale.map(row[0]);
            double sy = yScale.map(row[1]);
            oss << svgCircle(sx, sy, pointSize_, pointColor_.toHex());
        }

        if (!regResult.points.empty()) {
            auto sorted = regResult.points;
            std::sort(sorted.begin(), sorted.end(),
                      [](const auto& a, const auto& b) { return a[0] < b[0]; });

            std::vector<SvgPoint> svgPts;
            for (const auto& p : sorted) {
                svgPts.push_back(SvgPoint(xScale.map(p[0]), yScale.map(p[1])));
            }
            oss << svgPolyline(svgPts, lineColor_.toHex(), 2.5, "none");
        }

        double labelX = gridRect_.x + gridRect_.width - 10;
        double labelY = gridRect_.y + 20;
        oss << "<text x=\"" << labelX << "\" y=\"" << labelY
            << "\" text-anchor=\"end\""
            << " fill=\"" << lineColor_.toHex() << "\""
            << " font-size=\"14\" font-weight=\"bold\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(regResult.expression) << "</text>\n";
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgHeatmapChart 实现
//    算法来自 ECharts HeatmapView.ts / HeatmapSeries.ts
//    使用 Cartesian 坐标系，X/Y 轴均为 category 类型
//    visualMap 将 value 映射到颜色渐变
// ============================================================

SvgHeatmapChart::SvgHeatmapChart(int width, int height)
    : SvgChart(width, height) {
    // 默认颜色渐变: 浅蓝 → #5070dd (ECharts gradientColor 默认值)
    gradientColors_ = {
        SvgColor(208, 220, 249),  // 浅蓝 (modifyHSL #5070dd, lightness 0.9)
        SvgColor(80, 112, 221)     // #5070dd (themeColor)
    };
}

void SvgHeatmapChart::addData(int x, int y, double value) {
    data_.push_back(HeatmapDataItem(x, y, value));
    if (!valueRangeSet_) {
        if (data_.size() == 1) {
            minVal_ = maxVal_ = value;
        } else {
            minVal_ = std::min(minVal_, value);
            maxVal_ = std::max(maxVal_, value);
        }
    }
}

void SvgHeatmapChart::addData(const std::vector<HeatmapDataItem>& data) {
    for (const auto& item : data) {
        addData(item.x, item.y, item.value);
    }
}

void SvgHeatmapChart::setXLabels(const std::vector<std::string>& labels) {
    xLabels_ = labels;
}

void SvgHeatmapChart::setYLabels(const std::vector<std::string>& labels) {
    yLabels_ = labels;
}

void SvgHeatmapChart::setValueRange(double minVal, double maxVal) {
    minVal_ = minVal;
    maxVal_ = maxVal;
    valueRangeSet_ = true;
}

void SvgHeatmapChart::setColorGradient(const std::vector<SvgColor>& colors) {
    if (colors.size() >= 2) {
        gradientColors_ = colors;
    }
}

void SvgHeatmapChart::setShowLabel(bool show) {
    showLabel_ = show;
}

void SvgHeatmapChart::setLabelFontSize(int size) {
    labelFontSize_ = size;
}

void SvgHeatmapChart::setCellGap(double gap) {
    cellGap_ = gap;
}

void SvgHeatmapChart::setBorderRadius(double radius) {
    borderRadius_ = radius;
}

void SvgHeatmapChart::setShowVisualMap(bool show) {
    showVisualMap_ = show;
}

SvgColor SvgHeatmapChart::valueToColor(double value) const {
    if (gradientColors_.empty()) return SvgColor(0, 0, 0);
    if (gradientColors_.size() == 1) return gradientColors_[0];

    double range = maxVal_ - minVal_;
    double t = (range == 0) ? 0 : (value - minVal_) / range;
    t = std::max(0.0, std::min(1.0, t));

    int n = static_cast<int>(gradientColors_.size()) - 1;
    double scaledT = t * n;
    int idx = static_cast<int>(scaledT);
    if (idx >= n) idx = n - 1;
    double frac = scaledT - idx;

    const SvgColor& c1 = gradientColors_[idx];
    const SvgColor& c2 = gradientColors_[idx + 1];

    return SvgColor(
        static_cast<int>(c1.r + (c2.r - c1.r) * frac),
        static_cast<int>(c1.g + (c2.g - c1.g) * frac),
        static_cast<int>(c1.b + (c2.b - c1.b) * frac),
        c1.a + (c2.a - c1.a) * frac
    );
}

std::string SvgHeatmapChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.empty()) {
        oss << svgFooter();
        return oss.str();
    }

    int xMax = 0, yMax = 0;
    for (const auto& item : data_) {
        xMax = std::max(xMax, item.x);
        yMax = std::max(yMax, item.y);
    }
    int xCount = xMax + 1;
    int yCount = yMax + 1;

    std::vector<std::string> xLabels = xLabels_;
    if (xLabels.empty()) {
        for (int i = 0; i < xCount; ++i) {
            xLabels.push_back(std::to_string(i));
        }
    }
    std::vector<std::string> yLabels = yLabels_;
    if (yLabels.empty()) {
        for (int i = 0; i < yCount; ++i) {
            yLabels.push_back(std::to_string(i));
        }
    }

    double maxYLabelWidth = 0;
    for (const auto& label : yLabels) {
        maxYLabelWidth = std::max(maxYLabelWidth, label.length() * 8.0);
    }
    double marginLeft = std::max(kMarginLeft, maxYLabelWidth + 20);
    double marginRight = showVisualMap_ ? 80 : kMarginRight;
    double marginTop = kMarginTop;
    double marginBottom = kMarginBottom + 30;

    double gridX = marginLeft;
    double gridY = marginTop;
    double gridW = width_ - marginLeft - marginRight;
    double gridH = height_ - marginTop - marginBottom;

    double cellW = (gridW - cellGap_ * (xCount + 1)) / xCount;
    double cellH = (gridH - cellGap_ * (yCount + 1)) / yCount;

    // Y 轴标签
    for (int i = 0; i < yCount && i < static_cast<int>(yLabels.size()); ++i) {
        double y = gridY + cellGap_ + i * (cellH + cellGap_) + cellH / 2.0;
        oss << "<text x=\"" << (gridX - 8) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\""
            << " fill=\"#666\" font-size=\"12\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(yLabels[i]) << "</text>\n";
    }

    // X 轴标签
    for (int i = 0; i < xCount && i < static_cast<int>(xLabels.size()); ++i) {
        double x = gridX + cellGap_ + i * (cellW + cellGap_) + cellW / 2.0;
        double yOff = gridY + gridH + 14;
        oss << "<text x=\"" << x << "\" y=\"" << yOff
            << "\" text-anchor=\"middle\""
            << " fill=\"#666\" font-size=\"12\""
            << " font-family=\"" << fontFamily_ << "\">"
            << escapeXml(xLabels[i]) << "</text>\n";
    }

    // 绘制每个格子
    for (const auto& item : data_) {
        if (item.x < 0 || item.x >= xCount || item.y < 0 || item.y >= yCount) continue;
        if (!std::isfinite(item.value)) continue;

        double x = gridX + cellGap_ + item.x * (cellW + cellGap_);
        double y = gridY + cellGap_ + item.y * (cellH + cellGap_);

        SvgColor color = valueToColor(item.value);

        oss << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << cellW << "\" height=\"" << cellH
            << "\" fill=\"" << color.toHex() << "\"";
        if (borderRadius_ > 0) {
            oss << " rx=\"" << borderRadius_ << "\"";
        }
        oss << "/>\n";

        if (showLabel_) {
            double cx = x + cellW / 2.0;
            double cy = y + cellH / 2.0 + 4;
            int brightness = (color.r * 299 + color.g * 587 + color.b * 114) / 1000;
            std::string textColor = (brightness > 150) ? "#333" : "#fff";

            oss << "<text x=\"" << cx << "\" y=\"" << cy
                << "\" text-anchor=\"middle\""
                << " fill=\"" << textColor << "\" font-size=\"" << labelFontSize_ << "\""
                << " font-family=\"" << fontFamily_ << "\">"
                << fmt(item.value, 0) << "</text>\n";
        }
    }

    // visualMap 图例
    if (showVisualMap_) {
        double vmX = width_ - marginRight + 10;
        double vmY = gridY + 10;
        double vmW = 15;
        double vmH = gridH - 20;

        int steps = 50;
        double stepH = vmH / steps;
        for (int i = 0; i < steps; ++i) {
            double t = 1.0 - static_cast<double>(i) / (steps - 1);
            double val = minVal_ + t * (maxVal_ - minVal_);
            SvgColor c = valueToColor(val);
            oss << "<rect x=\"" << vmX << "\" y=\"" << (vmY + i * stepH)
                << "\" width=\"" << vmW << "\" height=\"" << (stepH + 0.5)
                << "\" fill=\"" << c.toHex() << "\"/>\n";
        }

        oss << "<rect x=\"" << vmX << "\" y=\"" << vmY
            << "\" width=\"" << vmW << "\" height=\"" << vmH
            << "\" fill=\"none\" stroke=\"#ccc\" stroke-width=\"1\"/>\n";

        oss << "<text x=\"" << (vmX + vmW + 5) << "\" y=\"" << (vmY + 4)
            << "\" text-anchor=\"start\""
            << " fill=\"#666\" font-size=\"10\""
            << " font-family=\"" << fontFamily_ << "\">"
            << fmt(maxVal_, 0) << "</text>\n";
        oss << "<text x=\"" << (vmX + vmW + 5) << "\" y=\"" << (vmY + vmH + 4)
            << "\" text-anchor=\"start\""
            << " fill=\"#666\" font-size=\"10\""
            << " font-family=\"" << fontFamily_ << "\">"
            << fmt(minVal_, 0) << "</text>\n";
    }

    oss << svgFooter();
    return oss.str();
}

// ============================================================
// SvgClusteringChart 实现
// 散点按簇着色 + 簇中心标记 + 图例
// 使用 ClusteringProcess 类进行聚类计算
// ============================================================

SvgClusteringChart::SvgClusteringChart(int width, int height)
    : SvgChart(width, height), palette_(defaultPalette()) {}

void SvgClusteringChart::setData(const std::vector<std::vector<double>>& data) {
    data_ = data;
}

void SvgClusteringChart::setClusterCount(int k) {
    clusterCount_ = std::max(2, k);
}

void SvgClusteringChart::setAssignments(const std::vector<int>& assignments) {
    assignments_ = assignments;
}

void SvgClusteringChart::setClusterColors(const std::vector<SvgColor>& colors) {
    clusterColors_ = colors;
}

void SvgClusteringChart::setPointSize(double size) {
    pointSize_ = std::max(1.0, size);
}

void SvgClusteringChart::setCentroidSize(double size) {
    centroidSize_ = std::max(1.0, size);
}

void SvgClusteringChart::setShowLegend(bool show) {
    showLegend_ = show;
}

std::string SvgClusteringChart::render() {
    std::ostringstream oss;
    oss << svgHeader();

    if (!title_.empty()) {
        oss << svgTitleText(title_);
    }

    if (data_.size() < 2) {
        oss << svgFooter();
        return oss.str();
    }

    // 运行聚类（外部传入标签时直接使用，质心取各簇均值）
    ClusteringProcess::Result clusterResult;
    if (assignments_.size() == data_.size()) {
        int k = clusterCount_;
        for (int a : assignments_) {
            if (a >= k) k = a + 1;
        }
        size_t dim = data_[0].size();
        clusterResult.centroids.assign(k, std::vector<double>(dim, 0.0));
        clusterResult.pointsInCluster.resize(k);
        clusterResult.clusterAssigned.resize(data_.size());
        std::vector<size_t> counts(k, 0);
        for (size_t i = 0; i < data_.size(); ++i) {
            int ci = assignments_[i];
            if (ci < 0) ci = 0;
            clusterResult.clusterAssigned[i].clusterIndex = ci;
            clusterResult.clusterAssigned[i].distance = 0.0;
            clusterResult.pointsInCluster[ci].push_back(data_[i]);
            for (size_t d = 0; d < dim; ++d) {
                clusterResult.centroids[ci][d] += data_[i][d];
            }
            counts[ci]++;
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] == 0) continue;
            for (size_t d = 0; d < dim; ++d) {
                clusterResult.centroids[c][d] /= static_cast<double>(counts[c]);
            }
        }
    } else {
        clusterResult = ClusteringProcess::simpleKMeans(data_, clusterCount_);
    }
    int k = static_cast<int>(clusterResult.centroids.size());

    // 确定簇颜色
    std::vector<SvgColor> colors;
    if (!clusterColors_.empty()) {
        colors = clusterColors_;
    } else {
        for (int i = 0; i < k; ++i) {
            colors.push_back(palette_[i % palette_.size()]);
        }
    }

    // 计算数据范围
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    for (const auto& row : data_) {
        xMin = std::min(xMin, row[0]);
        xMax = std::max(xMax, row[0]);
        yMin = std::min(yMin, row[1]);
        yMax = std::max(yMax, row[1]);
    }

    if (xMin == xMax) { xMin -= 1; xMax += 1; }
    if (yMin == yMax) { yMin -= 1; yMax += 1; }

    double xPadding = (xMax - xMin) * 0.1;
    double yPadding = (yMax - yMin) * 0.1;
    xMin -= xPadding; xMax += xPadding;
    yMin -= yPadding; yMax += yPadding;

    LinearScale xScale, yScale;
    xScale.setDomain(xMin, xMax);
    xScale.setRange(gridRect_.x, gridRect_.x + gridRect_.width);
    yScale.setDomain(yMin, yMax);
    yScale.setRange(gridRect_.y + gridRect_.height, gridRect_.y);

    // 轴
    std::vector<std::string> yLabels;
    int yTicks = 5;
    int yPrec  = tickPrecision(yMax - yMin, yTicks);
    for (int i = 0; i <= yTicks; ++i) {
        double val = yMin + (yMax - yMin) * i / yTicks;
        yLabels.push_back(fmt(val, yPrec));
    }
    std::vector<std::string> xLabels;
    int xTicks = 5;
    int xPrec  = tickPrecision(xMax - xMin, xTicks);
    for (int i = 0; i <= xTicks; ++i) {
        double val = xMin + (xMax - xMin) * i / xTicks;
        xLabels.push_back(fmt(val, xPrec));
    }
    oss << drawAxes(xLabels, yLabels);

    // 绘制散点（按簇着色）
    for (size_t i = 0; i < data_.size(); ++i) {
        int ci = clusterResult.clusterAssigned[i].clusterIndex;
        if (ci < 0 || ci >= static_cast<int>(colors.size())) continue;
        double sx = xScale.map(data_[i][0]);
        double sy = yScale.map(data_[i][1]);
        oss << svgCircle(sx, sy, pointSize_, colors[ci].toHex());
    }

    // 绘制簇中心
    for (int c = 0; c < k && c < static_cast<int>(colors.size()); ++c) {
        double cx = xScale.map(clusterResult.centroids[c][0]);
        double cy = yScale.map(clusterResult.centroids[c][1]);

        // 中心点（大号）
        oss << svgCircle(cx, cy, centroidSize_, colors[c].toHex());
        // 白色边框
        oss << "<circle cx=\"" << cx << "\" cy=\"" << cy
            << "\" r=\"" << centroidSize_
            << "\" fill=\"none\" stroke=\"#fff\" stroke-width=\"2\"/>\n";
        // 深色边框
        oss << "<circle cx=\"" << cx << "\" cy=\"" << cy
            << "\" r=\"" << centroidSize_
            << "\" fill=\"none\" stroke=\"" << colors[c].toHex()
            << "\" stroke-width=\"1\"/>\n";
    }

    // 图例（右上角）
    if (showLegend_) {
        double legendX = gridRect_.x + gridRect_.width - 10;
        double legendY = gridRect_.y + 15;
        for (int i = 0; i < k && i < static_cast<int>(colors.size()); ++i) {
            double ly = legendY + i * 20;
            oss << svgCircle(legendX - 100, ly, 5, colors[i].toHex());
            std::ostringstream label;
            label << "Cluster " << (i + 1) << " (" << clusterResult.pointsInCluster[i].size() << ")";
            oss << "<text x=\"" << (legendX - 88) << "\" y=\"" << (ly + 4)
                << "\" text-anchor=\"start\""
                << " fill=\"#333\" font-size=\"12\""
                << " font-family=\"" << fontFamily_ << "\">"
                << label.str() << "</text>\n";
        }
    }

    oss << svgFooter();
    return oss.str();
}
