#ifndef PARSEHTML_H
#define PARSEHTML_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "parsecss.h"
#include "webpdf.h"

namespace pz {

enum class HTMLNodeType {
    TEXT,
    ELEMENT,
    BR
};

struct HTMLNode {
    HTMLNodeType type;
    std::string text;
    std::string tag;
    std::map<std::string, std::string> attrs;
    CSSStyle style;
    std::vector<std::shared_ptr<HTMLNode>> children;

    HTMLNode() : type(HTMLNodeType::TEXT) {}
    static std::shared_ptr<HTMLNode> make_text(const std::string& t) {
        auto n = std::make_shared<HTMLNode>();
        n->type = HTMLNodeType::TEXT;
        n->text = t;
        return n;
    }
    static std::shared_ptr<HTMLNode> make_elem(const std::string& tg) {
        auto n = std::make_shared<HTMLNode>();
        n->type = HTMLNodeType::ELEMENT;
        n->tag = tg;
        return n;
    }
    static std::shared_ptr<HTMLNode> make_br() {
        auto n = std::make_shared<HTMLNode>();
        n->type = HTMLNodeType::BR;
        return n;
    }
};

struct TOCEntry {
    std::string title;
    int level;        // 1-7
    int page_no;      // estimated page number
    int link_id;      // webpdf internal link ID
};

enum class LineBufferSegType {
    TEXT,
    IMAGE
};

struct LineBufferSeg {
    LineBufferSegType type;
    std::string text;
    CSSStyle style;
    double width;
    double height;
    std::string img_src;
    double img_pad_left;
    double img_pad_right;
    double img_pad_top;
    double img_pad_bottom;
    std::string href;
};

class HTMLRenderer {
public:
    HTMLRenderer(webpdf* pdf);
    ~HTMLRenderer();

    void WriteHTML(const std::string& html, double line_height = 0);
    void set_base_style(const CSSStyle& style) { base_style = style; }
    void add_stylesheet(const std::string& css);

private:
    webpdf* pdf;
    CSSParser css_parser;
    CSSStyle base_style;
    double line_h = 5;
    std::map<std::string, CSSStyle> stylesheet;
    std::vector<TOCEntry> toc_entries;
    std::vector<std::shared_ptr<HTMLNode>> root_nodes;
    bool toc_pass1 = false;
    std::string toc_title; // title from cataloguecontent div's title attribute
    int toc_page_no = 0;   // page number where TOC is rendered
    unsigned char no_header_footer = 0; // bit0=1: no header, bit1=2: no footer, 3: no both

    std::string extract_style_tag(const std::string& html);
    std::vector<std::shared_ptr<HTMLNode>> parse_html(const std::string& html);
    void parse_attributes(const std::string& tag_str, std::string& tag_name,
                          std::map<std::string, std::string>& attrs);

    void render_nodes(const std::vector<std::shared_ptr<HTMLNode>>& nodes,
                      CSSStyle current_style, const std::string& href = "",
                      double parent_width = 0);

    void apply_style(const CSSStyle& style);
    void write_flow_text(const std::string& text, const CSSStyle& style, const std::string& href);
    double get_line_height_mm(const CSSStyle& style);
    void flush_line_buffer(bool new_line_after = true);
    void render_pagefull_image(const std::string& src);
    std::vector<LineBufferSeg> line_buf_;
    double line_buf_width_ = 0;
    double line_buf_lh_ = 0;
    double line_buf_fs_mm_ = 0;
    std::string line_buf_align_;
    void render_table(const std::shared_ptr<HTMLNode>& table_node, const CSSStyle& style);
    // Grid-aware table renderer used when any cell has colspan/rowspan > 1.
    void render_table_spanned(const std::shared_ptr<HTMLNode>& table_node, const CSSStyle& style,
                              const std::vector<std::shared_ptr<HTMLNode>>& all_rows,
                              const std::vector<bool>& row_is_thead);
    // Render a single cell (td-like) with cross-page support, background, and border
    void render_td_cell(const std::string& text, double cell_w, double lh,
                        double pad_l, double pad_r, double pad_t, double pad_b,
                        const CSSStyle& style);
    void draw_rounded_rect(double x, double y, double w, double h, double r, bool fill, bool stroke, double line_width);

    // TOC methods
    void collect_toc_entries(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style);
    void estimate_pages(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style,
                        double& y_pos, int& page_no);
    void render_toc(const CSSStyle& style);
    int get_toc_level(const std::string& class_attr);

    // Special page elements
    std::shared_ptr<HTMLNode> page_cover;
    CSSStyle page_cover_style;
    std::shared_ptr<HTMLNode> page_catalogue;
    CSSStyle page_catalogue_style;
    std::shared_ptr<HTMLNode> catalogue_content;
    CSSStyle catalogue_content_style;
    std::shared_ptr<HTMLNode> page_header;
    CSSStyle page_header_style;
    std::shared_ptr<HTMLNode> page_footer;
    CSSStyle page_footer_style;
    std::vector<std::shared_ptr<HTMLNode>> content_nodes;
    int total_pages = 0;
    int content_start_page = 0;
    bool render_header_footer = false;
    int toc_match_index = 0;
    int render_depth_ = 0; // recursion guard for render_nodes
    static const int MAX_RENDER_DEPTH = 512;

    void extract_special_elements(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style);
    void replace_placeholders_in_nodes(std::vector<std::shared_ptr<HTMLNode>>& nodes, int pagenum, int pagetotal);
    std::string replace_placeholders(const std::string& text, int pagenum, int pagetotal);
    void render_page_header();
    void render_page_footer();

    static std::string str_tolower(const std::string& s);
    static std::string str_trim(const std::string& s);
};

}

#endif
