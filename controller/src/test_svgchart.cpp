#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <sstream>
#ifdef ENABLE_CHART
#include "svg_chart.h"
#include "svg_stats.h"
#endif// ENABLE_CHART

// // Save SVG to file
// void saveSvg(const std::string& filename, const std::string& svg) {
//     std::ofstream file(filename);
//     file << svg;
//     file.close();
//     std::cout << "Generated: " << filename << std::endl;
// }
namespace http
{
//@urlpath(null,test_svgchart)
std::string test_svgchart(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_CHART
    client << "<p>" << "1. Basic Line Chart" << "</p>";
    // ============================================================
    // 1. Basic Line Chart
    // ============================================================
    {
        SvgLineChart chart(800, 500);
        chart.setTitle("Basic Line Chart");
        chart.setXLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        chart.addSeries({820, 932, 901, 934, 1290, 1330, 1320}, "Sales");
        chart.addSeries({620, 732, 701, 634, 890, 930, 920}, "Orders");
        //saveSvg("line_chart.svg", chart.render());
        client << chart.render();
    }
    client << "<p>" << "2. Basic Smooth Line Chart" << "</p>";
    // ============================================================
    // 2. Basic Smooth Line Chart
    // ============================================================
    {
        SvgSmoothLineChart chart(800, 500);
        chart.setTitle("Basic Smooth Line Chart");
        chart.setXLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        chart.addSeries({820, 932, 901, 934, 1290, 1330, 1320}, "Sales", SvgColor(), 0.5);
        chart.addSeries({620, 732, 701, 634, 890, 930, 920}, "Orders", SvgColor(), 0.5);
        //saveSvg("smooth_line_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "3. Basic Bar Chart" << "</p>";
    // ============================================================
    // 3. Basic Bar Chart
    // ============================================================
    {
        SvgBarChart chart(800, 500);
        chart.setTitle("Basic Bar Chart");
        chart.setXLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        chart.addSeries({120, 200, 150, 80, 70, 110, 130}, "Sales");
        chart.addSeries({80, 120, 100, 60, 50, 80, 90}, "Orders");
        //saveSvg("bar_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "3b. Bar Chart - Custom Bar Colors" << "</p>";
    // ============================================================
    // 3b. Bar Chart - Custom Bar Colors
    // ============================================================
    {
        SvgBarChart chart(800, 500);
        chart.setTitle("Bar Chart - Custom Bar Colors");
        chart.setXLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        chart.addSeries({120, 200, 150, 80, 70, 110, 130}, {
                                                               SvgColor(238, 102, 102),// Red
                                                               SvgColor(250, 200, 88), // Yellow
                                                               SvgColor(103, 224, 127),// Green
                                                               SvgColor(55, 162, 218), // Blue
                                                               SvgColor(154, 96, 180), // Purple
                                                               SvgColor(252, 132, 82), // Orange
                                                               SvgColor(115, 192, 222),// Cyan
                                                           },
                        "Sales");
        //saveSvg("bar_colors.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "3c. Horizontal Bar Chart (World Population)" << "</p>";
    // ============================================================
    // 3c. Horizontal Bar Chart (World Population)
    // ============================================================
    {
        SvgHorizontalBarChart chart(800, 500);
        chart.setTitle("World Population - Horizontal Bar");
        chart.setYLabels({"China", "India", "USA", "Indonesia", "Brazil", "Pakistan", "Nigeria", "Bangladesh"});
        chart.addSeries({141178, 138000, 33100, 27352, 21256, 22089, 20613, 16468}, "Population(10k)");
        //saveSvg("horizontal_bar.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "4. Basic Scatter Chart" << "</p>";
    // ============================================================
    // 4. Basic Scatter Chart
    // ============================================================
    {
        SvgScatterChart chart(800, 500);
        chart.setTitle("Basic Scatter Chart");
        chart.addSeries({{10.0, 8.04}, {8.0, 6.95}, {13.0, 7.58}, {9.0, 8.81}, {11.0, 8.33}, {14.0, 9.96}, {6.0, 7.24}, {4.0, 4.26}, {12.0, 10.84}, {7.0, 4.82}, {5.0, 5.68}}, "Dataset A", SvgColor(), 7);
        chart.addSeries({{8.0, 6.5}, {10.0, 8.0}, {12.0, 9.5}, {6.0, 5.5}, {14.0, 11.0}, {4.0, 3.5}, {9.0, 7.0}, {11.0, 8.5}}, "Dataset B", SvgColor(), 7);
        //saveSvg("scatter_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "5. Donut Chart" << "</p>";
    // ============================================================
    // 5. Donut Chart
    // ============================================================
    {
        SvgPieChart chart(800, 500);
        chart.setTitle("Donut Chart");
        chart.setDonut(true);
        chart.addData("Search Engine", 1048);
        chart.addData("Direct", 735);
        chart.addData("Email", 580);
        chart.addData("Affiliate", 484);
        chart.addData("Video", 300);
        //saveSvg("donut_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "6. Pie Chart" << "</p>";
    // ============================================================
    // 6. Pie Chart
    // ============================================================
    {
        SvgPieChart chart(800, 500);
        chart.setTitle("Pie Chart");
        chart.setDonut(false);
        chart.addData("Search Engine", 1048);
        chart.addData("Direct", 735);
        chart.addData("Email", 580);
        chart.addData("Affiliate", 484);
        chart.addData("Video", 300);
        //saveSvg("pie_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "7. Basic Radar Chart" << "</p>";
    // ============================================================
    // 7. Basic Radar Chart
    // ============================================================
    {
        SvgRadarChart chart(800, 500);
        chart.setTitle("Basic Radar Chart");
        chart.setIndicators({"Sales", "Admin", "Tech", "Support", "R&D", "Marketing"});
        chart.addSeries({80, 90, 70, 85, 95, 75}, "Budget");
        chart.addSeries({60, 75, 80, 70, 85, 65}, "Actual");
        //saveSvg("radar_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "Radar Chart - Border Only (No Fill)" << "</p>";
    // Radar Chart - Border Only (No Fill)
    {
        SvgRadarChart chart(800, 500);
        chart.setTitle("Radar Chart - Border Only");
        chart.setIndicators({"Sales", "Admin", "Tech", "Support", "R&D", "Marketing"});
        chart.setFillOpacity(0);  // no fill
        chart.setBorderWidth(2.5);// thicker border
        chart.addSeries({80, 90, 70, 85, 95, 75}, "Budget", SvgColor(84, 112, 198));
        chart.addSeries({60, 75, 80, 70, 85, 65}, "Actual", SvgColor(238, 102, 102));
        //saveSvg("radar_border.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "8. Basic Gauge Chart" << "</p>";
    // ============================================================
    // 8. Basic Gauge Chart
    // ============================================================
    {
        SvgGaugeChart chart(600, 400);
        chart.setTitle("Basic Gauge");
        chart.setGaugeType(SvgGaugeChart::TYPE_BASIC);
        chart.setGaugeTitle("CPU Usage");
        chart.setValue(72.5);
        chart.setMinMax(0, 100);
        //saveSvg("gauge_basic.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "9. Progress Gauge Chart" << "</p>";
    // ============================================================
    // 9. Progress Gauge Chart
    // ============================================================
    {
        SvgGaugeChart chart(600, 400);
        chart.setTitle("Progress Gauge");
        chart.setGaugeType(SvgGaugeChart::TYPE_PROGRESS);
        chart.setGaugeTitle("Progress");
        chart.setValue(68);
        chart.setMinMax(0, 100);
        chart.setColorStops({{0.33, SvgColor(103, 224, 227)},
                             {0.66, SvgColor(55, 162, 218)},
                             {1.0, SvgColor(250, 200, 88)}});
        //saveSvg("gauge_progress.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "10. Stage Speed Gauge Chart" << "</p>";
    // ============================================================
    // 10. Stage Speed Gauge Chart
    // ============================================================
    {
        SvgGaugeChart chart(600, 400);
        chart.setTitle("Stage Speed Gauge");
        chart.setGaugeType(SvgGaugeChart::TYPE_STAGE_SPEED);
        chart.setGaugeTitle("Speed (Mbps)");
        chart.setValue(85);
        chart.setMinMax(0, 200);
        chart.setSplitNumber(10);
        // Green-Yellow-Red speed stages
        chart.setColorStops({
            {0.25, SvgColor(103, 224, 127)},// Low - Green
            {0.5, SvgColor(250, 200, 88)},  // Medium - Yellow
            {0.75, SvgColor(252, 132, 82)}, // High - Orange
            {1.0, SvgColor(238, 102, 102)}  // Extreme - Red
        });
        //saveSvg("gauge_stage_speed.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "11. Grade Gauge Chart" << "</p>";
    // ============================================================
    // 11. Grade Gauge Chart
    //     Algorithms from ECharts GaugeView.ts (gauge-simple.html)
    //     Features: semi-circle arc (180~0), labels inside arc,
    //               tangential rotation, inherited tick colors
    // ============================================================
    {
        SvgGaugeChart chart(600, 400);
        chart.setTitle("Grade Gauge");
        chart.setGaugeType(SvgGaugeChart::TYPE_LEVEL);
        chart.setGaugeTitle("Score");
        chart.setValue(75);
        chart.setMinMax(0, 100);
        // Semi-circle arc (top) - ECharts grade gauge feature
        chart.setAngleRange(180, 0);
        // Four grades: Poor / Fair / Good / Excellent
        chart.setColorStops({
            {0.25, SvgColor(255, 110, 118)},// Poor - Red #FF6E76
            {0.5, SvgColor(253, 221, 96)},  // Fair - Yellow #FDDD60
            {0.75, SvgColor(88, 217, 249)}, // Good - Blue #58D9F9
            {1.0, SvgColor(124, 255, 178)}  // Excellent - Green #7CFFB2
        });
        chart.setSplitNumber(8);
        // Labels inside arc (negative distance), tangential rotation
        chart.setLabelDistance(-60);
        chart.setLabelRotate("tangential");
        // Custom label text at positions 0.125, 0.375, 0.625, 0.875
        chart.setLabelFormatter({
            "",         // 0: 0
            "Poor",     // 1: 12.5
            "",         // 2: 25
            "Fair",     // 3: 37.5
            "",         // 4: 50
            "Good",     // 5: 62.5
            "",         // 6: 75
            "Excellent",// 7: 87.5
            "",         // 8: 100
        });
        // Pointer length 80%
        chart.setPointerLength(0.8);
        // Ticks inherit color
        chart.setTickInheritColor(true);
        chart.setSplitLineWidth(5);
        // Title and detail offset
        chart.setTitleOffset(0, -50);
        chart.setDetailOffset(0, -30);
        //saveSvg("gauge_level.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "12. Funnel Chart" << "</p>";
    // ============================================================
    // 12. Funnel Chart
    // ============================================================
    {
        SvgFunnelChart chart(800, 500);
        chart.setTitle("Funnel Chart");
        chart.setSort("descending");
        chart.setFunnelAlign("center");
        chart.setGap(2);
        chart.addData("Visit", 10000);
        chart.addData("Add to Cart", 6000);
        chart.addData("Place Order", 3500);
        chart.addData("Pay", 2200);
        chart.addData("Complete", 1200);
        //saveSvg("funnel_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "13. Basic Candlestick Chart" << "</p>";
    // ============================================================
    // 13. Basic Candlestick Chart
    //     Algorithms from ECharts candlestickLayout.ts / CandlestickView.ts
    // ============================================================
    {
        SvgCandlestickChart chart(900, 500);
        chart.setTitle("Basic Candlestick Chart");
        chart.setXLabels({"07/14", "07/15", "07/16", "07/17", "07/18", "07/21", "07/22", "07/23", "07/24", "07/25", "07/28", "07/29", "07/30", "07/31", "08/01", "08/04", "08/05", "08/06", "08/07", "08/08"});
        chart.addData({
            {2320, 2302, 2287, 2362},// 07/14 bearish
            {2300, 2292, 2273, 2341},// 07/15 bearish
            {2295, 2310, 2280, 2315},// 07/16 bullish
            {2310, 2330, 2295, 2350},// 07/17 bullish
            {2335, 2350, 2310, 2360},// 07/18 bullish
            {2350, 2310, 2290, 2380},// 07/21 bearish
            {2310, 2280, 2260, 2330},// 07/22 bearish
            {2280, 2320, 2265, 2335},// 07/23 bullish
            {2320, 2340, 2300, 2360},// 07/24 bullish
            {2340, 2360, 2310, 2380},// 07/25 bullish
            {2360, 2340, 2320, 2390},// 07/28 bearish
            {2340, 2370, 2330, 2395},// 07/29 bullish
            {2370, 2390, 2340, 2410},// 07/30 bullish
            {2390, 2370, 2350, 2420},// 07/31 bearish
            {2370, 2380, 2360, 2400},// 08/01 bullish
            {2380, 2350, 2330, 2400},// 08/04 bearish
            {2350, 2330, 2310, 2370},// 08/05 bearish
            {2330, 2360, 2310, 2375},// 08/06 bullish
            {2360, 2380, 2340, 2400},// 08/07 bullish
            {2380, 2350, 2330, 2400},// 08/08 bearish
        });
        //saveSvg("candlestick_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "14. Bar Chart with Negative Value" << "</p>";
    // ============================================================
    // 14. Bar Chart with Negative Value
    //     Using SvgBarChart with positive/negative values, zero line at center
    // ============================================================
    {
        SvgBarChart chart(800, 500);
        chart.setTitle("Bar Chart with Negative Value");
        chart.setXLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        chart.addSeries({120, -50, 150, -80, 70, -110, 130}, "Income/Expense");
        //saveSvg("bar_negative.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "15. Alternating Labels (Bar Chart with Negative Value)" << "</p>";
    // ============================================================
    // 15. Alternating Labels (Bar Chart with Negative Value)
    //     Algorithms from ECharts bar-negative2 example
    //     Features: horizontal bars, X-axis on top, dashed dividers, alternating labels
    // ============================================================
    {
        SvgNegativeBarChart chart(600, 450);
        chart.setTitle("Alternating Labels");
        chart.addData(-0.07, "ten");
        chart.addData(-0.09, "nine");
        chart.addData(0.2, "eight");
        chart.addData(0.44, "seven");
        chart.addData(-0.23, "six");
        chart.addData(0.08, "five");
        chart.addData(-0.17, "four");
        chart.addData(0.47, "three");
        chart.addData(-0.36, "two");
        chart.addData(0.18, "one");
        //saveSvg("bar_negative2.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "16. Kernel Density Estimation" << "</p>";
    // ============================================================
    // 16. Kernel Density Estimation - Two Group Comparison
    //     Group A: 50 items, right-skewed (income-like)
    //     Group B: 60 items, symmetric, slightly left-skewed
    // ============================================================
    {
        SvgDensityChart chart(800, 500);
        chart.setTitle("Kernel Density Estimation - Two Group Comparison");

        // Group A: Right-skewed (income-like distribution, ~50 data points)
        std::vector<double> groupA = {
            42,
            45,
            48,
            50,
            52,
            53,
            55,
            55,
            56,
            57,
            58,
            58,
            59,
            60,
            60,
            61,
            61,
            62,
            62,
            63,
            63,
            64,
            64,
            65,
            65,
            66,
            67,
            68,
            69,
            70,
            71,
            72,
            73,
            74,
            75,
            77,
            78,
            80,
            82,
            84,
            86,
            88,
            90,
            93,
            96,
            100,
            105,
            110,
            118,
            130};

        // Group B: More symmetric, different center (~60 data points)
        std::vector<double> groupB = {
            38,
            40,
            42,
            43,
            44,
            45,
            46,
            47,
            48,
            48,
            49,
            49,
            50,
            50,
            51,
            51,
            52,
            52,
            52,
            53,
            53,
            53,
            54,
            54,
            54,
            55,
            55,
            55,
            55,
            56,
            56,
            56,
            56,
            57,
            57,
            57,
            58,
            58,
            58,
            59,
            59,
            60,
            60,
            61,
            61,
            62,
            62,
            63,
            64,
            65,
            66,
            67,
            68,
            69,
            70,
            72,
            74,
            76,
            78,
            82};

        chart.addSeries(groupA, "Group A (n=50)", SvgColor(84, 112, 198));
        chart.addSeries(groupB, "Group B (n=60)", SvgColor(238, 102, 102));
        chart.setShowRug(false);
        //saveSvg("density_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "17. 九宫格图 - 3x3 Grid Chart" << "</p>";
    // ============================================================
    // 17. 九宫格图 - 3x3 Grid Chart
    //     每个格子自定义颜色和文字，双轴标签
    // ============================================================
    {
        SvgGridChart chart(800, 600);
        chart.setTitle("3x3 Grid Chart - Custom Colors and Text");

        // 设置格子内容（行、列、文字、背景色、文字颜色、字体大小）
        chart.setCell(0, 0, "Item A1", SvgColor(255, 230, 230), SvgColor(180, 30, 30), 14);
        chart.setCell(0, 1, "Item A2", SvgColor(230, 255, 230), SvgColor(30, 130, 30), 14);
        chart.setCell(0, 2, "Item A3", SvgColor(230, 230, 255), SvgColor(30, 30, 180), 14);
        chart.setCell(1, 0, "Item B1", SvgColor(255, 255, 220), SvgColor(150, 120, 20), 14);
        chart.setCell(1, 1, "Item B2", SvgColor(240, 220, 255), SvgColor(130, 30, 180), 14);
        chart.setCell(1, 2, "Item B3", SvgColor(220, 255, 255), SvgColor(20, 130, 130), 14);
        chart.setCell(2, 0, "Item C1", SvgColor(255, 220, 240), SvgColor(180, 30, 120), 14);
        chart.setCell(2, 1, "Item C2", SvgColor(240, 240, 220), SvgColor(100, 100, 30), 14);
        chart.setCell(2, 2, "Item C3", SvgColor(230, 240, 255), SvgColor(50, 80, 150), 14);

        // X 轴底部标签（3 个）
        chart.setXLabels("Column 1", "Column 2", "Column 3");

        // Y 轴左侧标签（3 个）
        chart.setYLabels("Row 1", "Row 2", "Row 3");

        // 设置边框
        chart.setCellBorder("#999", 1.5);

        //saveSvg("grid_chart.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "18. 九宫格图 - 单轴标签模式" << "</p>";
    // ============================================================
    // 18. 九宫格图 - 单轴标签模式
    // ============================================================
    {
        SvgGridChart chart(800, 600);
        chart.setTitle("3x3 Grid Chart - Single Axis Labels");

        // 类似 BCG 矩阵风格
        chart.setCell(0, 0, "Star", SvgColor(100, 200, 100), SvgColor(255, 255, 255), 16);
        chart.setCell(0, 1, "Star", SvgColor(120, 210, 120), SvgColor(255, 255, 255), 16);
        chart.setCell(0, 2, "Question\nMark", SvgColor(255, 200, 100), SvgColor(255, 255, 255), 16);
        chart.setCell(1, 0, "Star", SvgColor(130, 220, 130), SvgColor(255, 255, 255), 16);
        chart.setCell(1, 1, "Question\nMark", SvgColor(255, 210, 110), SvgColor(255, 255, 255), 16);
        chart.setCell(1, 2, "Dog", SvgColor(255, 150, 150), SvgColor(255, 255, 255), 16);
        chart.setCell(2, 0, "Cash Cow", SvgColor(100, 180, 220), SvgColor(255, 255, 255), 16);
        chart.setCell(2, 1, "Dog", SvgColor(255, 160, 160), SvgColor(255, 255, 255), 16);
        chart.setCell(2, 2, "Dog", SvgColor(255, 170, 170), SvgColor(255, 255, 255), 16);

        // X 轴底部标签（1 个居中）
        chart.setXLabel("Market Growth Rate");

        // Y 轴左侧标签（1 个居中）
        chart.setYLabel("Relative Market Share");

        chart.setCellBorder("#666", 1);
        //saveSvg("grid_chart_single_label.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "19. 九宫格图 - 标题+内容模式（顶对齐）" << "</p>";
    // ============================================================
    // 19. 九宫格图 - 标题+内容模式（顶对齐）
    // ============================================================
    {
        SvgGridChart chart(800, 600);
        chart.setTitle("3x3 Grid Chart - Title + Content");

        // 使用 setCell 的 title+content 重载，标题顶对齐
        chart.setCell(0, 0, "Revenue", "12.5M\n+8.3% YoY", SvgColor(230, 245, 240), SvgColor(30, 80, 50), SvgColor(60, 140, 80), 14, 11);
        chart.setCell(0, 1, "Cost", "8.2M\n-2.1% YoY", SvgColor(245, 235, 230), SvgColor(140, 60, 30), SvgColor(180, 80, 40), 14, 11);
        chart.setCell(0, 2, "Profit", "4.3M\n+15.6% YoY", SvgColor(230, 240, 255), SvgColor(30, 50, 140), SvgColor(50, 100, 200), 14, 11);

        chart.setCell(1, 0, "Users", "48.2K\n+12.0%", SvgColor(240, 245, 230), SvgColor(80, 100, 30), SvgColor(120, 150, 40), 14, 11);
        chart.setCell(1, 1, "DAU", "12.8K\n+5.3%", SvgColor(240, 230, 245), SvgColor(100, 30, 120), SvgColor(150, 50, 180), 14, 11);
        chart.setCell(1, 2, "Retention", "72.5%\n+1.2%", SvgColor(235, 245, 245), SvgColor(20, 100, 100), SvgColor(30, 140, 140), 14, 11);

        chart.setCell(2, 0, "NPS", "68\n+3 pts", SvgColor(245, 240, 230), SvgColor(120, 90, 20), SvgColor(180, 130, 30), 14, 11);
        chart.setCell(2, 1, "CSAT", "4.8/5.0\n+0.2", SvgColor(255, 235, 240), SvgColor(150, 30, 50), SvgColor(200, 40, 70), 14, 11);
        chart.setCell(2, 2, "Churn", "3.2%\n-0.5%", SvgColor(245, 245, 240), SvgColor(100, 60, 10), SvgColor(150, 90, 20), 14, 11);

        chart.setXLabels("Q1", "Q2", "Q3");
        chart.setYLabels("Finance", "Growth", "Satisfaction");
        chart.setCellBorder("#aaa", 1.5);

        //saveSvg("grid_chart_content.svg", chart.render());
        client << chart.render();
    }

    client << "<p>" << "20. 九宫格图 - 标题居中 + 内容偏移 + Y轴不旋转" << "</p>";
    // ============================================================
    // 20. 九宫格图 - 标题居中 + 内容偏移 + Y轴不旋转
    // ============================================================
    {
        SvgGridChart chart(800, 600);
        chart.setTitle("3x3 Grid - Centered Title + Content Offset + No Rotation");

        // Y 轴标签不旋转
        chart.setYLabelRotate(false);

        chart.setCell(0, 0, "Revenue", "12.5M\n+8.3% YoY", SvgColor(230, 245, 240), SvgColor(30, 80, 50), SvgColor(60, 140, 80), 14, 11);
        chart.setCell(0, 1, "Cost", "8.2M\n-2.1% YoY", SvgColor(245, 235, 230), SvgColor(140, 60, 30), SvgColor(180, 80, 40), 14, 11);
        chart.setCell(0, 2, "Profit", "4.3M\n+15.6% YoY", SvgColor(230, 240, 255), SvgColor(30, 50, 140), SvgColor(50, 100, 200), 14, 11);

        chart.setCell(1, 0, "Users", "48.2K\n+12.0%", SvgColor(240, 245, 230), SvgColor(80, 100, 30), SvgColor(120, 150, 40), 14, 11);
        chart.setCell(1, 1, "DAU", "12.8K\n+5.3%", SvgColor(240, 230, 245), SvgColor(100, 30, 120), SvgColor(150, 50, 180), 14, 11);
        chart.setCell(1, 2, "Retention", "72.5%\n+1.2%", SvgColor(235, 245, 245), SvgColor(20, 100, 100), SvgColor(30, 140, 140), 14, 11);

        chart.setCell(2, 0, "NPS", "68\n+3 pts", SvgColor(245, 240, 230), SvgColor(120, 90, 20), SvgColor(180, 130, 30), 14, 11);
        chart.setCell(2, 1, "CSAT", "4.8/5.0\n+0.2", SvgColor(255, 235, 240), SvgColor(150, 30, 50), SvgColor(200, 40, 70), 14, 11);
        chart.setCell(2, 2, "Churn", "3.2%\n-0.5%", SvgColor(245, 245, 240), SvgColor(100, 60, 10), SvgColor(150, 90, 20), 14, 11);

        // 设置所有格子标题居中
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 3; ++c)
            {
                chart.setCellTitleAlign(r, c, "center");
            }
        }

        // 设置格子内容距标题的偏移量（默认 10px，这里加大到 18px）
        chart.setCellContentOffset(0, 0, 18);
        chart.setCellContentOffset(0, 1, 18);
        chart.setCellContentOffset(0, 2, 18);
        chart.setCellContentOffset(1, 0, 18);
        chart.setCellContentOffset(1, 1, 18);
        chart.setCellContentOffset(1, 2, 18);
        chart.setCellContentOffset(2, 0, 18);
        chart.setCellContentOffset(2, 1, 18);
        chart.setCellContentOffset(2, 2, 18);

        chart.setXLabels("Q1", "Q2", "Q3");
        chart.setYLabels("Finance", "Growth", "Satisfaction");
        chart.setCellBorder("#aaa", 1.5);

        //saveSvg("grid_chart_center.svg", chart.render());
        client << chart.render();
    }

    // ============================================================
    // 28. 热力图 - Punch Card (小时 x 星期)
    //     算法来自 ECharts HeatmapView.ts / HeatmapSeries.ts
    // ============================================================
    {
        SvgHeatmapChart chart(900, 500);
        chart.setTitle("Heatmap - Punch Card");

        std::vector<std::string> hours = {
            "12a",
            "1a",
            "2a",
            "3a",
            "4a",
            "5a",
            "6a",
            "7a",
            "8a",
            "9a",
            "10a",
            "11a",
            "12p",
            "1p",
            "2p",
            "3p",
            "4p",
            "5p",
            "6p",
            "7p",
            "8p",
            "9p",
            "10p",
            "11p"};
        std::vector<std::string> days = {
            "Saturday",
            "Friday",
            "Thursday",
            "Wednesday",
            "Tuesday",
            "Monday",
            "Sunday"};

        chart.setXLabels(hours);
        chart.setYLabels(days);

        // Data from ECharts heatmap test data
        std::vector<HeatmapDataItem> data = {
            {0, 0, 5},
            {0, 1, 1},
            {0, 2, 0},
            {0, 3, 0},
            {0, 4, 0},
            {0, 5, 0},
            {0, 6, 0},
            {0, 7, 0},
            {0, 8, 0},
            {0, 9, 0},
            {0, 10, 0},
            {0, 11, 2},
            {0, 12, 4},
            {0, 13, 1},
            {0, 14, 1},
            {0, 15, 3},
            {0, 16, 4},
            {0, 17, 6},
            {0, 18, 4},
            {0, 19, 4},
            {0, 20, 3},
            {0, 21, 3},
            {0, 22, 2},
            {0, 23, 5},
            {1, 0, 7},
            {1, 1, 0},
            {1, 2, 0},
            {1, 3, 0},
            {1, 4, 0},
            {1, 5, 0},
            {1, 6, 0},
            {1, 7, 0},
            {1, 8, 0},
            {1, 9, 0},
            {1, 10, 5},
            {1, 11, 2},
            {1, 12, 2},
            {1, 13, 6},
            {1, 14, 9},
            {1, 15, 11},
            {1, 16, 6},
            {1, 17, 7},
            {1, 18, 8},
            {1, 19, 12},
            {1, 20, 5},
            {1, 21, 5},
            {1, 22, 7},
            {1, 23, 2},
            {2, 0, 1},
            {2, 1, 1},
            {2, 2, 0},
            {2, 3, 0},
            {2, 4, 0},
            {2, 5, 0},
            {2, 6, 0},
            {2, 7, 0},
            {2, 8, 0},
            {2, 9, 0},
            {2, 10, 3},
            {2, 11, 2},
            {2, 12, 1},
            {2, 13, 9},
            {2, 14, 8},
            {2, 15, 10},
            {2, 16, 6},
            {2, 17, 5},
            {2, 18, 5},
            {2, 19, 5},
            {2, 20, 7},
            {2, 21, 4},
            {2, 22, 2},
            {2, 23, 4},
            {3, 0, 7},
            {3, 1, 3},
            {3, 2, 0},
            {3, 3, 0},
            {3, 4, 0},
            {3, 5, 0},
            {3, 6, 0},
            {3, 7, 0},
            {3, 8, 1},
            {3, 9, 0},
            {3, 10, 5},
            {3, 11, 4},
            {3, 12, 7},
            {3, 13, 14},
            {3, 14, 13},
            {3, 15, 12},
            {3, 16, 9},
            {3, 17, 5},
            {3, 18, 5},
            {3, 19, 10},
            {3, 20, 6},
            {3, 21, 4},
            {3, 22, 4},
            {3, 23, 1},
            {4, 0, 1},
            {4, 1, 3},
            {4, 2, 0},
            {4, 3, 0},
            {4, 4, 0},
            {4, 5, 1},
            {4, 6, 0},
            {4, 7, 0},
            {4, 8, 0},
            {4, 9, 2},
            {4, 10, 4},
            {4, 11, 4},
            {4, 12, 2},
            {4, 13, 4},
            {4, 14, 4},
            {4, 15, 14},
            {4, 16, 12},
            {4, 17, 1},
            {4, 18, 8},
            {4, 19, 5},
            {4, 20, 3},
            {4, 21, 7},
            {4, 22, 3},
            {4, 23, 0},
            {5, 0, 2},
            {5, 1, 1},
            {5, 2, 0},
            {5, 3, 3},
            {5, 4, 0},
            {5, 5, 0},
            {5, 6, 0},
            {5, 7, 0},
            {5, 8, 2},
            {5, 9, 0},
            {5, 10, 4},
            {5, 11, 1},
            {5, 12, 5},
            {5, 13, 10},
            {5, 14, 5},
            {5, 15, 7},
            {5, 16, 11},
            {5, 17, 6},
            {5, 18, 0},
            {5, 19, 5},
            {5, 20, 3},
            {5, 21, 4},
            {5, 22, 2},
            {5, 23, 0},
            {6, 0, 1},
            {6, 1, 0},
            {6, 2, 0},
            {6, 3, 0},
            {6, 4, 0},
            {6, 5, 0},
            {6, 6, 0},
            {6, 7, 0},
            {6, 8, 0},
            {6, 9, 0},
            {6, 10, 1},
            {6, 11, 0},
            {6, 12, 2},
            {6, 13, 1},
            {6, 14, 3},
            {6, 15, 4},
            {6, 16, 0},
            {6, 17, 0},
            {6, 18, 0},
            {6, 19, 0},
            {6, 20, 1},
            {6, 21, 2},
            {6, 22, 2},
            {6, 23, 6}};
        chart.addData(data);
        chart.setValueRange(0, 14);
        chart.setCellGap(0);
        //saveSvg("heatmap_punch_card.svg", chart.render());
        client << chart.render();
    }

    // ============================================================
    // 29. 热力图 - 自定义颜色渐变（绿-黄-红）
    // ============================================================
    {
        SvgHeatmapChart chart(900, 500);
        chart.setTitle("Heatmap - Custom Gradient (Green-Yellow-Red)");

        std::vector<std::string> hours = {
            "12a",
            "1a",
            "2a",
            "3a",
            "4a",
            "5a",
            "6a",
            "7a",
            "8a",
            "9a",
            "10a",
            "11a",
            "12p",
            "1p",
            "2p",
            "3p",
            "4p",
            "5p",
            "6p",
            "7p",
            "8p",
            "9p",
            "10p",
            "11p"};
        std::vector<std::string> days = {
            "Sat",
            "Fri",
            "Thu",
            "Wed",
            "Tue",
            "Mon",
            "Sun"};
        chart.setXLabels(hours);
        chart.setYLabels(days);

        std::vector<HeatmapDataItem> data = {
            {0, 0, 5},
            {1, 0, 7},
            {2, 0, 1},
            {3, 0, 7},
            {4, 0, 1},
            {5, 0, 2},
            {6, 0, 1},
            {0, 1, 1},
            {1, 1, 0},
            {2, 1, 1},
            {3, 1, 3},
            {4, 1, 3},
            {5, 1, 1},
            {6, 1, 0},
            {0, 2, 0},
            {1, 2, 0},
            {2, 2, 0},
            {3, 2, 0},
            {4, 2, 0},
            {5, 2, 0},
            {6, 2, 0},
            {0, 3, 0},
            {1, 3, 0},
            {2, 3, 0},
            {3, 3, 0},
            {4, 3, 0},
            {5, 3, 3},
            {6, 3, 0},
            {0, 4, 0},
            {1, 4, 0},
            {2, 4, 0},
            {3, 4, 0},
            {4, 4, 0},
            {5, 4, 0},
            {6, 4, 0},
            {0, 5, 0},
            {1, 5, 0},
            {2, 5, 0},
            {3, 5, 0},
            {4, 5, 1},
            {5, 5, 0},
            {6, 5, 0},
            {0, 6, 0},
            {1, 6, 0},
            {2, 6, 0},
            {3, 6, 0},
            {4, 6, 0},
            {5, 6, 0},
            {6, 6, 0},
            {0, 7, 0},
            {1, 7, 0},
            {2, 7, 0},
            {3, 7, 0},
            {4, 7, 0},
            {5, 7, 0},
            {6, 7, 0},
            {0, 8, 0},
            {1, 8, 0},
            {2, 8, 0},
            {3, 8, 1},
            {4, 8, 0},
            {5, 8, 2},
            {6, 8, 0},
            {0, 9, 0},
            {1, 9, 0},
            {2, 9, 0},
            {3, 9, 0},
            {4, 9, 2},
            {5, 9, 0},
            {6, 9, 0},
            {0, 10, 0},
            {1, 10, 5},
            {2, 10, 3},
            {3, 10, 5},
            {4, 10, 4},
            {5, 10, 4},
            {6, 10, 1},
            {0, 11, 2},
            {1, 11, 2},
            {2, 11, 2},
            {3, 11, 4},
            {4, 11, 4},
            {5, 11, 1},
            {6, 11, 0},
            {0, 12, 4},
            {1, 12, 2},
            {2, 12, 1},
            {3, 12, 7},
            {4, 12, 2},
            {5, 12, 5},
            {6, 12, 2},
            {0, 13, 1},
            {1, 13, 6},
            {2, 13, 9},
            {3, 13, 14},
            {4, 13, 4},
            {5, 13, 10},
            {6, 13, 1},
            {0, 14, 1},
            {1, 14, 9},
            {2, 14, 8},
            {3, 14, 13},
            {4, 14, 4},
            {5, 14, 5},
            {6, 14, 3},
            {0, 15, 3},
            {1, 15, 11},
            {2, 15, 10},
            {3, 15, 12},
            {4, 15, 14},
            {5, 15, 7},
            {6, 15, 4},
            {0, 16, 4},
            {1, 16, 6},
            {2, 16, 6},
            {3, 16, 9},
            {4, 16, 12},
            {5, 16, 11},
            {6, 16, 0},
            {0, 17, 6},
            {1, 17, 7},
            {2, 17, 5},
            {3, 17, 5},
            {4, 17, 1},
            {5, 17, 6},
            {6, 17, 0},
            {0, 18, 4},
            {1, 18, 8},
            {2, 18, 5},
            {3, 18, 5},
            {4, 18, 8},
            {5, 18, 0},
            {6, 18, 0},
            {0, 19, 4},
            {1, 19, 12},
            {2, 19, 5},
            {3, 19, 10},
            {4, 19, 5},
            {5, 19, 5},
            {6, 19, 0},
            {0, 20, 3},
            {1, 20, 5},
            {2, 20, 7},
            {3, 20, 6},
            {4, 20, 3},
            {5, 20, 3},
            {6, 20, 1},
            {0, 21, 3},
            {1, 21, 5},
            {2, 21, 4},
            {3, 21, 4},
            {4, 21, 7},
            {5, 21, 4},
            {6, 21, 2},
            {0, 22, 2},
            {1, 22, 7},
            {2, 22, 2},
            {3, 22, 4},
            {4, 22, 3},
            {5, 22, 2},
            {6, 22, 2},
            {0, 23, 5},
            {1, 23, 2},
            {2, 23, 4},
            {3, 23, 1},
            {4, 23, 0},
            {5, 23, 0},
            {6, 23, 6}};
        chart.addData(data);
        chart.setValueRange(0, 14);
        // Green-Yellow-Red gradient
        chart.setColorGradient({
            SvgColor(103, 224, 127),// green
            SvgColor(250, 200, 88), // yellow
            SvgColor(238, 102, 102) // red
        });
        chart.setCellGap(1);
        chart.setBorderRadius(2);
        //saveSvg("heatmap_custom_gradient.svg", chart.render());
        client << chart.render();
    }

    // ============================================================
    // 30. 乔哈里视窗 (Johari Window) - 2x2 象限图
    // ============================================================
    {
        SvgJohariWindowChart chart(800, 600);
        chart.setTitle("Johari Window");

        // 公开区 (Open): 自己知道、别人也知道
        chart.setQuadrant(SvgJohariWindowChart::OPEN,
                          "Open Area",
                          "Name, skills, experience\nBehaviors, attitudes\nPublic information",
                          SvgColor(200, 230, 255),// 浅蓝
                          SvgColor(30, 60, 140),  // 深蓝标题
                          SvgColor(60, 100, 180), // 蓝色内容
                          15,
                          12);

        // 盲区 (Blind): 自己不知道、别人知道
        chart.setQuadrant(SvgJohariWindowChart::BLIND,
                          "Blind Spot",
                          "Habits you don't notice\nCommunication style\nImpact on others",
                          SvgColor(255, 220, 200),// 浅橙
                          SvgColor(160, 60, 20),  // 深橙标题
                          SvgColor(200, 90, 40),  // 橙色内容
                          15,
                          12);

        // 隐藏区 (Hidden): 自己知道、别人不知道
        chart.setQuadrant(SvgJohariWindowChart::HIDDEN,
                          "Hidden Area",
                          "Private thoughts\nPersonal history\nUnshared feelings",
                          SvgColor(200, 255, 220),// 浅绿
                          SvgColor(20, 120, 50),  // 深绿标题
                          SvgColor(40, 160, 80),  // 绿色内容
                          15,
                          12);

        // 未知区 (Unknown): 自己不知道、别人也不知道
        chart.setQuadrant(SvgJohariWindowChart::UNKNOWN,
                          "Unknown Area",
                          "Untapped potential\nSubconscious motives\nFuture possibilities",
                          SvgColor(240, 230, 255),// 浅紫
                          SvgColor(100, 30, 140), // 深紫标题
                          SvgColor(140, 60, 190), // 紫色内容
                          15,
                          12);

        chart.setCellBorder("#999", 1.5);
        chart.setCellBorderRadius(6);
        chart.setContentOffset(14);
        //saveSvg("johari_window.svg", chart.render());
        client << chart.render();
    }

    // ============================================================
    // 31. 乔哈里视窗 - 无背景色 + 标题居中
    // ============================================================
    {
        SvgJohariWindowChart chart(800, 600);
        chart.setTitle("Johari Window - No Background, Centered Title");

        chart.setQuadrant(SvgJohariWindowChart::OPEN,
                          "Open Area",
                          "Known to self and others\nShared knowledge\nPublic identity",
                          SvgColor(),
                          SvgColor(30, 60, 140),
                          SvgColor(100, 140, 200),
                          15,
                          12);

        chart.setQuadrant(SvgJohariWindowChart::BLIND,
                          "Blind Spot",
                          "Unknown to self, known to others\nFeedback reveals these\nGrowth opportunities",
                          SvgColor(),
                          SvgColor(180, 80, 30),
                          SvgColor(210, 120, 60),
                          15,
                          12);

        chart.setQuadrant(SvgJohariWindowChart::HIDDEN,
                          "Hidden Area",
                          "Known to self, unknown to others\nPrivate self\nSelective disclosure",
                          SvgColor(),
                          SvgColor(20, 120, 50),
                          SvgColor(60, 160, 90),
                          15,
                          12);

        chart.setQuadrant(SvgJohariWindowChart::UNKNOWN,
                          "Unknown Area",
                          "Unknown to both\nUntapped potential\nDiscovery through experience",
                          SvgColor(),
                          SvgColor(100, 30, 140),
                          SvgColor(150, 70, 200),
                          15,
                          12);

        chart.setShowBackground(false);
        chart.setTitleAlign("center");
        chart.setCellBorder("#bbb", 1.5);
        chart.setCellBorderRadius(6);
        chart.setContentOffset(14);
        //saveSvg("johari_window_clean.svg", chart.render());
        client << chart.render();
    }

    client << "<p>All SVG charts generated successfully!</p>";
#else
    client << "<p>Please: cmake .. -DENABLE_CHART=ON </p>";
#endif// ENABLE_CHART

    return "";
}

//@urlpath(null,test_svgstats)
std::string test_svgstats(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_CHART
    client << "<h2>========== 原始数据 & 统计结果 ==========</h2>";

    // ============================================================
    // 一、LinearRegression 示例（原始数据）
    // ============================================================
    client << "<h3>LinearRegression 示例</h3>";

    // ---- 示例 1: 线性回归 y = ax + b ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        // 面积(m²) → 价格(万元)
        std::vector<std::vector<double>> data = {
            {50, 150},
            {70, 210},
            {90, 270},
            {110, 330},
            {130, 390}};
        auto result = LinearRegression::linear(data);
        oss << "<b>【线性回归】房价 = a * 面积 + b</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  斜率 a:  " << result.parameters.gradient << "<br>";
        oss << "  截距 b:  " << result.parameters.intercept << "<br>";
        oss << "  预测点:  ";
        for (const auto &p : result.points)
        {
            oss << "(" << p[0] << "," << p[1] << ") ";
        }
        oss << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 2: 指数回归 y = a * e^(bx) ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {1, 4862},
            {2, 5294},
            {3, 5934},
            {4, 7171},
            {5, 8964},
            {6, 10202},
            {7, 11962},
            {8, 14928},
            {9, 16909},
            {10, 18547},
            {11, 21617},
            {12, 26638},
            {13, 34634},
            {14, 46759},
            {15, 58478},
            {16, 67884},
            {17, 74462},
            {18, 79395}};
        auto result = LinearRegression::exponential(data);
        oss << "<b>【指数回归】GDP = a * e^(b*年份)</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  系数 a:  " << result.parameters.intercept << "<br>";
        oss << "  指数 b:  " << result.parameters.gradient << "<br>";
        oss << "  预测第 20 年: " << result.parameters.intercept
            << " * e^(" << result.parameters.gradient << " * 20) = "
            << result.parameters.intercept * std::exp(result.parameters.gradient * 20)
            << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 3: 对数回归 y = a + b*ln(x) ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {1, 5},
            {2, 12},
            {3, 17},
            {5, 22},
            {10, 29},
            {20, 35},
            {50, 42}};
        auto result = LinearRegression::logarithmic(data);
        oss << "<b>【对数回归】收益 = a + b*ln(投入)</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  截距 a:  " << result.parameters.intercept << "<br>";
        oss << "  斜率 b:  " << result.parameters.gradient << "<br>";
        oss << "  投入 100 时预测收益: " << result.parameters.intercept
            << " + " << result.parameters.gradient << " * ln(100) = "
            << result.parameters.intercept + result.parameters.gradient * std::log(100)
            << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 4: 多项式回归 y = a0 + a1*x + a2*x^2 + ... ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {0, 0},
            {1, 1.2},
            {2, 3.8},
            {3, 8.9},
            {4, 16.1},
            {5, 25.0}};
        auto result = LinearRegression::polynomial(data, 2);
        oss << "<b>【多项式回归 (order=2)】抛物线拟合</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  系数:    ";
        for (size_t i = 0; i < result.parameters.coefficients.size(); ++i)
        {
            oss << "a" << i << "=" << result.parameters.coefficients[i] << " ";
        }
        oss << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 5: 过原点线性回归 y = ax ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {1, 3.1},
            {2, 5.9},
            {3, 9.2},
            {4, 11.8},
            {5, 15.2}};
        LinearRegression::Config config;
        config.method = LinearRegression::LINEAR_THROUGH_ORIGIN;
        auto result   = LinearRegression::run(data, config);
        oss << "<b>【过原点线性回归】距离 = a * 时间</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  速度 a:  " << result.parameters.gradient << " m/s<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 6: 多维指定列 ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {1, 0, 3},
            {2, 0, 5},
            {3, 0, 7},
            {4, 0, 9},
            {5, 0, 11}};
        LinearRegression::Config config;
        config.method     = LinearRegression::LINEAR;
        config.dimensions = {0, 2};// x=第0列, y=第2列
        auto result       = LinearRegression::run(data, config);
        oss << "<b>【指定维度】x=col0, y=col2</b><br>";
        oss << "  公式:    " << result.expression << "<br>";
        oss << "  预测点:  ";
        for (const auto &p : result.points)
        {
            oss << "(" << p[0] << "," << p[1] << ") ";
        }
        oss << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ============================================================
    // 二、ClusteringProcess 示例（原始数据）
    // ============================================================
    client << "<h3>ClusteringProcess 示例</h3>";

    // ---- 示例 1: 客户分群 ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        std::vector<std::vector<double>> data = {
            {2, 1},
            {3, 2},
            {2.5, 1},// 低价值客户
            {15, 8},
            {18, 10},
            {16, 9},// 中等客户
            {50, 25},
            {55, 30},
            {48, 28}// 高价值客户
        };
        auto result = ClusteringProcess::simpleKMeans(data, 3);
        oss << "<b>【客户分群】k=3</b><br>";
        oss << "  原始数据: {消费额(千元), 月访问次数}<br>";
        for (size_t i = 0; i < data.size(); ++i)
        {
            oss << "    点" << i << ": (" << data[i][0] << ", " << data[i][1] << ")<br>";
        }
        oss << "  聚类结果:<br>";
        for (size_t c = 0; c < result.centroids.size(); ++c)
        {
            oss << "    群 " << c << " 中心: (消费="
                << result.centroids[c][0] << "k, 访问="
                << result.centroids[c][1] << "次)  "
                << result.pointsInCluster[c].size() << " 人<br>";
        }
        oss << "<br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 2: 二维散点聚类 + 每个点的分配 ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {0.1, 0.2},
            {0.2, 0.1},
            {0.3, 0.3},
            {0.1, 0.4},
            {0.5, 0.2},
            {9.8, 9.9},
            {9.9, 9.8},
            {10.1, 10.0},
            {10.0, 10.2},
            {9.7, 10.1}};
        auto result = ClusteringProcess::simpleKMeans(data, 2);
        oss << "<b>【二维散点聚类】k=2，10 个点</b><br>";
        oss << "  各点分配: ";
        for (size_t i = 0; i < result.clusterAssigned.size(); ++i)
        {
            oss << "点" << i << "→群" << result.clusterAssigned[i].clusterIndex << " ";
        }
        oss << "<br>";
        oss << "  群 0 中心: (" << result.centroids[0][0] << ", " << result.centroids[0][1] << ")<br>";
        oss << "  群 1 中心: (" << result.centroids[1][0] << ", " << result.centroids[1][1] << ")<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 3: 多维数据聚类 + 指定维度 ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {1.0, 2.0, 100},
            {1.1, 2.1, 101},
            {10.0, 20.0, 200},
            {10.1, 20.1, 201}};
        ClusteringProcess::Config config;
        config.clusterCount = 2;
        config.dimensions   = {0, 1};// 只用第 0 和第 1 列做聚类
        auto result         = ClusteringProcess::run(data, config);
        oss << "<b>【多维聚类】3D 数据，用前 2 维聚类</b><br>";
        oss << "  群 0 中心: (" << result.centroids[0][0] << ", " << result.centroids[0][1] << ")<br>";
        oss << "  群 1 中心: (" << result.centroids[1][0] << ", " << result.centroids[1][1] << ")<br>";
        oss << "  点 0,1 → 群 " << result.clusterAssigned[0].clusterIndex << "<br>";
        oss << "  点 2,3 → 群 " << result.clusterAssigned[2].clusterIndex << "<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ---- 示例 4: 每个点的距离信息 ----
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<std::vector<double>> data = {
            {0.1, 0.2},
            {1.0, 1.0},
            {0.3, 0.1},
            {9.8, 9.9},
            {9.9, 9.8},
            {10.0, 10.0}};
        auto result = ClusteringProcess::simpleKMeans(data, 2);
        oss << "<b>【距离信息】每个点到其所在群中心的距离</b><br>";
        for (size_t i = 0; i < result.clusterAssigned.size(); ++i)
        {
            oss << "  点" << i << " (" << data[i][0] << "," << data[i][1]
                << ") → 群" << result.clusterAssigned[i].clusterIndex
                << "  距离²=" << result.clusterAssigned[i].distance << "<br>";
        }
        oss << "<br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ============================================================
    // 三、KernelDensity 测试（原始数据）
    // ============================================================
    client << "<h3>KernelDensity 测试</h3>";
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);
        std::vector<double> data = {1.0, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0};
        KernelDensity kde(data, 0);
        oss << "  原始数据: ";
        for (size_t i = 0; i < data.size(); ++i)
        {
            oss << data[i] << " ";
        }
        oss << "<br>";
        oss << "  density(2.5) = " << kde.density(2.5) << "<br>";
        double q50 = kde.quantile(0.5);
        oss << "  quantile(0.5) = " << q50 << " (data range: " << kde.dataMin() << " ~ " << kde.dataMax() << ")<br>";
        oss << "  generateCurve(10).size() = " << kde.generateCurve(10).size() << "<br>";
        auto ci = kde.centralInterval(0.5);
        oss << "  centralInterval(0.5) = (" << ci.first << ", " << ci.second << ")<br><br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ============================================================
    // 四、LinearRegression 单元测试
    // ============================================================
    client << "<h3>LinearRegression 单元测试</h3>";
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);

        // Test 1: Simple linear regression y = 2x + 1
        {
            std::vector<std::vector<double>> data = {
                {1, 3},
                {2, 5},
                {3, 7},
                {4, 9},
                {5, 11}};
            auto result = LinearRegression::linear(data);
            bool pass   = std::abs(result.parameters.gradient - 2.0) < 0.01 && std::abs(result.parameters.intercept - 1.0) < 0.01;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": linear y = 2x + 1, "
                << "a=" << result.parameters.gradient << " b=" << result.parameters.intercept << "<br>";
        }

        // Test 2: Linear through origin y = 3x
        {
            std::vector<std::vector<double>> data = {
                {1, 3},
                {2, 6},
                {3, 9},
                {4, 12},
                {5, 15}};
            LinearRegression::Config config;
            config.method = LinearRegression::LINEAR_THROUGH_ORIGIN;
            auto result   = LinearRegression::run(data, config);
            bool pass     = std::abs(result.parameters.gradient - 3.0) < 0.01;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": linear through origin y = 3x, "
                << "a=" << result.parameters.gradient << "<br>";
        }

        // Test 3: Exponential regression y = 2 * e^(0.5*x)
        {
            std::vector<std::vector<double>> data;
            for (int i = 1; i <= 10; ++i)
            {
                data.push_back({static_cast<double>(i), 2.0 * std::exp(0.5 * i)});
            }
            auto result = LinearRegression::exponential(data);
            bool pass   = std::abs(result.parameters.gradient - 0.5) < 0.05 && std::abs(result.parameters.intercept - 2.0) < 0.5;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": exponential y = 2e^(0.5x), "
                << "a=" << result.parameters.intercept << " b=" << result.parameters.gradient << "<br>";
        }

        // Test 4: Logarithmic regression y = 1 + 2*ln(x)
        {
            std::vector<std::vector<double>> data;
            for (int i = 1; i <= 10; ++i)
            {
                data.push_back({static_cast<double>(i), 1.0 + 2.0 * std::log(i)});
            }
            auto result = LinearRegression::logarithmic(data);
            bool pass   = std::abs(result.parameters.intercept - 1.0) < 0.01 && std::abs(result.parameters.gradient - 2.0) < 0.01;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": logarithmic y = 1 + 2ln(x), "
                << "a=" << result.parameters.intercept << " b=" << result.parameters.gradient << "<br>";
        }

        // Test 5: Polynomial regression y = x^2
        {
            std::vector<std::vector<double>> data;
            for (int i = 0; i <= 5; ++i)
            {
                data.push_back({static_cast<double>(i), static_cast<double>(i * i)});
            }
            auto result = LinearRegression::polynomial(data, 2);
            bool pass   = result.parameters.coefficients.size() == 3 && std::abs(result.parameters.coefficients[2] - 1.0) < 0.01;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": polynomial y = x^2, "
                << "a2=" << result.parameters.coefficients[2] << "<br>";
        }

        // Test 6: Real-world data (ECharts example)
        {
            std::vector<std::vector<double>> data = {
                {1, 4862.4},
                {2, 5294.7},
                {3, 5934.5},
                {4, 7171.0},
                {5, 8964.4},
                {6, 10202.2},
                {7, 11962.5},
                {8, 14928.3},
                {9, 16909.2},
                {10, 18547.9},
                {11, 21617.8},
                {12, 26638.1},
                {13, 34634.4},
                {14, 46759.4},
                {15, 58478.1},
                {16, 67884.6},
                {17, 74462.6},
                {18, 79395.7}};
            auto result = LinearRegression::exponential(data);
            bool pass   = result.points.size() == 18;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": real-world exponential (18 points), "
                << "expression=" << result.expression << "<br>";
        }
        oss << "<br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ============================================================
    // 五、Clustering 单元测试
    // ============================================================
    client << "<h3>Clustering 单元测试</h3>";
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4);

        // Test 1: Two clearly separated clusters
        {
            std::vector<std::vector<double>> data = {
                {0.1, 0.2},
                {0.2, 0.1},
                {0.3, 0.3},
                {0.1, 0.4},
                {0.5, 0.2},
                {9.8, 9.9},
                {9.9, 9.8},
                {10.1, 10.0},
                {10.0, 10.2},
                {9.7, 10.1}};
            auto result = ClusteringProcess::simpleKMeans(data, 2);
            int c0      = result.clusterAssigned[0].clusterIndex;
            int c1      = result.clusterAssigned[5].clusterIndex;
            bool pass   = result.centroids.size() == 2 && c0 != c1;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": 2-cluster separation (c0=" << c0 << " c1=" << c1 << ")<br>";
        }

        // Test 2: Three clusters
        {
            std::vector<std::vector<double>> data = {
                {0.1, 0.2},
                {0.2, 0.3},
                {0.3, 0.1},
                {5.0, 5.1},
                {5.1, 5.0},
                {4.9, 5.2},
                {9.8, 9.9},
                {9.9, 9.7},
                {10.0, 10.1}};
            auto result      = ClusteringProcess::simpleKMeans(data, 3);
            bool allNonEmpty = true;
            for (size_t c = 0; c < result.pointsInCluster.size(); ++c)
            {
                if (result.pointsInCluster[c].size() < 1)
                    allNonEmpty = false;
            }
            bool pass = result.centroids.size() == 3 && allNonEmpty;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": 3-cluster, all non-empty<br>";
        }

        // Test 3: Multi-dimensional data
        {
            std::vector<std::vector<double>> data = {
                {1.0, 2.0, 3.0},
                {1.1, 2.1, 3.1},
                {10.0, 20.0, 30.0},
                {10.1, 20.1, 30.1}};
            auto result = ClusteringProcess::simpleKMeans(data, 2);
            bool pass   = result.centroids.size() == 2 && result.clusterAssigned[0].clusterIndex != result.clusterAssigned[2].clusterIndex;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": 3D clustering<br>";
        }

        // Test 4: Edge case - tight data
        {
            std::vector<std::vector<double>> data = {
                {1.0, 2.0},
                {1.1, 2.1},
                {0.9, 1.9}};
            auto result = ClusteringProcess::simpleKMeans(data, 2);
            bool pass   = result.centroids.size() >= 1;
            oss << "  " << (pass ? "PASS" : "FAIL") << ": tight data edge case<br>";
        }
        oss << "<br>";
        client << "<pre>" << oss.str() << "</pre>";
    }

    // ============================================================
    // 六、SVG 图表输出
    // ============================================================
    client << "<hr><h2>========== SVG 图表 ==========</h2>";

    client << "<p>21. 线性回归图 - 房价预测</p>";
    {
        SvgRegressionChart chart(800, 500);
        chart.setTitle("Linear Regression - House Price Prediction");
        chart.setData({{50, 150}, {60, 175}, {70, 210}, {80, 240}, {90, 270}, {100, 300}, {110, 330}, {120, 360}, {130, 390}, {140, 420}});
        chart.setMethod(LinearRegression::LINEAR);
        chart.setPointColor(SvgColor(84, 112, 198));
        chart.setLineColor(SvgColor(238, 102, 102));
        client << chart.render();
    }

    client << "<p>22. 指数回归图 - GDP 增长</p>";
    {
        SvgRegressionChart chart(800, 500);
        chart.setTitle("Exponential Regression - GDP Growth");
        chart.setData({{1, 2.1}, {2, 2.5}, {3, 3.1}, {4, 4.0}, {5, 5.2}, {6, 6.8}, {7, 8.9}, {8, 11.5}, {9, 14.8}, {10, 19.0}});
        chart.setMethod(LinearRegression::EXPONENTIAL);
        chart.setPointColor(SvgColor(60, 162, 131));
        chart.setLineColor(SvgColor(238, 102, 102));
        client << chart.render();
    }

    client << "<p>23. 对数回归图 - 学习曲线</p>";
    {
        SvgRegressionChart chart(800, 500);
        chart.setTitle("Logarithmic Regression - Learning Curve");
        chart.setData({{1, 20}, {2, 35}, {3, 45}, {4, 52}, {5, 57}, {6, 61}, {8, 67}, {10, 71}, {15, 78}, {20, 83}});
        chart.setMethod(LinearRegression::LOGARITHMIC);
        chart.setPointColor(SvgColor(250, 200, 88));
        chart.setLineColor(SvgColor(238, 102, 102));
        client << chart.render();
    }

    client << "<p>24. 多项式回归图 - 二次曲线</p>";
    {
        SvgRegressionChart chart(800, 500);
        chart.setTitle("Polynomial Regression - Quadratic");
        chart.setData({{0, 2.1}, {1, 7.7}, {2, 13.6}, {3, 27.2}, {4, 40.9}, {5, 61.1}, {6, 82.3}, {7, 108.5}, {8, 138.2}, {9, 170.0}});
        chart.setMethod(LinearRegression::POLYNOMIAL);
        chart.setPolynomialOrder(2);
        chart.setPointColor(SvgColor(154, 96, 180));
        chart.setLineColor(SvgColor(238, 102, 102));
        client << chart.render();
    }

    client << "<p>25. 聚类散点图 - 客户分群 (3 类)</p>";
    {
        SvgClusteringChart chart(800, 500);
        chart.setTitle("K-Means Clustering - Customer Segmentation");
        chart.setData({{2, 1}, {3, 2}, {2.5, 1.5}, {4, 3}, {3.5, 2.5}, {1.5, 1}, {2, 2.5}, {3, 1.5}, {12, 8}, {15, 10}, {14, 7}, {13, 9}, {16, 11}, {11, 8}, {15, 8.5}, {14, 10}, {45, 25}, {50, 30}, {48, 28}, {52, 32}, {55, 30}, {47, 26}, {50, 27}, {53, 31}});
        chart.setClusterCount(3);
        chart.setPointSize(7);
        chart.setCentroidSize(12);
        client << chart.render();
    }

    client << "<p>26. 聚类散点图 - 4 类</p>";
    {
        SvgClusteringChart chart(800, 500);
        chart.setTitle("K-Means Clustering - 4 Clusters");
        chart.setData({{5, 5}, {6, 4}, {4, 6}, {5, 7}, {6, 5}, {7, 4}, {4, 5}, {5, 25}, {6, 26}, {4, 24}, {5, 27}, {7, 25}, {6, 24}, {4, 26}, {25, 5}, {26, 4}, {24, 6}, {25, 7}, {27, 5}, {26, 6}, {24, 5}, {25, 25}, {26, 26}, {24, 24}, {25, 27}, {27, 25}, {26, 24}, {24, 26}});
        chart.setClusterCount(4);
        chart.setPointSize(7);
        chart.setCentroidSize(12);
        client << chart.render();
    }

    client << "<p>27. 多元线性回归 - 房价预测（面积+卧室数+房龄）</p>";
    {
        SvgRegressionChart chart(800, 500);
        chart.setTitle("Multivariate Linear Regression - House Price");
        chart.setData({{60, 2, 20, 180},
                       {80, 2, 15, 230},
                       {100, 3, 10, 310},
                       {120, 3, 8, 360},
                       {140, 3, 5, 420},
                       {160, 4, 15, 440},
                       {180, 4, 12, 500},
                       {200, 4, 10, 560},
                       {90, 2, 25, 200},
                       {110, 3, 18, 290},
                       {130, 3, 20, 340},
                       {150, 4, 8, 430},
                       {170, 4, 6, 490},
                       {190, 4, 5, 530},
                       {70, 2, 30, 160},
                       {85, 3, 22, 250},
                       {105, 3, 12, 320},
                       {125, 3, 7, 380},
                       {145, 4, 10, 410},
                       {165, 4, 8, 470}});
        chart.setMethod(LinearRegression::MULTIVARIATE_LINEAR);
        chart.setPointColor(SvgColor(84, 112, 198));
        chart.setLineColor(SvgColor(238, 102, 102));
        chart.setPointSize(6);
        client << chart.render();
    }

    client << "<p>All SVG stats tests completed!</p>";
#else
    client << "<p>Please: cmake .. -DENABLE_CHART=ON </p>";
#endif// ENABLE_CHART
    return "";
}

}// namespace http