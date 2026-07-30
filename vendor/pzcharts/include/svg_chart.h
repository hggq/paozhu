/*
 * SVG Chart Generator - C++ classes for generating SVG charts
 * Algorithms extracted from ECharts 6.1.0
 *
 * Supported chart types:
 *   - Basic Line Chart (基础折线图)
 *   - Basic Smooth Line Chart (基础平滑折线图)
 *   - Basic Bar Chart (基础柱状图)
 *   - Basic Scatter Chart (基础散点图)
 *   - Pie / Donut Chart (环形图)
 *   - Basic Radar Chart (基础雷达图)
 *   - Gauge Chart (仪表盘 - 基础/进度/阶段速度/等级)
 *   - Funnel Chart (漏斗图)
 */

#ifndef SVG_CHART_H
#define SVG_CHART_H

#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>

#include "svg_stats.h"

// ============================================================
// 通用数据结构
// ============================================================

struct SvgPoint {
    double x;
    double y;
    SvgPoint() : x(0), y(0) {}
    SvgPoint(double x_, double y_) : x(x_), y(y_) {}
};

struct SvgColor {
    int r, g, b;
    double a; // 0.0 ~ 1.0
    SvgColor(int r_ = 0, int g_ = 0, int b_ = 0, double a_ = 1.0)
        : r(r_), g(g_), b(b_), a(a_) {}
    std::string toRgba() const {
        std::ostringstream oss;
        oss << "rgba(" << r << "," << g << "," << b << "," << a << ")";
        return oss.str();
    }
    std::string toHex() const {
        std::ostringstream oss;
        oss << "#" << std::hex << std::setfill('0')
            << std::setw(2) << r
            << std::setw(2) << g
            << std::setw(2) << b;
        return oss.str();
    }
    static SvgColor fromHex(const std::string& hex) {
        if (hex.empty() || hex[0] != '#') return SvgColor();
        int r = 0, g = 0, b = 0;
        if (hex.size() == 7) {
            r = std::stoi(hex.substr(1, 2), nullptr, 16);
            g = std::stoi(hex.substr(3, 2), nullptr, 16);
            b = std::stoi(hex.substr(5, 2), nullptr, 16);
        }
        return SvgColor(r, g, b);
    }
};

// 默认调色板（来自 ECharts 默认配色）
inline std::vector<SvgColor> defaultPalette() {
    return {
        {84, 112, 198},  // #5470c6
        {145, 204, 117}, // #91cc75
        {250, 200, 88},  // #fac858
        {238, 102, 102}, // #ee6666
        {115, 192, 222}, // #73c0de
        {60, 162, 131},  // #3ca283
        {252, 132, 82},  // #fc8452
        {154, 96, 180},  // #9a60b4
        {234, 124, 204}, // #ea7ccc
    };
}

// ============================================================
// 饼图数据项
// ============================================================

struct PieDataItem {
    std::string name;
    double value;
    SvgColor color;
    PieDataItem(const std::string& n = "", double v = 0, const SvgColor& c = SvgColor())
        : name(n), value(v), color(c) {}
};

// ============================================================
// 坐标变换工具
// ============================================================

struct ChartRect {
    double x, y, width, height;
    ChartRect(double x_ = 0, double y_ = 0, double w = 0, double h = 0)
        : x(x_), y(y_), width(w), height(h) {}
};

class LinearScale {
public:
    LinearScale() : domainMin_(0), domainMax_(1), rangeMin_(0), rangeMax_(1) {}

    void setDomain(double min, double max) {
        domainMin_ = min;
        domainMax_ = max;
    }

    void setRange(double min, double max) {
        rangeMin_ = min;
        rangeMax_ = max;
    }

    double map(double value) const {
        double ratio = (domainMax_ == domainMin_)
            ? 0.0 : (value - domainMin_) / (domainMax_ - domainMin_);
        return rangeMin_ + ratio * (rangeMax_ - rangeMin_);
    }

private:
    double domainMin_, domainMax_, rangeMin_, rangeMax_;
};

// ============================================================
// 基础 SVG 图表类
// ============================================================

class SvgChart {
public:
    SvgChart(int width = 800, int height = 500);
    virtual ~SvgChart() = default;

    // 设置图表尺寸
    void setSize(int width, int height);

    // 设置标题
    void setTitle(const std::string& title);

    // 设置字体（默认 "Arial, sans-serif"）
    void setFontFamily(const std::string& font);

    // 生成 SVG 字符串
    virtual std::string render() = 0;

protected:
    int width_;
    int height_;
    std::string title_;
    std::string fontFamily_ = "Arial, sans-serif";
    ChartRect gridRect_; // 绘图区域

    // 栅格边距
    static constexpr double kMarginLeft = 60;
    static constexpr double kMarginRight = 20;
    static constexpr double kMarginTop = 50;
    static constexpr double kMarginBottom = 40;

    // SVG 工具方法
    std::string svgHeader() const;
    std::string svgFooter() const;
    std::string svgRect(double x, double y, double w, double h,
                        const std::string& fill, double rx = 0) const;
    std::string svgCircle(double cx, double cy, double r,
                          const std::string& fill) const;
    std::string svgLine(double x1, double y1, double x2, double y2,
                        const std::string& stroke, double width = 1) const;
    std::string svgPolyline(const std::vector<SvgPoint>& pts,
                            const std::string& stroke, double width = 2,
                            const std::string& fill = "none") const;
    std::string svgPath(const std::string& d,
                        const std::string& stroke, double width = 2,
                        const std::string& fill = "none") const;
    std::string svgText(double x, double y, const std::string& text,
                        const std::string& anchor = "middle",
                        const std::string& fill = "#333",
                        int fontSize = 12) const;
    std::string svgTitleText(const std::string& text) const;

    // 绘制坐标轴
    std::string drawAxes(const std::vector<std::string>& xLabels = {},
                         const std::vector<std::string>& yLabels = {}) const;

    // 格式化数值
    static std::string fmt(double val, int precision = 1);

    // 按值域跨度自适应小数位数：跨度小时刻度取整会出现重复标签（如 1、1、0、-1、-1）
    static int tickPrecision(double span, int ticks);

    // XML text escaping (& → &amp; < → &lt; > → &gt;)
    static std::string escapeXml(const std::string& text);
};

// ============================================================
// 1. 基础折线图 (Basic Line Chart)
// ============================================================

class SvgLineChart : public SvgChart {
public:
    SvgLineChart(int width = 800, int height = 500);

    // 添加一组数据系列
    // data: 数据点 (y值)
    // name: 系列名称
    // color: 线条颜色 (空则使用默认调色板)
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor());

    // 设置 X 轴标签
    void setXLabels(const std::vector<std::string>& labels);

    // 生成 SVG
    std::string render() override;

private:
    struct Series {
        std::vector<double> data;
        std::string name;
        SvgColor color;
    };
    std::vector<Series> series_;
    std::vector<std::string> xLabels_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
};

// ============================================================
// 2. 基础平滑折线图 (Basic Smooth Line Chart)
//    算法来自 ECharts poly.ts 的 drawSegment
// ============================================================

class SvgSmoothLineChart : public SvgChart {
public:
    SvgSmoothLineChart(int width = 800, int height = 500);

    // 添加一组数据系列
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor(),
                   double smooth = 0.5);

    void setXLabels(const std::vector<std::string>& labels);

    std::string render() override;

private:
    struct Series {
        std::vector<double> data;
        std::string name;
        SvgColor color;
        double smooth;
    };
    std::vector<Series> series_;
    std::vector<std::string> xLabels_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;

    // 平滑曲线核心算法：生成贝塞尔曲线路径
    // 从 ECharts poly.ts drawSegment 提取
    std::string buildSmoothPath(const std::vector<SvgPoint>& points,
                                double smooth) const;
};

// ============================================================
// 3. 基础柱状图 (Basic Bar Chart)
// ============================================================

class SvgBarChart : public SvgChart {
public:
    SvgBarChart(int width = 800, int height = 500);

    // 添加一组数据系列（统一颜色）
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor());

    // 添加一组数据系列（每个柱子独立颜色）
    void addSeries(const std::vector<double>& data,
                   const std::vector<SvgColor>& colors,
                   const std::string& name = "");

    void setXLabels(const std::vector<std::string>& labels);

    std::string render() override;

private:
    struct Series {
        std::vector<double> data;
        std::vector<SvgColor> colors; // 每个柱子的颜色
        std::string name;
        SvgColor color; // 默认颜色
        bool usePerBarColor = false; // 是否使用独立颜色
    };
    std::vector<Series> series_;
    std::vector<std::string> xLabels_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
};

// ============================================================
// 3b. 水平条形图 (Horizontal Bar Chart)
//     算法来自 ECharts BarView.ts (isHorizontal 模式)
// ============================================================

class SvgHorizontalBarChart : public SvgChart {
public:
    SvgHorizontalBarChart(int width = 800, int height = 500);

    // 添加一组数据系列（统一颜色）
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor());

    // 添加一组数据系列（每个柱子独立颜色）
    void addSeries(const std::vector<double>& data,
                   const std::vector<SvgColor>& colors,
                   const std::string& name = "");

    // 设置 Y 轴标签（类别名）
    void setYLabels(const std::vector<std::string>& labels);

    std::string render() override;

private:
    struct Series {
        std::vector<double> data;
        std::vector<SvgColor> colors;
        std::string name;
        SvgColor color;
        bool usePerBarColor = false;
    };
    std::vector<Series> series_;
    std::vector<std::string> yLabels_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
};

// ============================================================
// 4. 基础散点图 (Basic Scatter Chart)
// ============================================================

class SvgScatterChart : public SvgChart {
public:
    SvgScatterChart(int width = 800, int height = 500);

    // 添加一组数据系列
    // data: (x, y) 点对
    void addSeries(const std::vector<SvgPoint>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor(),
                   double symbolSize = 8);

    std::string render() override;

private:
    struct Series {
        std::vector<SvgPoint> data;
        std::string name;
        SvgColor color;
        double symbolSize;
    };
    std::vector<Series> series_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
};

// ============================================================
// 5. 环形图 (Pie / Donut Chart)
//    算法来自 ECharts pieLayout.ts
// ============================================================

class SvgPieChart : public SvgChart {
public:
    SvgPieChart(int width = 800, int height = 500);

    // 添加数据项
    void addData(const std::string& name, double value,
                 const SvgColor& color = SvgColor());

    // 设置内半径比例 (0 = 饼图, >0 = 环形图)
    // 例如 0.5 表示内半径是外半径的 50%
    void setInnerRadius(double ratio);

    // 设置是否为环形图
    void setDonut(bool donut);

    std::string render() override;

private:
    std::vector<PieDataItem> data_;
    double innerRadiusRatio_ = 0.0;
    bool isDonut_ = false;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;

    // 计算扇区 SVG 路径 (来自 ECharts pieLayout 算法)
    std::string buildSectorPath(double cx, double cy, double r, double r0,
                                double startAngle, double endAngle) const;
    // 计算扇区标签位置
    SvgPoint labelPosition(double cx, double cy, double r,
                           double startAngle, double endAngle) const;
};

// ============================================================
// 6. 基础雷达图 (Basic Radar Chart)
//    算法来自 ECharts Radar.ts 的 coordToPoint 和 radarLayout.ts
// ============================================================

class SvgRadarChart : public SvgChart {
public:
    SvgRadarChart(int width = 800, int height = 500);

    // 设置雷达图的指标（维度）名称
    void setIndicators(const std::vector<std::string>& names);

    // 添加一组数据系列
    // data: 每个指标的值
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor());

    // 设置填充透明度（0=不填充，默认 0.3）
    void setFillOpacity(double opacity);

    // 设置边框宽度（默认 2）
    void setBorderWidth(double width);

    std::string render() override;

private:
    struct Series {
        std::vector<double> data;
        std::string name;
        SvgColor color;
    };
    std::vector<Series> series_;
    std::vector<std::string> indicators_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
    double fillOpacity_ = 0.3;       // 填充透明度
    double borderWidth_ = 2;         // 边框宽度

    // 将坐标值转换为 SVG 坐标点 (来自 ECharts Radar.ts coordToPoint)
    SvgPoint coordToPoint(double cx, double cy, double r, double r0,
                          double value, double min, double max,
                          double angle) const;

    // 计算雷达图布局
    struct RadarLayout {
        double cx, cy;       // 中心点
        double r, r0;        // 外半径、内半径
        double startAngle;   // 起始角度
        std::vector<double> axisAngles; // 每个轴的角度
    };
    RadarLayout computeLayout() const;
};

// ============================================================
// 7. 仪表盘 (Gauge Chart)
//    算法来自 ECharts GaugeView.ts / GaugeSeries.ts / PointerPath.ts
//    支持四种类型：基础仪表盘、进度仪表盘、阶段速度仪表盘、等级仪表盘
// ============================================================

struct GaugeColorStop {
    double percent;     // 0.0 ~ 1.0
    SvgColor color;
    GaugeColorStop(double p = 0, const SvgColor& c = SvgColor())
        : percent(p), color(c) {}
};

class SvgGaugeChart : public SvgChart {
public:
    // 仪表盘类型
    enum GaugeType {
        TYPE_BASIC,       // 基础仪表盘
        TYPE_PROGRESS,    // 进度仪表盘
        TYPE_STAGE_SPEED, // 阶段速度仪表盘
        TYPE_LEVEL        // 等级仪表盘
    };

    SvgGaugeChart(int width = 600, int height = 400);

    // 设置仪表盘类型
    void setGaugeType(GaugeType type);

    // 设置当前值
    void setValue(double value);

    // 设置数值范围
    void setMinMax(double minVal, double maxVal);

    // 设置角度范围（默认 225° ~ -45°）
    void setAngleRange(double startAngle, double endAngle);

    // 设置颜色分段（用于阶段速度仪表盘和等级仪表盘）
    // 例如: {{0.3, {103,224,227}}, {0.7, {55,162,218}}, {1.0, {253,102,109}}}
    void setColorStops(const std::vector<GaugeColorStop>& stops);

    // 设置标题
    void setGaugeTitle(const std::string& title);

    // 设置标题偏移（百分比，相对圆心）
    void setTitleOffset(double xPercent, double yPercent);

    // 设置详情偏移（百分比，相对圆心）
    void setDetailOffset(double xPercent, double yPercent);

    // 设置分割段数
    void setSplitNumber(int n);

    // 显示/隐藏指针
    void setShowPointer(bool show);

    // 显示/隐藏进度条
    void setShowProgress(bool show);

    // 显示/隐藏刻度
    void setShowTicks(bool show);

    // 显示/隐藏刻度标签
    void setShowLabels(bool show);

    // 设置标签距离（负值表示在弧内侧）
    void setLabelDistance(double distance);

    // 设置标签旋转方式: "tangential"=切向, "radial"=径向, ""=无旋转
    void setLabelRotate(const std::string& rotate);

    // 设置标签格式化器（每个标签位置的自定义文本）
    void setLabelFormatter(const std::vector<std::string>& labels);

    // 设置指针长度（占半径的百分比，0~1）
    void setPointerLength(double length);

    // 设置刻度是否继承颜色
    void setTickInheritColor(bool inherit);

    // 设置分割线宽度
    void setSplitLineWidth(double width);

    // 设置进度条颜色
    // 传入 SvgColor(0,0,0,0) 表示使用默认调色板色(#5470c6)
    void setProgressColor(const SvgColor& color);

    std::string render() override;

private:
    GaugeType gaugeType_ = TYPE_BASIC;
    double value_ = 50;
    double minVal_ = 0;
    double maxVal_ = 100;
    double startAngle_ = 225;   // 度
    double endAngle_ = -45;     // 度
    std::vector<GaugeColorStop> colorStops_;
    std::string gaugeTitle_;
    double titleOffsetX_ = 0;        // 标题 X 偏移（百分比）
    double titleOffsetY_ = 20;       // 标题 Y 偏移（百分比，默认 20%）
    double detailOffsetX_ = 0;       // 详情 X 偏移（百分比）
    double detailOffsetY_ = 40;      // 详情 Y 偏移（百分比，默认 40%）
    int splitNumber_ = 10;
    bool showPointer_ = true;
    bool showProgress_ = false;
    bool showTicks_ = true;
    bool showLabels_ = true;
    double labelDistance_ = 15;        // 标签距离（默认 15，负值=内侧）
    std::string labelRotate_;          // 标签旋转: "tangential", "radial", ""
    std::vector<std::string> labelFormatter_; // 自定义标签文本
    double pointerLength_ = 0.6;       // 指针长度（占半径比例）
    bool tickInheritColor_ = false;    // 刻度是否继承颜色
    double splitLineWidth_ = 3;        // 分割线宽度
    SvgColor progressColor_;           // 进度条颜色
    bool progressColorSet_ = false;    // 是否已设置进度条颜色

    // 获取指定百分比对应的颜色
    SvgColor getColor(double percent) const;

    // 绘制圆弧路径
    std::string arcPath(double cx, double cy, double r, double r0,
                        double startAngle, double endAngle,
                        bool clockwise = true) const;

    // 绘制指针
    std::string drawPointer(double cx, double cy, double r,
                            double angleDeg, double pointerLen) const;

    // 绘制锚点
    std::string drawAnchor(double cx, double cy) const;

    // 绘制刻度、分割线和标签
    std::string drawTicks(double cx, double cy, double r,
                          double startAngle, double endAngle) const;

    // 绘制标题和详情
    std::string drawTitleAndDetail(double cx, double cy, double r) const;
};

// ============================================================
// 8. 漏斗图 (Funnel Chart)
//    算法来自 ECharts funnelLayout.ts / FunnelView.ts
// ============================================================

struct FunnelDataItem {
    std::string name;
    double value;
    SvgColor color;
    FunnelDataItem(const std::string& n = "", double v = 0,
                   const SvgColor& c = SvgColor())
        : name(n), value(v), color(c) {}
};

class SvgFunnelChart : public SvgChart {
public:
    SvgFunnelChart(int width = 800, int height = 500);

    // 添加数据项
    void addData(const std::string& name, double value,
                 const SvgColor& color = SvgColor());

    // 设置排序方式: "descending" (默认), "ascending", "none"
    void setSort(const std::string& sort);

    // 设置对齐方式: "left", "center" (默认), "right"
    void setFunnelAlign(const std::string& align);

    // 设置项目间距
    void setGap(int gap);

    // 设置最小/最大尺寸（百分比 0~100）
    void setMinSize(double percent);
    void setMaxSize(double percent);

    // 设置方向: "vertical" (默认), "horizontal"
    void setOrient(const std::string& orient);

    std::string render() override;

private:
    std::vector<FunnelDataItem> data_;
    std::string sort_ = "descending";
    std::string funnelAlign_ = "center";
    int gap_ = 2;
    double minSize_ = 0;    // 百分比
    double maxSize_ = 100;  // 百分比
    std::string orient_ = "vertical";
    std::vector<SvgColor> palette_;

    // 漏斗布局计算
    struct FunnelLayout {
        std::vector<std::vector<SvgPoint>> points; // 每个 item 的 4 个顶点
    };
    FunnelLayout computeLayout(const ChartRect& viewRect) const;
};

// ============================================================
// 8b. 交错正负轴标签 (Bar Chart with Negative Value / Alternating Labels)
//     算法来自 ECharts bar-negative2 示例
//     特征：水平条形图、X轴在上方、虚线分割线、标签显示在柱子上
// ============================================================

class SvgNegativeBarChart : public SvgChart {
public:
    SvgNegativeBarChart(int width = 800, int height = 500);

    // 添加数据：value 可为正或负，label 为类别名
    void addData(double value, const std::string& label,
                 const SvgColor& color = SvgColor());

    std::string render() override;

private:
    struct DataItem {
        double value;
        std::string label;
        SvgColor color;
    };
    std::vector<DataItem> data_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
};

// ============================================================
// 9. 基础 K 线图 (Candlestick Chart)
//    算法来自 ECharts candlestickLayout.ts / CandlestickView.ts
// ============================================================

struct CandlestickData {
    double open;   // 开盘价
    double close;  // 收盘价
    double low;    // 最低价
    double high;   // 最高价
    CandlestickData(double o = 0, double c = 0, double l = 0, double h = 0)
        : open(o), close(c), low(l), high(h) {}
};

class SvgCandlestickChart : public SvgChart {
public:
    SvgCandlestickChart(int width = 800, int height = 500);

    // 添加一组 K 线数据
    void addData(const std::vector<CandlestickData>& data,
                 const std::string& name = "");

    // 设置 X 轴标签（日期）
    void setXLabels(const std::vector<std::string>& labels);

    // 设置阳线/阴线颜色
    void setColorUp(const SvgColor& color);    // 阳线（收盘 > 开盘）
    void setColorDown(const SvgColor& color);  // 阴线（收盘 < 开盘）

    // 设置蜡烛宽度
    void setCandleWidth(double width);

    // 设置是否显示十字星特殊颜色
    void setDojiColor(const SvgColor& color);

    std::string render() override;

private:
    std::vector<CandlestickData> data_;
    std::vector<std::string> xLabels_;
    SvgColor colorUp_   = SvgColor(235, 84, 84);   // 阳线红 #eb5454
    SvgColor colorDown_ = SvgColor(71, 178, 98);    // 阴线绿 #47b262
    SvgColor borderUp_  = SvgColor(235, 84, 84);
    SvgColor borderDown_ = SvgColor(71, 178, 98);
    SvgColor dojiColor_ = SvgColor();               // 十字星颜色（空=不使用）
    double candleWidth_ = 0; // 0 = 自动计算

    // 判断 sign（来自 ECharts getSign）
    // 1 = 阳线, -1 = 阴线, 0 = 十字星
    int getSign(size_t index) const;
};

// ============================================================
// 10. Kernel Density Estimation Chart
//     Uses stats.h KernelDensity for pure statistical computation
//     Features: smooth density curves, multi-group comparison, no fill (PDF-safe)
// ============================================================

class SvgDensityChart : public SvgChart {
public:
    SvgDensityChart(int width = 800, int height = 500);

    // Add a density series from raw data
    void addSeries(const std::vector<double>& data,
                   const std::string& name = "",
                   const SvgColor& color = SvgColor());

    // Show rug plot (data tick marks at bottom)
    void setShowRug(bool show);

    std::string render() override;

private:
    struct Series {
        KernelDensity kde;
        std::string name;
        SvgColor color;
    };
    std::vector<Series> series_;
    std::vector<SvgColor> palette_;
    int paletteIdx_ = 0;
    bool showRug_ = false;
};

// ============================================================
// 10b. 乔哈里视窗 (Johari Window)
//      2x2 象限图，展示自我认知模型
//      - 公开区 (Open): 自己知道、别人也知道
//      - 盲区 (Blind): 自己不知道、别人知道
//      - 隐藏区 (Hidden): 自己知道、别人不知道
//      - 未知区 (Unknown): 自己不知道、别人也不知道
// ============================================================

class SvgJohariWindowChart : public SvgChart {
public:
    // 象限索引
    enum Quadrant {
        OPEN = 0,     // 左上: 公开区
        BLIND = 1,    // 右上: 盲区
        HIDDEN = 2,   // 左下: 隐藏区
        UNKNOWN = 3   // 右下: 未知区
    };

    SvgJohariWindowChart(int width = 800, int height = 600);

    // 设置一个象限（标题 + 内容，标题顶对齐，内容支持 \n 多行）
    void setQuadrant(Quadrant q, const std::string& title, const std::string& content,
                     const SvgColor& bgColor = SvgColor(),
                     const SvgColor& titleColor = SvgColor(),
                     const SvgColor& contentColor = SvgColor(),
                     int titleFontSize = 14, int contentFontSize = 11);

    // 设置 X 轴标签（底部两列）
    // left: 左侧列标签（默认 "Known to Others"）
    // right: 右侧列标签（默认 "Unknown to Others"）
    void setXLabels(const std::string& left, const std::string& right);

    // 设置 Y 轴标签（左侧两行）
    // top: 上方行标签（默认 "Known to Self"）
    // bottom: 下方行标签（默认 "Unknown to Self"）
    void setYLabels(const std::string& top, const std::string& bottom);

    // 设置全局边框颜色和宽度
    void setCellBorder(const std::string& color, double width = 1);

    // 设置格子圆角半径
    void setCellBorderRadius(double radius);

    // 设置标题和内容之间的间距（像素）
    void setContentOffset(double offset);

    // 设置标题对齐方式: "start" (左对齐，默认), "center" (居中)
    void setTitleAlign(const std::string& align);

    // 设置是否显示背景色（默认 true）
    void setShowBackground(bool show);

    std::string render() override;

private:
    struct QuadrantData {
        std::string title;
        std::string content;
        SvgColor bgColor = SvgColor(255, 255, 255);
        SvgColor titleColor = SvgColor(51, 51, 51);
        SvgColor contentColor = SvgColor(102, 102, 102);
        int titleFontSize = 14;
        int contentFontSize = 11;
    };
    QuadrantData quadrants_[4];
    std::string xLabelLeft_   = "Known to Others";
    std::string xLabelRight_  = "Unknown to Others";
    std::string yLabelTop_    = "Known to Self";
    std::string yLabelBottom_ = "Unknown to Self";
    std::string cellBorderColor_ = "#ccc";
    double cellBorderWidth_ = 1;
    double cellBorderRadius_ = 0;
    double contentOffset_ = 12;
    std::string titleAlign_ = "start";
    bool showBackground_ = true;
};

// ============================================================
// 11. 九宫格图 (3x3 Grid Chart)
//     每个格子可自定义背景色、边框、标题和内容
//     只有标题时居中，有标题+内容时标题顶对齐
//     两个轴可定义 3 个或 1 个文字名称
// ============================================================

class SvgGridChart : public SvgChart {
public:
    struct CellData {
        std::string title;          // 标题（居中或顶对齐）
        std::string content;        // 内容（标题下方）
        SvgColor bgColor;           // 背景色
        SvgColor titleColor;        // 标题颜色
        SvgColor contentColor;      // 内容颜色
        int titleFontSize = 12;     // 标题字号
        int contentFontSize = 10;   // 内容字号
        SvgColor borderColor;       // 格子独立边框颜色（未设置则用全局）
        double borderWidth = -1;    // 格子独立边框宽度（-1 = 用全局）
        std::string titleAlign;     // 标题对齐（空=默认:纯标题居中,有内容左对齐; "start"/"center"/"end"）
        double contentOffset = 10;  // 内容文本距标题底部距离（像素）
    };

    SvgGridChart(int width = 800, int height = 600);

    // 设置单个格子（只有标题，居中显示）
    void setCell(int row, int col, const std::string& title,
                 const SvgColor& bgColor = SvgColor(),
                 const SvgColor& titleColor = SvgColor(),
                 int titleFontSize = 12);

    // 设置单个格子（标题 + 内容，标题顶对齐）
    void setCell(int row, int col, const std::string& title, const std::string& content,
                 const SvgColor& bgColor = SvgColor(),
                 const SvgColor& titleColor = SvgColor(),
                 const SvgColor& contentColor = SvgColor(),
                 int titleFontSize = 12, int contentFontSize = 10);

    // 设置格子独立边框
    void setCellBorder(int row, int col, const std::string& color, double width);

    // 设置格子标题对齐方式（"start"=左对齐 / "center"=居中 / "end"=右对齐；空字符串=默认）
    void setCellTitleAlign(int row, int col, const std::string& align);

    // 设置格子内容文本距标题底部距离（像素）
    void setCellContentOffset(int row, int col, double offset);

    // 设置 X 轴底部标签（3 个）
    void setXLabels(const std::string& l0, const std::string& l1, const std::string& l2);

    // 设置 Y 轴左侧标签（3 个）
    void setYLabels(const std::string& l0, const std::string& l1, const std::string& l2);

    // 设置 X 轴底部标签（1 个，居中）
    void setXLabel(const std::string& label);

    // 设置 Y 轴左侧标签（1 个，居中）
    void setYLabel(const std::string& label);

    // 设置 Y 轴标签位置（默认左侧）
    void setYLabelPosition(const std::string& pos); // "left" (default) or "right"

    // 设置 X 轴标签位置（默认底部）
    void setXLabelPosition(const std::string& pos); // "bottom" (default) or "top"

    // 设置 Y 轴标签是否逆时针旋转 90 度（默认 true）
    void setYLabelRotate(bool rotate);

    // 设置全局格子边框颜色和宽度
    void setCellBorder(const std::string& color, double width = 1);

    // 设置格子圆角半径
    void setCellBorderRadius(double radius);

    std::string render() override;

private:
    CellData cells_[3][3];
    std::string xLabels_[3];
    std::string yLabels_[3];
    bool xLabelsSet_ = false;
    bool yLabelsSet_ = false;
    std::string cellBorderColor_ = "#ccc";
    double cellBorderWidth_ = 1;
    double cellBorderRadius_ = 0;
    std::string yLabelPos_ = "left";
    std::string xLabelPos_ = "bottom";
    bool yLabelRotate_ = true;         // Y 轴标签是否逆时针旋转 90 度
};

// ============================================================
// 12. 回归线图 (Regression Chart)
//     散点图 + 回归曲线 + 公式标签
//     使用 LinearRegression 类进行回归计算
// ============================================================

class SvgRegressionChart : public SvgChart {
public:
    SvgRegressionChart(int width = 800, int height = 500);

    // 设置原始数据点 [[x1,y1], [x2,y2], ...]
    void setData(const std::vector<std::vector<double>>& data);

    // 设置回归方法
    void setMethod(LinearRegression::Method method);

    // 设置多项式回归的阶数（仅 POLYNOMIAL 时生效）
    void setPolynomialOrder(int order);

    // 设置散点颜色
    void setPointColor(const SvgColor& color);

    // 设置回归线颜色
    void setLineColor(const SvgColor& color);

    // 设置散点大小
    void setPointSize(double size);

    std::string render() override;

private:
    std::vector<std::vector<double>> data_;
    LinearRegression::Method method_ = LinearRegression::LINEAR;
    int polynomialOrder_ = 2;
    SvgColor pointColor_ = SvgColor(84, 112, 198);
    SvgColor lineColor_ = SvgColor(238, 102, 102);
    double pointSize_ = 6;
};

// ============================================================
// 13. 热力图 (Heatmap Chart)
//     算法来自 ECharts HeatmapView.ts / HeatmapSeries.ts
//     使用 Cartesian 坐标系，X/Y 轴均为 category 类型
//     每个格子为矩形，颜色根据 visualMap 映射
// ============================================================

struct HeatmapDataItem {
    int x;          // category index for x-axis
    int y;          // category index for y-axis
    double value;
    HeatmapDataItem(int x_ = 0, int y_ = 0, double v = 0)
        : x(x_), y(y_), value(v) {}
};

class SvgHeatmapChart : public SvgChart {
public:
    SvgHeatmapChart(int width = 800, int height = 500);

    // 添加单条数据 [x, y, value]
    void addData(int x, int y, double value);

    // 批量添加数据
    void addData(const std::vector<HeatmapDataItem>& data);

    // 设置 X 轴标签（category 名称）
    void setXLabels(const std::vector<std::string>& labels);

    // 设置 Y 轴标签（category 名称）
    void setYLabels(const std::vector<std::string>& labels);

    // 设置 visualMap 数值范围
    void setValueRange(double minVal, double maxVal);

    // 设置颜色渐变（默认: 浅蓝 → #5070dd）
    // 至少需要 2 个颜色，value 在 min~max 之间线性插值
    void setColorGradient(const std::vector<SvgColor>& colors);

    // 显示/隐藏格子内的数值标签
    void setShowLabel(bool show);

    // 设置标签字号
    void setLabelFontSize(int size);

    // 设置格子之间的间距（像素，默认 1）
    void setCellGap(double gap);

    // 设置格子圆角半径
    void setBorderRadius(double radius);

    // 设置是否显示 visualMap 图例
    void setShowVisualMap(bool show);

    std::string render() override;

private:
    std::vector<HeatmapDataItem> data_;
    std::vector<std::string> xLabels_;
    std::vector<std::string> yLabels_;
    double minVal_ = 0;
    double maxVal_ = 1;
    bool valueRangeSet_ = false;
    std::vector<SvgColor> gradientColors_;
    bool showLabel_ = true;
    int labelFontSize_ = 11;
    double cellGap_ = 1;
    double borderRadius_ = 0;
    bool showVisualMap_ = true;

    // 根据 value 在 minVal~maxVal 之间线性插值颜色
    SvgColor valueToColor(double value) const;
};

// ============================================================
// 14. 聚类散点图 (Clustering Scatter Chart)
//     散点按簇着色 + 簇中心标记 + 图例
//     使用 ClusteringProcess 类进行聚类计算
// ============================================================

class SvgClusteringChart : public SvgChart {
public:
    SvgClusteringChart(int width = 800, int height = 500);

    // 设置原始数据点 [[x1,y1], [x2,y2], ...]
    void setData(const std::vector<std::vector<double>>& data);

    // 设置聚类个数
    void setClusterCount(int k);

    // 设置外部聚类标签（与 data 一一对应，0-based 簇号）
    // 设置后 render() 直接使用该标签着色，质心取各簇均值；不设置则内部运行 K-Means
    void setAssignments(const std::vector<int>& assignments);

    // 自定义每个簇的颜色（不设置则使用默认调色板）
    void setClusterColors(const std::vector<SvgColor>& colors);

    // 设置散点大小
    void setPointSize(double size);

    // 设置中心点大小
    void setCentroidSize(double size);

    // 是否绘制右上角内置图例（默认绘制）。簇数较多时图例会压住数据点，
    // 外部另有图例说明时可关掉
    void setShowLegend(bool show);

    std::string render() override;

private:
    std::vector<std::vector<double>> data_;
    int clusterCount_ = 3;
    std::vector<int> assignments_;
    std::vector<SvgColor> clusterColors_;
    std::vector<SvgColor> palette_;
    double pointSize_ = 6;
    double centroidSize_ = 10;
    bool showLegend_ = true;
};

#endif // SVG_CHART_H
