#ifndef PZ_WORD_H
#define PZ_WORD_H

#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <string_view>
#include <cstddef>

#include "pzzip.h"  // pack_docx 使用 pz::zip（ENABLE_OFFICE 依赖 ENABLE_ZIP）

namespace pz
{
struct WORD_RUN
{
    std::string text;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    std::string color;       // w:color 的 val 值，如 "FF0000"
    std::string highlight;   // w:highlight 的 val 值，如 "yellow"
    unsigned char font_size = 0;
    std::string font_name;
};

struct WORD_IMAGE
{
    std::string path;  // 相对于 HTML 文件的图片路径
    std::vector<unsigned char> data;  // 图片二进制数据（DOCX→HTML 使用）
    std::string ext;   // 图片扩展名，如 "png", "jpg", "jpeg", "gif"
    int width_emu = 0;   // 图片宽度 EMU
    int height_emu = 0;  // 图片高度 EMU
};

struct WORD_PARAGRAPH
{
    std::vector<WORD_RUN> runs;
    unsigned char heading_level = 0;
    std::string text_align;       // text-align: left/center/right/justify
    unsigned int line_height = 0; // line-height px（0 表示不设置）
    std::vector<WORD_IMAGE> images;  // 行内图片
};

struct WORD_TABLE_CELL
{
    std::string text;
    unsigned char row_span = 1;
    unsigned char col_span = 1;
    std::string bg_color;       // background-color
    std::string text_color;     // 文字颜色
    unsigned int width = 0;     // 宽度 px
    unsigned int height = 0;    // 高度 px
    unsigned int padding = 0;   // 内边距 px
    std::string text_align;     // 水平对齐: left/center/right/both
    std::string vertical_align; // 垂直对齐: top/center/bottom
    std::vector<WORD_IMAGE> images;  // 单元格内图片
};

struct WORD_TABLE_ROW
{
    std::vector<WORD_TABLE_CELL> cells;
};

struct WORD_TABLE
{
    std::vector<WORD_TABLE_ROW> rows;
    unsigned int cols = 0;
    unsigned int width = 0;        // 表格宽度 px
    unsigned int width_pct = 0;    // 表格宽度百分比 1-100（优先于 width，0 表示未设置）
    std::string align;             // 表格对齐: left/center/right（center 即居中显示）
    std::string border_color;      // 边框颜色
    unsigned int border_size = 0;  // 边框大小 px（0 表示默认）
    std::vector<unsigned int> col_widths;  // 每列宽度（dxa 单位），空表示均分
};

enum class WORD_CONTENT_TYPE
{
    PARAGRAPH,
    TABLE
};

struct WORD_CONTENT
{
    WORD_CONTENT_TYPE type = WORD_CONTENT_TYPE::PARAGRAPH;
    WORD_PARAGRAPH paragraph;
    WORD_TABLE table;
};

class word
{
    public:
        word();
        bool read(const std::string &);
        bool read_from_unzipped(const std::string &);
        bool read_html(const std::string &html_file);
        // 直接从内存 HTML 字符串解析（新增，免临时文件）；不修改 html_base_dir，
        // 调用方可预先/事后设置 html_base_dir 指定图片相对路径解析基目录
        bool read_html_content(const std::string &html);
        bool write(const std::string &docx_file);
        // 生成 docx 并返回内存字节流（新增，免临时文件）；失败返回空串，原因见 error_msg
        std::string write_to_buffer();
        std::string to_html() const;
        std::string to_html_with_images(const std::string &output_dir) const;
        void clear();

        // 设置页边距 px（1px = 15 twips）
        void set_margins(unsigned int top, unsigned int right, unsigned int bottom, unsigned int left)
        {
            margin_top = top;
            margin_right = right;
            margin_bottom = bottom;
            margin_left = left;
        }

        size_t get_content_count() const { return contents.size(); }
        const WORD_CONTENT &get_content(size_t idx) const {
            static const WORD_CONTENT empty;
            return idx < contents.size() ? contents[idx] : empty;
        }

    public:
        std::vector<WORD_CONTENT> contents;
        std::string error_msg;
        std::string html_base_dir;  // HTML 文件所在目录，用于解析图片相对路径

        // 图片收集（DOCX 生成期间填充）
        struct COLLECTED_IMAGE
        {
            std::string rId;
            std::string zipPath;
            std::string origPath;
            std::string ext;
            int widthEmu = 0;
            int heightEmu = 0;
            std::vector<unsigned char> data;  // 直接存储二进制数据（base64 data URI 解码后使用）
        };
        static std::string get_image_ext(const std::string &path);
        static bool read_image_dims(const std::string &filePath, int &w, int &h);
        static bool read_image_dims_from_memory(const unsigned char *data, size_t len, int &w, int &h);
        static std::string base64_encode(const unsigned char *data, size_t len);
        static std::vector<unsigned char> base64_decode(const std::string &s);

        // 全局默认样式（来自 HTML <head><style> 的 body 规则），text 字段不使用
        WORD_RUN default_style;

        // 页边距 px，默认对应 1440/1800 twips 的常规边距
        unsigned int margin_top = 96;
        unsigned int margin_right = 120;
        unsigned int margin_bottom = 96;
        unsigned int margin_left = 120;

        static const size_t MAX_FILE_SIZE = 100 * 1024 * 1024;
        static const size_t MAX_ZIP_ENTRIES = 10000;
        static const size_t MAX_TOTAL_IMAGE_SIZE = 256 * 1024 * 1024;  // read() 图片数据总量上限
        static const unsigned int MAX_HEADING_LEVEL = 5;
        static const int MAX_PARSE_DEPTH = 64;  // 递归解析深度上限，防止恶意嵌套打爆栈

    private:
        // 一个 XML 元素的位置信息，next_element 解析得到
        struct XML_TAG
        {
            std::string name;        // 标签名，如 "w:p"
            size_t open_start = 0;   // '<' 的位置
            size_t open_end = 0;     // 开始标签 '>' 之后的位置
            size_t body_start = 0;   // 元素内容起始位置
            size_t body_end = 0;     // 匹配的闭合标签 '<' 的位置
            size_t next = 0;         // 整个元素结束之后的位置
            bool self_closing = false;
        };

        // 表格样式（styles.xml 中 w:type="table"），document.xml 里只引用 styleId，
        // 颜色/边框定义在样式里，解析时套用到引用它的表格
        struct WORD_TABLE_STYLE
        {
            std::string border_color;          // 表级边框颜色
            unsigned int border_size = 0;      // 边框大小 px
            std::string first_row_fill;        // 表头行背景色 (firstRow)
            std::string first_row_text_color;  // 表头行文字颜色
            std::string band_fill;             // 隔行条纹背景色 (band1Horz)
        };
        std::map<std::string, WORD_TABLE_STYLE> table_styles;  // styleId → 样式

        bool is_safe_zip_path(const std::string &path) const;
        bool next_element(const std::string &content, size_t si, size_t end, XML_TAG &tag) const;
        std::string get_tag_attr(const std::string &content, const XML_TAG &tag, std::string_view attr_name) const;
        void process_document(const std::string &file_content);
        void parse_styles(const std::string &file_content);
        void parse_body(const std::string &content, size_t begin, size_t end, int depth = 0);
        void parse_paragraph(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, int depth = 0);
        void parse_paragraph_properties(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, WORD_RUN &default_run);
        void parse_run(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, WORD_RUN &default_run);
        void parse_run_properties(const std::string &content, size_t begin, size_t end, WORD_RUN &run);
        void parse_table(const std::string &content, size_t begin, size_t end, WORD_TABLE &table);
        void parse_table_row(const std::string &content, size_t begin, size_t end, WORD_TABLE_ROW &row, std::vector<bool> &vmerge_flags);
        void parse_table_cell(const std::string &content, size_t begin, size_t end, WORD_TABLE_CELL &cell, bool &vmerge_continue);

        // HTML 解析（用于 read_html / read_html_content）
        // 解析已在内存中的完整 HTML 文本（read_html 读完文件后、read_html_content 直接复用）
        bool parse_html_string(const std::string &html_content);
        void parse_html_head_style(const std::string &css);
        void apply_css_margin(const std::string &prop, const std::string &val);
        void parse_html_body(const std::string &content, size_t begin, size_t end);
        void parse_html_block(const std::string &content, size_t begin, size_t end, const std::string &tag_name, const std::string &attrs = "");
        void parse_html_inline(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, const WORD_RUN &default_run = WORD_RUN(), int depth = 0);
        void parse_html_table(const std::string &content, size_t begin, size_t end, WORD_TABLE &table, const std::string &table_attrs = "");
        void parse_html_style(const std::string &style_str, WORD_RUN &run) const;
        void parse_html_cell_style(const std::string &style_str, WORD_TABLE_CELL &cell) const;
        void parse_html_paragraph_style(const std::string &style_str, WORD_PARAGRAPH &para) const;

        // CSS 尺寸值 "12px" → 数字
        static unsigned int parse_css_px(const std::string &val);

        // 从属性字符串中取指定属性值（用于 HTML 解析）
        std::string get_html_attr(const std::string &attrs, std::string_view attr_name) const;

        // DOCX 生成
        // 把所有 docx 条目写入已创建的 pz::zip（文件/内存模式通用，write 与 write_to_buffer 共用）
        bool pack_docx(pz::zip &zf);
        std::string gen_docx_content_types() const;
        std::string gen_docx_rels() const;
        std::string gen_docx_document_rels() const;
        std::string gen_docx_document() const;
        std::string gen_docx_styles() const;

        // 图片支持（生成期）
        mutable std::vector<COLLECTED_IMAGE> collected_images;
        mutable int next_image_rId = 100;
        std::string gen_docx_drawing(size_t img_idx, const std::string &rId, int w, int h) const;
        static std::string collect_image_private(COLLECTED_IMAGE &out, const std::string &rel_path,
                                                  const std::string &base_dir, unsigned int counter, int &next_rId);

        // 图片支持（解析期 DOCX→HTML）
        // rId → target（如 "rId5" → "media/image1.png"），在 read() 中从 _rels 解析填充
        std::map<std::string, std::string> image_rels_;
        // target → binary data，在 read() 中从 ZIP 条目提取填充
        std::map<std::string, std::vector<unsigned char>> image_datas_;

        std::string parse_drawing_rid(const std::string &content, size_t begin, size_t end) const;
        bool get_image_by_rid(const std::string &rId, std::vector<unsigned char> &out, std::string &ext) const;
};

}
#endif
