#include "parsehtml.h"
#include "parsesvg.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace pz {

static std::string fmt(double v, int decimals = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << v;
    return oss.str();
}

static std::string decode_html_entities(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '&') {
            size_t semi = s.find(';', i + 1);
            if (semi != std::string::npos && semi - i < 12) {
                std::string entity = s.substr(i + 1, semi - i - 1);
                if (entity == "lt") { result += '<'; i = semi + 1; continue; }
                else if (entity == "gt") { result += '>'; i = semi + 1; continue; }
                else if (entity == "amp") { result += '&'; i = semi + 1; continue; }
                else if (entity == "quot") { result += '"'; i = semi + 1; continue; }
                else if (entity == "apos") { result += '\''; i = semi + 1; continue; }
                else if (entity == "nbsp") { result += ' '; i = semi + 1; continue; }
                else if (!entity.empty() && entity[0] == '#') {
                    // Numeric entity: &#123; or &#x1F;
                    int cp = 0;
                    bool valid = false;
                    if (entity.size() > 1 && (entity[1] == 'x' || entity[1] == 'X')) {
                        try { cp = std::stoi(entity.substr(2), nullptr, 16); valid = true; } catch(...) {}
                    } else {
                        try { cp = std::stoi(entity.substr(1)); valid = true; } catch(...) {}
                    }
                    if (valid && cp > 0 && cp < 0x110000) {
                        // Encode as UTF-8
                        if (cp < 0x80) { result += (char)cp; }
                        else if (cp < 0x800) {
                            result += (char)(0xC0 | (cp >> 6));
                            result += (char)(0x80 | (cp & 0x3F));
                        } else if (cp < 0x10000) {
                            result += (char)(0xE0 | (cp >> 12));
                            result += (char)(0x80 | ((cp >> 6) & 0x3F));
                            result += (char)(0x80 | (cp & 0x3F));
                        } else {
                            result += (char)(0xF0 | (cp >> 18));
                            result += (char)(0x80 | ((cp >> 12) & 0x3F));
                            result += (char)(0x80 | ((cp >> 6) & 0x3F));
                            result += (char)(0x80 | (cp & 0x3F));
                        }
                        i = semi + 1; continue;
                    }
                }
            }
        }
        result += s[i];
        i++;
    }
    return result;
}

static bool is_safe_file_path(const std::string& path) {
    if (path.empty()) return false;
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '/') {
            if (i + 1 < path.size() && path[i + 1] == '/') return false;
        } else if (path[i] == '\\') {
            if (i + 1 < path.size() && path[i + 1] == '\\') return false;
        }
    }
    if (path.find("/../") != std::string::npos) return false;
    if (path.find("\\..\\") != std::string::npos) return false;
    if (path.compare(0, 3, "../") == 0) return false;
    if (path.compare(0, 3, "..\\") == 0) return false;
    if (path.compare(path.size() - 3, 3, "/..") == 0) return false;
    if (path.compare(path.size() - 3, 3, "\\..") == 0) return false;
    if (path == ".." || path == ".") return false;
    if (path[0] == '/' || path[0] == '\\') return false;
    if (path.size() >= 2 && std::isalpha((unsigned char)path[0]) && path[1] == ':') return false;
    if (path.find("://") != std::string::npos) return false;
    if (path.find("data:") == 0) return false;
    return true;
}

HTMLRenderer::HTMLRenderer(webpdf* p) : pdf(p) {}

HTMLRenderer::~HTMLRenderer() {
    if (pdf) {
        pdf->page_header_cb = nullptr;
        pdf->page_footer_cb = nullptr;
    }
}

std::string HTMLRenderer::str_tolower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return r;
}

std::string HTMLRenderer::str_trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

static uint32_t utf8_next_codepoint(const unsigned char*& s, size_t& len) {
    if (len == 0) return 0;
    uint32_t cp = 0;
    unsigned char c = s[0];
    int seq = 1;
    if (c < 0x80) { cp = c; seq = 1; }
    else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; seq = 2; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; seq = 3; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; seq = 4; }
    for (int i = 1; i < seq && (size_t)i < len; i++)
        cp = (cp << 6) | (s[i] & 0x3F);
    if (seq > (int)len) seq = (int)len;
    s += seq;
    len -= seq;
    return cp;
}

static bool is_cjk(uint32_t cp) {
    return (cp >= 0x3000 && cp <= 0x303F) ||
           (cp >= 0x3040 && cp <= 0x30FF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0xAC00 && cp <= 0xD7AF) ||
           (cp >= 0xFF00 && cp <= 0xFFEF) ||
           (cp >= 0x20000 && cp <= 0x2A6DF);
}

void HTMLRenderer::parse_attributes(const std::string& tag_str, std::string& tag_name,
                                     std::map<std::string, std::string>& attrs) {
    std::string s = str_trim(tag_str);
    size_t sp = s.find_first_of(" \t\n\r");
    if (sp == std::string::npos) {
        tag_name = str_tolower(s);
        return;
    }
    tag_name = str_tolower(s.substr(0, sp));
    std::string rest = s.substr(sp);
    size_t pos = 0;
    while (pos < rest.size()) {
        while (pos < rest.size() && std::isspace((unsigned char)rest[pos])) pos++;
        if (pos >= rest.size()) break;
        size_t eq = rest.find('=', pos);
        size_t next_sp = rest.find_first_of(" \t\n\r", pos);
        if (eq == std::string::npos || (next_sp != std::string::npos && next_sp < eq)) {
            std::string name = str_tolower(rest.substr(pos, next_sp - pos));
            attrs[name] = "";
            if (next_sp == std::string::npos) break;
            pos = next_sp;
            continue;
        }
        std::string name = str_tolower(rest.substr(pos, eq - pos));
        pos = eq + 1;
        while (pos < rest.size() && std::isspace((unsigned char)rest[pos])) pos++;
        std::string value;
        if (pos < rest.size() && (rest[pos] == '"' || rest[pos] == '\'')) {
            char quote = rest[pos];
            pos++;
            size_t end_q = rest.find(quote, pos);
            if (end_q == std::string::npos) {
                value = rest.substr(pos);
                pos = rest.size();
            } else {
                value = rest.substr(pos, end_q - pos);
                pos = end_q + 1;
            }
        } else {
            size_t end_v = rest.find_first_of(" \t\n\r>", pos);
            if (end_v == std::string::npos) {
                value = rest.substr(pos);
                pos = rest.size();
            } else {
                value = rest.substr(pos, end_v - pos);
                pos = end_v;
            }
        }
        attrs[name] = value;
    }
}

std::vector<std::shared_ptr<HTMLNode>> HTMLRenderer::parse_html(const std::string& html) {
    auto root = HTMLNode::make_elem("root");
    std::vector<std::shared_ptr<HTMLNode>> stack;
    stack.push_back(root);

    const std::string& orig = html;  // keep original for <pre> raw content
    std::string s;
            for (char c : html) {
                if (c == '\r' || c == '\n' || c == '\t') {
                    s += ' ';
                } else {
                    s += c;
                }
            }
    //std::string s = html;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t lt = s.find('<', pos);
        if (lt == std::string::npos) {
            std::string text = decode_html_entities(s.substr(pos));
            if (!text.empty())
                stack.back()->children.push_back(HTMLNode::make_text(text));
            break;
        }
        if (lt > pos) {
            std::string text = decode_html_entities(s.substr(pos, lt - pos));
            if (!text.empty())
                stack.back()->children.push_back(HTMLNode::make_text(text));
        }

        // Special handling: <pre> - extract raw content preserving newlines from original HTML
        {
            std::string low4 = str_tolower(s.substr(lt, std::min((size_t)5, s.size() - lt)));
            if (low4.size() >= 4 && low4[0] == '<' && low4[1] == 'p' && low4[2] == 'r' && low4[3] == 'e'
                && (low4.size() == 4 || low4[4] == '>' || low4[4] == ' ')) {
                size_t pre_gt = s.find('>', lt);
                if (pre_gt != std::string::npos) {
                    std::string pre_close = str_tolower(s).find("</pre>", pre_gt + 1) != std::string::npos
                        ? "</pre>" : "";
                    size_t pre_end = pre_close.empty() ? std::string::npos
                        : str_tolower(s).find(pre_close, pre_gt + 1);
                    if (pre_end != std::string::npos) {
                        size_t content_start = pre_gt + 1;
                        // Extract raw content from ORIGINAL html (preserves newlines)
                        std::string raw = orig.substr(content_start, pre_end - content_start);
                        auto pre_node = HTMLNode::make_elem("pre");
                        pre_node->children.push_back(HTMLNode::make_text(decode_html_entities(raw)));
                        stack.back()->children.push_back(pre_node);
                        pos = pre_end + pre_close.size();
                        continue;
                    }
                }
            }
        }
        if (lt + 1 < s.size() && s[lt + 1] == '!') {
            // Check for HTML comment: <!-- ... -->
            if (lt + 3 < s.size() && s[lt + 2] == '-' && s[lt + 3] == '-') {
                size_t end_comment = s.find("-->", lt + 4);
                if (end_comment == std::string::npos) break;
                pos = end_comment + 3;
            } else {
                size_t gt = s.find('>', lt);
                if (gt == std::string::npos) break;
                pos = gt + 1;
            }
            continue;
        }
        if (lt + 1 < s.size() && s[lt + 1] == '?') {
            size_t gt = s.find('>', lt);
            if (gt == std::string::npos) break;
            pos = gt + 1;
            continue;
        }
        bool is_close = (lt + 1 < s.size() && s[lt + 1] == '/');
        size_t gt = s.find('>', lt);
        if (gt == std::string::npos) break;
        std::string tag_content = is_close ? s.substr(lt + 2, gt - lt - 2) : s.substr(lt + 1, gt - lt - 1);

        bool self_closing = false;
        if (!tag_content.empty() && tag_content.back() == '/') {
            self_closing = true;
            tag_content = tag_content.substr(0, tag_content.size() - 1);
        }

        if (is_close) {
            std::string close_tag = str_tolower(str_trim(tag_content));
            while (!stack.empty() && stack.back()->tag != close_tag && stack.back()->tag != "root")
                stack.pop_back();
            if (!stack.empty() && stack.size() > 1 && stack.back()->tag == close_tag)
                stack.pop_back();
        } else {
            std::string tag_name;
            std::map<std::string, std::string> attrs;
            parse_attributes(tag_content, tag_name, attrs);

            if (tag_name == "style") {
                size_t end_style = str_tolower(s).find("</style>", gt);
                if (end_style == std::string::npos) break;
                pos = end_style + 8;
                continue;
            }

            if (tag_name == "br") {
                stack.back()->children.push_back(HTMLNode::make_br());
                pos = gt + 1;
                continue;
            }

            auto node = HTMLNode::make_elem(tag_name);
            node->attrs = attrs;

            auto it = attrs.find("style");
            if (it != attrs.end())
                node->style = css_parser.parse_inline(it->second);

            bool is_self_closing = (self_closing || tag_name == "img" || tag_name == "hr" ||
                                    tag_name == "input" || tag_name == "meta" || tag_name == "link");

            stack.back()->children.push_back(node);
            if (!is_self_closing)
                stack.push_back(node);
        }
        pos = gt + 1;
    }
    return root->children;
}

double HTMLRenderer::get_line_height_mm(const CSSStyle& style) {
    if (style.line_height > 0)
        return style.line_height / pdf->k;
    double fs = style.font_size > 0 ? style.font_size : pdf->FontSizePt;
    return fs * 1.2 / pdf->k;
}

void HTMLRenderer::draw_rounded_rect(double x, double y, double w, double h, double r, bool fill, bool stroke, double line_width) {
    r = std::min(r, std::min(w / 2, h / 2));
    double k_bezier = 0.5522847498 * r;

    pdf->_out("q");
    pdf->_out(fmt(pdf->k) + " 0 0 " + fmt(-pdf->k) + " 0 " + fmt(pdf->h * pdf->k) + " cm");

    std::string path;
    path += fmt(x + r) + " " + fmt(y) + " m ";
    path += fmt(x + w - r) + " " + fmt(y) + " l ";
    path += fmt(x + w - r + k_bezier) + " " + fmt(y) + " " +
            fmt(x + w) + " " + fmt(y + r - k_bezier) + " " +
            fmt(x + w) + " " + fmt(y + r) + " c ";
    path += fmt(x + w) + " " + fmt(y + h - r) + " l ";
    path += fmt(x + w) + " " + fmt(y + h - r + k_bezier) + " " +
            fmt(x + w - r + k_bezier) + " " + fmt(y + h) + " " +
            fmt(x + w - r) + " " + fmt(y + h) + " c ";
    path += fmt(x + r) + " " + fmt(y + h) + " l ";
    path += fmt(x + r - k_bezier) + " " + fmt(y + h) + " " +
            fmt(x) + " " + fmt(y + h - r + k_bezier) + " " +
            fmt(x) + " " + fmt(y + h - r) + " c ";
    path += fmt(x) + " " + fmt(y + r) + " l ";
    path += fmt(x) + " " + fmt(y + r - k_bezier) + " " +
            fmt(x + r - k_bezier) + " " + fmt(y) + " " +
            fmt(x + r) + " " + fmt(y) + " c ";
    path += "h";

    pdf->_out(path);
    if (fill && stroke)
        pdf->_out("B");
    else if (fill)
        pdf->_out("f");
    else if (stroke) {
        pdf->_out(fmt(line_width) + " w");
        pdf->_out("S");
    }
    pdf->_out("Q");
}

void HTMLRenderer::apply_style(const CSSStyle& style) {
    std::string family;
    std::string st;
    double sz = 0;

    if (!style.font_family.empty())
        family = style.font_family;
    if (style.bold) st += "B";
    if (style.italic) st += "I";
    if (style.underline) st += "U";
    if (style.font_size > 0)
        sz = style.font_size;

    if (!family.empty() || !st.empty() || sz > 0) {
        bool bold_success = true;
        try {
            pdf->SetFont(family, st, sz);
        } catch (const WebPDFException&) {
            if (!st.empty()) {
                std::string fallback_st;
                if (style.italic) fallback_st += "I";
                if (style.underline) fallback_st += "U";
                try {
                    pdf->SetFont(family, fallback_st, sz);
                    if (style.bold)
                        bold_success = false;
                } catch (const WebPDFException&) {
                    try {
                        pdf->SetFont(family, "", sz);
                        if (style.bold)
                            bold_success = false;
                    } catch (const WebPDFException&) {
                    }
                }
            }
        }
        if (style.bold && !bold_success) {
            pdf->_out("2 Tr 0.3 w");
        } else {
            pdf->_out("0 Tr");
        }
    }

    if (!style.color.empty()) {
        std::istringstream iss(style.color);
        int r = 0, g = 0, b = 0;
        iss >> r >> g >> b;
        pdf->SetTextColor(r, g, b);
    }
}

void HTMLRenderer::flush_line_buffer(bool new_line_after) {
    if (line_buf_.empty()) return;

    if (pdf->AutoPageBreak && !pdf->InHeader && !pdf->InFooter) {
        double text_bottom = pdf->GetY() + 0.5 * line_buf_lh_ + 0.6 * line_buf_fs_mm_;
        if (text_bottom > pdf->PageBreakTrigger) {
            pdf->AddPage();
        }
    }

    double content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
    double x = pdf->GetX();
    double y_base = pdf->GetY();

    if (x <= pdf->lMargin + 0.001) {
        if (line_buf_align_ == "center") {
            x = pdf->lMargin + (content_w - line_buf_width_) / 2;
        } else if (line_buf_align_ == "right") {
            x = pdf->lMargin + content_w - line_buf_width_;
        }
    }

    double cur_x = x;
    for (auto& seg : line_buf_) {
        if (seg.type == LineBufferSegType::TEXT) {
            apply_style(seg.style);
            double seg_y = y_base + 0.5 * line_buf_lh_ + 0.3 * (seg.style.font_size > 0 ? seg.style.font_size * 0.352778 : pdf->FontSizePt * 0.352778);
            double tx = cur_x + seg.img_pad_left;
            pdf->Text(tx, seg_y, seg.text);
            if (!seg.href.empty()) {
                double link_h = line_buf_lh_;
                double link_y = y_base;
                pdf->Link(tx, link_y, seg.width, link_h, seg.href);
            }
        } else if (seg.type == LineBufferSegType::IMAGE) {
            double img_y = y_base + (line_buf_lh_ - seg.height) / 2.0 + seg.img_pad_top - seg.img_pad_bottom;
            if (img_y < y_base) img_y = y_base;
            double img_x = cur_x + seg.img_pad_left;
            double img_w = seg.width - seg.img_pad_left - seg.img_pad_right;
            double img_h = seg.height - seg.img_pad_top - seg.img_pad_bottom;
            bool is_svg = (seg.img_src.size() > 4 && str_tolower(seg.img_src.substr(seg.img_src.size() - 4)) == ".svg");
            if (is_svg) {
                std::ifstream svg_file(pdf->_resolve_image_path(seg.img_src));
                if (svg_file.is_open()) {
                    std::stringstream ss;
                    ss << svg_file.rdbuf();
                    std::string svg_content = ss.str();
                    auto svg_nodes = parse_html(svg_content);
                    for (auto& n : svg_nodes) {
                        if (n->type == HTMLNodeType::ELEMENT && n->tag == "svg") {
                            double svg_w = 0, svg_h = 0;
                            auto sw_it = n->attrs.find("width");
                            if (sw_it != n->attrs.end()) { try { svg_w = std::stod(sw_it->second); } catch(...) {} }
                            auto sh_it = n->attrs.find("height");
                            if (sh_it != n->attrs.end()) { try { svg_h = std::stod(sh_it->second); } catch(...) {} }
                            double px2mm = 0.75 / pdf->k;
                            double orig_w = svg_w * px2mm;
                            double orig_h = svg_h * px2mm;
                            double scale = 1.0;
                            if (orig_w > 0 && orig_h > 0) {
                                double scale_w = img_w / orig_w;
                                double scale_h = img_h / orig_h;
                                scale = std::min(scale_w, scale_h);
                            }
                            double draw_w = orig_w * scale;
                            double draw_h = orig_h * scale;
                            double draw_x = img_x + (img_w - draw_w) / 2.0;
                            double draw_y = img_y + (img_h - draw_h) / 2.0;
                            SVGRenderer svg_renderer(pdf);
                            svg_renderer.render(n, draw_x, draw_y, draw_w, draw_h);
                            break;
                        }
                    }
                }
            } else {
                pdf->Image(seg.img_src, img_x, img_y, img_w, img_h);
            }
        }
        cur_x += seg.width;
    }

    pdf->SetX(x + line_buf_width_);

    line_buf_.clear();
    line_buf_width_ = 0;

    if (new_line_after) {
        pdf->Ln(line_buf_lh_);
    }
}

void HTMLRenderer::render_pagefull_image(const std::string& src) {
    if (src.empty()) return;

    flush_line_buffer(true);
    line_buf_lh_ = 0;
    line_buf_fs_mm_ = 0;
    line_buf_align_.clear();

    double saved_lm = pdf->lMargin;
    double saved_rm = pdf->rMargin;
    double saved_tm = pdf->tMargin;
    double saved_bm = pdf->bMargin;
    bool saved_auto = pdf->AutoPageBreak;

    double page_w = pdf->GetPageWidth();
    double page_h = pdf->GetPageHeight();

    bool page_empty = (pdf->GetY() <= saved_tm + 0.01);
    
    // If current page has content, start a fresh page first.
    // The new page will have a header from AddPage(), but we'll clear it below.
    if (!page_empty) {
        pdf->AddPage();
        page_empty = true;
    }
    
    // Now page is always "empty" — remove only the header content
    // (keep the initial graphics state written by AddPage)
    pdf->ClearHeaderFromPage();
    pdf->SetShowHeader(false);
    pdf->SetShowFooter(false);
    pdf->SetLeftMargin(0);
    pdf->SetRightMargin(0);
    pdf->SetAutoPageBreak(false);
    pdf->SetXY(0, 0);

    bool is_svg_pf = (src.size() > 4 && str_tolower(src.substr(src.size() - 4)) == ".svg");

    if (is_svg_pf) {
        std::ifstream svg_file(pdf->_resolve_image_path(src));
        if (svg_file.is_open()) {
            std::stringstream ss;
            ss << svg_file.rdbuf();
            std::string svg_content = ss.str();
            auto svg_nodes = parse_html(svg_content);
            for (auto& n : svg_nodes) {
                if (n->type == HTMLNodeType::ELEMENT && n->tag == "svg") {
                    double svg_w = 0, svg_h = 0;
                    auto sw_it = n->attrs.find("width");
                    if (sw_it != n->attrs.end()) { try { svg_w = std::stod(sw_it->second); } catch(...) {} }
                    auto sh_it = n->attrs.find("height");
                    if (sh_it != n->attrs.end()) { try { svg_h = std::stod(sh_it->second); } catch(...) {} }
                    double px2mm = 0.75 / pdf->k;
                    double orig_w = svg_w * px2mm;
                    double orig_h = svg_h * px2mm;
                    double scale = 1.0;
                    if (orig_w > 0 && orig_h > 0) {
                        double scale_w = page_w / orig_w;
                        double scale_h = page_h / orig_h;
                        scale = std::min(scale_w, scale_h);
                    }
                    double draw_w = orig_w * scale;
                    double draw_h = orig_h * scale;
                    double draw_x = (page_w - draw_w) / 2.0;
                    double draw_y = (page_h - draw_h) / 2.0;
                    SVGRenderer svg_renderer(pdf);
                    svg_renderer.render(n, draw_x, draw_y, draw_w, draw_h);
                    break;
                }
            }
        }
    } else {
        double real_w = 0, real_h = 0;
        double draw_w = page_w;
        double draw_h = page_h;
        if (pdf->GetImageSize(src, real_w, real_h) && real_w > 0 && real_h > 0) {
            double ratio_w = page_w / real_w;
            double ratio_h = page_h / real_h;
            double ratio = std::min(ratio_w, ratio_h);
            draw_w = real_w * ratio;
            draw_h = real_h * ratio;
        }
        double draw_x = (page_w - draw_w) / 2.0;
        double draw_y = (page_h - draw_h) / 2.0;
        pdf->Image(src, draw_x, draw_y, draw_w, draw_h);
    }

    pdf->SetLeftMargin(saved_lm);
    pdf->SetRightMargin(saved_rm);
    pdf->SetAutoPageBreak(saved_auto, saved_bm);
    pdf->SetShowFooter(false);
    pdf->SetShowHeader(true);
    pdf->AddPage();
    pdf->SetShowFooter(true);
    pdf->SetXY(saved_lm, saved_tm);
}

void HTMLRenderer::write_flow_text(const std::string& text, const CSSStyle& style, const std::string& href) {
    if (text.empty()) return;
    double lh = get_line_height_mm(style);
    double fs_pt = style.font_size > 0 ? style.font_size : pdf->FontSizePt;
    double fs_mm = fs_pt * 0.352778;
    std::string cur_href = href;

    if (line_buf_lh_ < lh) line_buf_lh_ = lh;
    if (line_buf_fs_mm_ < fs_mm) line_buf_fs_mm_ = fs_mm;
    if (line_buf_align_.empty()) line_buf_align_ = style.text_align;

    double avail_w;
    double total_avail_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
    if (!line_buf_.empty()) {
        avail_w = total_avail_w - line_buf_width_;
    } else {
        avail_w = pdf->GetPageWidth() - pdf->GetX() - pdf->rMargin;
        if (avail_w <= 0) {
            pdf->Ln(lh);
            avail_w = total_avail_w;
        }
    }
    const unsigned char* p = (const unsigned char*)text.c_str();
    size_t remaining = text.size();
    std::string cur_line;
    double cur_line_w = 0;
    std::string cur_word;
    double cur_word_w = 0;
    bool first_line = true;

    auto output_complete_line = [&](const std::string& line_text, double line_w) {
        if (line_text.empty() && line_buf_.empty()) return;
        if (pdf->AutoPageBreak && !pdf->InHeader && !pdf->InFooter) {
            double text_bottom = pdf->GetY() + 0.5 * line_buf_lh_ + 0.6 * line_buf_fs_mm_;
            if (text_bottom > pdf->PageBreakTrigger) {
                pdf->AddPage();
                avail_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
            }
        }
        if (!line_text.empty()) {
            LineBufferSeg seg;
            seg.type = LineBufferSegType::TEXT;
            seg.text = line_text;
            seg.style = style;
            seg.width = line_w;
            seg.height = 0;
            seg.img_pad_left = 0;
            seg.img_pad_right = 0;
            seg.img_pad_top = 0;
            seg.img_pad_bottom = 0;
            seg.href = cur_href;
            line_buf_.push_back(seg);
            line_buf_width_ += line_w;
        }
        flush_line_buffer(true);
        line_buf_lh_ = lh;
        line_buf_fs_mm_ = fs_mm;
        line_buf_align_ = style.text_align;
        avail_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
    };

    auto flush_word = [&]() {
        if (cur_word.empty()) return;
        
        int ascii_count = 0;
        for (unsigned char c : cur_word) {
            if (c < 0x80) ascii_count++;
        }
        bool is_long_word = (ascii_count > 20);
        
        if (line_buf_width_ + cur_line_w + cur_word_w > total_avail_w + 0.001 || is_long_word) {
            double line_remain = total_avail_w - line_buf_width_ - cur_line_w;
            if (line_remain < 0) line_remain = 0;
            
            const unsigned char* wp = (const unsigned char*)cur_word.c_str();
            size_t wr = cur_word.size();
            std::string part;
            double part_w = 0;
            bool first_part = true;
            
            while (wr > 0) {
                const unsigned char* old_wp = wp;
                size_t old_wr = wr;
                uint32_t cp = utf8_next_codepoint(wp, wr);
                if (cp == 0 && wp == old_wp) break;
                int cb = (int)(old_wr - wr);
                std::string cstr((const char*)old_wp, cb);
                double cw_val = pdf->GetStringWidth(cstr);
                
                double cur_avail = first_part ? line_remain : total_avail_w;
                if (part_w + cw_val > cur_avail + 0.001 && !part.empty()) {
                    if (first_part) {
                        cur_line += part;
                        cur_line_w += part_w;
                        output_complete_line(cur_line, cur_line_w);
                        cur_line.clear();
                        cur_line_w = 0;
                    } else {
                        output_complete_line(part, part_w);
                    }
                    first_line = false;
                    first_part = false;
                    part.clear();
                    part_w = 0;
                }
                part += cstr;
                part_w += cw_val;
            }
            
            if (!part.empty()) {
                cur_line += part;
                cur_line_w += part_w;
            }
        } else {
            double add_w = line_buf_width_ + cur_line_w + cur_word_w;
            if (add_w <= total_avail_w + 0.001) {
                cur_line += cur_word;
                cur_line_w += cur_word_w;
            } else {
                if (!cur_line.empty() || !line_buf_.empty()) {
                    output_complete_line(cur_line, cur_line_w);
                    first_line = false;
                }
                cur_line = cur_word;
                cur_line_w = cur_word_w;
            }
        }
        cur_word.clear();
        cur_word_w = 0;
    };

    while (remaining > 0) {
        const unsigned char* old_p = p;
        size_t old_remaining = remaining;
        uint32_t cp = utf8_next_codepoint(p, remaining);
        if (cp == 0 && p == old_p) break;

        int bytes_consumed = (int)(old_remaining - remaining);
        std::string char_utf8((const char*)old_p, bytes_consumed);

        double cw = pdf->GetStringWidth(char_utf8);
        bool is_newline = (cp == '\n');
        bool is_space = (cp == ' ' || cp == '\t' || cp == '\r') || is_newline;
        bool cjk = is_cjk(cp);

        if (is_newline) {
            // Hard line break (from <pre> content)
            flush_word();
            output_complete_line(cur_line, cur_line_w);
            cur_line.clear();
            cur_line_w = 0;
            first_line = false;
        } else if (is_space) {
            flush_word();
            if (cur_line.empty() && line_buf_.empty()) {
                continue;
            }
            double add_w = line_buf_width_ + cur_line_w + cw;
            if (add_w <= total_avail_w + 0.001) {
                cur_line += " ";
                cur_line_w += cw;
            } else {
                output_complete_line(cur_line, cur_line_w);
                cur_line.clear();
                cur_line_w = 0;
                first_line = false;
            }
        } else if (cjk) {
            flush_word();
            double add_w = line_buf_width_ + cur_line_w + cw;
            if (add_w <= total_avail_w + 0.001 || (cur_line.empty() && line_buf_.empty())) {
                cur_line += char_utf8;
                cur_line_w += cw;
            } else {
                output_complete_line(cur_line, cur_line_w);
                cur_line = char_utf8;
                cur_line_w = cw;
                first_line = false;
            }
        } else {
            double add_w = line_buf_width_ + cur_line_w + cur_word_w + cw;
            if (add_w > total_avail_w + 0.001 && !cur_word.empty()) {
                int ascii_cnt = 0;
                for (unsigned char c : cur_word) {
                    if (c < 0x80) ascii_cnt++;
                }
                if (ascii_cnt > 20) {
                    flush_word();
                } else {
                    if (!cur_line.empty() || !line_buf_.empty()) {
                        output_complete_line(cur_line, cur_line_w);
                        first_line = false;
                    }
                    cur_line = cur_word;
                    cur_line_w = cur_word_w;
                    cur_word.clear();
                    cur_word_w = 0;
                }
            }
            cur_word += char_utf8;
            cur_word_w += cw;
        }
    }

    flush_word();

    if (!cur_line.empty()) {
        LineBufferSeg seg;
        seg.type = LineBufferSegType::TEXT;
        seg.text = cur_line;
        seg.style = style;
        seg.width = cur_line_w;
        seg.height = 0;
        seg.img_pad_left = 0;
        seg.img_pad_right = 0;
        seg.img_pad_top = 0;
        seg.img_pad_bottom = 0;
        seg.href = cur_href;
        line_buf_.push_back(seg);
        line_buf_width_ += cur_line_w;
    }
}


// Collect rows grouped by section (thead, tbody, tfoot)
struct TableSection {
    std::string tag; // "thead", "tbody", "tfoot"
    std::vector<std::shared_ptr<HTMLNode>> rows;
};

static void collect_sections(const std::vector<std::shared_ptr<HTMLNode>>& nodes,
                             std::vector<TableSection>& sections) {
    bool found_section = false;
    for (auto& n : nodes) {
        if (n->type == HTMLNodeType::ELEMENT) {
            if (n->tag == "thead" || n->tag == "tbody" || n->tag == "tfoot") {
                found_section = true;
                TableSection sec;
                sec.tag = n->tag;
                for (auto& c : n->children) {
                    if (c->type == HTMLNodeType::ELEMENT && c->tag == "tr") {
                        sec.rows.push_back(c);
                    }
                }
                sections.push_back(sec);
            } else if (n->tag == "tr") {
                // tr directly under table, treat as implicit tbody
                found_section = true;
                if (sections.empty() || !sections.back().tag.empty()) {
                    TableSection sec;
                    sec.tag = "tbody";
                    sections.push_back(sec);
                }
                sections.back().rows.push_back(n);
            }
        }
    }
    if (!found_section) {
        // No sections found, collect all rows as implicit tbody
        TableSection sec;
        sec.tag = "tbody";
        for (auto& n : nodes) {
            if (n->type == HTMLNodeType::ELEMENT && n->tag == "tr") {
                sec.rows.push_back(n);
            }
        }
        if (!sec.rows.empty()) sections.push_back(sec);
    }
}

static int count_cols(const std::shared_ptr<HTMLNode>& tr) {
    int count = 0;
    for (auto& c : tr->children) {
        if (c->type == HTMLNodeType::ELEMENT && (c->tag == "td" || c->tag == "th")) {
            count++;
        }
    }
    return count;
}

static std::string node_to_text(const std::shared_ptr<HTMLNode>& node) {
    std::string result;
    if (node->type == HTMLNodeType::TEXT) {
        result += node->text;
    } else if (node->type == HTMLNodeType::ELEMENT) {
        if (node->tag == "br") {
            result += '\f';
        } else {
            for (auto& c : node->children) {
                result += node_to_text(c);
            }
        }
    }
    return result;
}

// Trim and collapse whitespace for table cell text (preserve \f for <br>)
static std::string trim_cell_text(const std::string& s) {
    std::string result;
    bool last_was_space = true; // start true to trim leading spaces
    for (char c : s) {
        if (c == '\f' || c == '\n') {
            // Trim trailing space before line break
            while (!result.empty() && result.back() == ' ') result.pop_back();
            result += c;
            last_was_space = true; // treat as boundary
        } else if (c == ' ' || c == '\t') {
            if (!last_was_space) {
                result += ' ';
                last_was_space = true;
            }
        } else {
            result += c;
            last_was_space = false;
        }
    }
    // Trim trailing spaces
    while (!result.empty() && result.back() == ' ') result.pop_back();
    // Trim trailing line break
    while (!result.empty() && (result.back() == '\f' || result.back() == '\n')) {
        // Only trim if it was followed by only spaces (already trimmed)
        if (result.size() > 1 && (result[result.size()-2] == '\f' || result[result.size()-2] == '\n'))
            result.pop_back();
        else
            break;
    }
    return result;
}

static std::shared_ptr<HTMLNode> clone_nodes(const std::shared_ptr<HTMLNode>& node) {
    if (!node) return nullptr;
    auto cloned = std::make_shared<HTMLNode>();
    cloned->type = node->type;
    cloned->text = node->text;
    cloned->tag = node->tag;
    cloned->attrs = node->attrs;
    cloned->style = node->style;
    for (auto& child : node->children) {
        cloned->children.push_back(clone_nodes(child));
    }
    return cloned;
}

static bool has_class(const std::shared_ptr<HTMLNode>& node, const std::string& class_name) {
    if (node->type != HTMLNodeType::ELEMENT) return false;
    auto it = node->attrs.find("class");
    if (it == node->attrs.end()) return false;
    std::istringstream iss(it->second);
    std::string c;
    while (iss >> c) {
        if (c == class_name) return true;
    }
    return false;
}

static std::shared_ptr<HTMLNode> find_first_img(const std::shared_ptr<HTMLNode>& node) {
    if (!node) return nullptr;
    if (node->type == HTMLNodeType::ELEMENT && node->tag == "img") {
        return node;
    }
    for (auto& child : node->children) {
        auto found = find_first_img(child);
        if (found) return found;
    }
    return nullptr;
}

static bool is_empty_node(const std::shared_ptr<HTMLNode>& node) {
    if (!node) return true;
    if (node->type == HTMLNodeType::TEXT) {
        for (char c : node->text) {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                return false;
        }
        return true;
    }
    if (node->type == HTMLNodeType::BR) {
        return false;
    }
    for (auto& child : node->children) {
        if (!is_empty_node(child))
            return false;
    }
    return true;
}

static void find_special_elements(
    const std::vector<std::shared_ptr<HTMLNode>>& nodes,
    std::shared_ptr<HTMLNode>& page_cover,
    std::shared_ptr<HTMLNode>& page_catalogue,
    std::shared_ptr<HTMLNode>& catalogue_content,
    std::shared_ptr<HTMLNode>& page_header,
    std::shared_ptr<HTMLNode>& page_footer,
    std::vector<std::shared_ptr<HTMLNode>>& content_nodes) {

    for (auto& node : nodes) {
        if (node->type != HTMLNodeType::ELEMENT) {
            content_nodes.push_back(node);
            continue;
        }
        const std::string& tag = node->tag;
        if (tag == "head" || tag == "title" || tag == "meta" || tag == "link") {
            continue;
        }
        if (tag == "html" || tag == "body") {
            find_special_elements(node->children,
                page_cover, page_catalogue, catalogue_content, page_header, page_footer, content_nodes);
            continue;
        }
        if (tag == "div") {
            if (has_class(node, "pagecover")) {
                page_cover = node;
                continue;
            }
            if (has_class(node, "pagecatalogue")) {
                page_catalogue = node;
                continue;
            }
            if (has_class(node, "cataloguecontent")) {
                catalogue_content = node;
                continue;
            }
            if (has_class(node, "pageheader")) {
                page_header = node;
                continue;
            }
            if (has_class(node, "pagefooter")) {
                page_footer = node;
                continue;
            }
        }
        content_nodes.push_back(node);
    }
}

void HTMLRenderer::render_table(const std::shared_ptr<HTMLNode>& table_node, const CSSStyle& style) {
    std::vector<TableSection> sections;
    collect_sections(table_node->children, sections);
    if (sections.empty()) return;

    // Collect all rows for column count and width calculation
    std::vector<std::shared_ptr<HTMLNode>> all_rows;
    for (auto& sec : sections) {
        for (auto& r : sec.rows) all_rows.push_back(r);
    }
    if (all_rows.empty()) return;

    int num_cols = 0;
    for (auto& row : all_rows) {
        int c = count_cols(row);
        if (c > num_cols) num_cols = c;
    }
    if (num_cols == 0) return;

    // Calculate column widths from content and TD/TH style
    std::vector<double> col_text_widths(num_cols, 0); // max content width (text + padding)
    std::vector<double> col_th_widths(num_cols, 0);   // TH explicit style width
    std::vector<bool> col_has_th_width(num_cols, false);
    std::vector<double> col_td_widths(num_cols, 0);   // TD explicit style width
    std::vector<bool> col_has_td_width(num_cols, false);
    std::vector<double> col_pct_widths(num_cols, 0);   // percentage width (0-100)
    double default_pad = 2.0; // 2mm default padding per cell side
    double min_col_w = pdf->GetStringWidth("W") + default_pad * 2; // min width = one char

    // Save font state before measuring
    std::string saved_family = pdf->FontFamily;
    std::string saved_style = pdf->FontStyle;
    double saved_size = pdf->FontSize;
    std::string saved_text_color = pdf->TextColor;

    for (auto& row : all_rows) {
        std::vector<std::shared_ptr<HTMLNode>> cells;
        for (auto& c : row->children) {
            if (c->type == HTMLNodeType::ELEMENT && (c->tag == "td" || c->tag == "th")) {
                cells.push_back(c);
            }
        }
        for (int i = 0; i < (int)cells.size() && i < num_cols; i++) {
            // Get cell style for padding and font
            CSSStyle td_style;
            auto sit2 = stylesheet.find(cells[i]->tag);
            if (sit2 != stylesheet.end())
                css_parser.merge_style(td_style, sit2->second);
            css_parser.merge_style(td_style, cells[i]->style);
            if (cells[i]->tag == "th") td_style.bold = true;
            apply_style(td_style);

            double pad_l = td_style.padding_left > 0 ? td_style.padding_left / pdf->k : default_pad;
            double pad_r = td_style.padding_right > 0 ? td_style.padding_right / pdf->k : default_pad;

            // Text width: max line width of the cell (text + padding)
            std::string text = trim_cell_text(node_to_text(cells[i]));
            double max_line_w = 0;

            // Replace \r\n, \r with \n, then split by \n
            std::string clean_text;
            clean_text.reserve(text.size());
            for (size_t p = 0; p < text.size(); p++) {
                char c = text[p];
                if (c == '\r') {
                    if (p + 1 < text.size() && text[p + 1] == '\n')
                        p++;
                    clean_text += '\n';
                } else {
                    clean_text += c;
                }
            }

            std::istringstream lss(clean_text);
            std::string line;
            while (std::getline(lss, line)) {
                // Trim leading/trailing whitespace for width calculation
                size_t ls = line.find_first_not_of(" \t");
                size_t le = line.find_last_not_of(" \t");
                if (ls == std::string::npos) continue;
                std::string trimmed = line.substr(ls, le - ls + 1);
                double lw = pdf->GetStringWidth(trimmed);
                if (lw > max_line_w) max_line_w = lw;
            }

            double tw = max_line_w + pad_l + pad_r;
            if (tw < min_col_w) tw = min_col_w;
            if (tw > col_text_widths[i]) col_text_widths[i] = tw;

            // Explicit style width (TH takes priority over TD)
            if (td_style.width > 0) {
                double w = td_style.width / pdf->k;
                if (cells[i]->tag == "th") {
                    if (w > col_th_widths[i]) {
                        col_th_widths[i] = w;
                        col_has_th_width[i] = true;
                    }
                } else {
                    if (w > col_td_widths[i]) {
                        col_td_widths[i] = w;
                        col_has_td_width[i] = true;
                    }
                }
            }
            // Store percentage width
            if (td_style.width_pct > 0 && td_style.width_pct > col_pct_widths[i]) {
                col_pct_widths[i] = td_style.width_pct;
            }
        }
    }

    // Restore font state after measuring
    pdf->FontFamily = saved_family;
    pdf->FontStyle = saved_style;
    pdf->FontSize = saved_size;
    pdf->FontSizePt = saved_size;
    pdf->TextColor = saved_text_color;

    // Calculate natural width for each column
    std::vector<double> natural_w(num_cols, 0);
    for (int i = 0; i < num_cols; i++) {
        if (col_has_th_width[i]) {
            natural_w[i] = col_th_widths[i];
        } else if (col_has_td_width[i]) {
            natural_w[i] = col_td_widths[i];
        } else {
            natural_w[i] = col_text_widths[i];
        }
        if (natural_w[i] < min_col_w) natural_w[i] = min_col_w;
    }

    // Check for explicit table width
    double table_w = 0;
    double max_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;

    if (style.width_pct > 0) {
        table_w = max_w * style.width_pct / 100.0;
    } else if (style.width > 0) {
        table_w = style.width / pdf->k;
    } else {
        // No explicit table width: calculate total width with tolerance
        double pre_t_w = 0;
        for (int i = 0; i < num_cols; i++) {
            pre_t_w += natural_w[i];
        }
        if (pre_t_w > max_w) {
            table_w = max_w;
        } else {
            table_w = pre_t_w + min_col_w * num_cols;
            if (table_w > max_w) table_w = max_w;
        }
    }

    // Calculate column widths with table width known
    std::vector<double> col_widths(num_cols, 0);
    double remaining = table_w;
    int auto_cols = 0;

    // First pass: columns with TH/TD explicit width get their width
    for (int i = 0; i < num_cols; i++) {
        if (col_has_th_width[i]) {
            col_widths[i] = col_th_widths[i];
            remaining -= col_widths[i];
        } else if (col_has_td_width[i]) {
            col_widths[i] = col_td_widths[i];
            remaining -= col_widths[i];
        } else if (col_pct_widths[i] > 0) {
            col_widths[i] = table_w * col_pct_widths[i] / 100.0;
            remaining -= col_widths[i];
        } else {
            auto_cols++;
        }
    }

    // Second pass: distribute remaining equally among auto columns
    if (auto_cols > 0) {
        double share = remaining / auto_cols;
        if (share < min_col_w) share = min_col_w;
        for (int i = 0; i < num_cols; i++) {
            if (!col_has_th_width[i] && !col_has_td_width[i] && col_pct_widths[i] <= 0) {
                col_widths[i] = share;
            }
        }
    }

    // If total exceeds table width, scale down proportionally
    double total = 0;
    for (double w : col_widths) total += w;
    if (total > table_w && total > 0) {
        double scale = table_w / total;
        for (double& w : col_widths) w *= scale;
    }

    // Ensure minimum width
    for (double& w : col_widths) {
        if (w < min_col_w) w = min_col_w;
    }


    auto border_it = table_node->attrs.find("border");
    bool has_border = (border_it != table_node->attrs.end() && border_it->second != "0");

    double lh = get_line_height_mm(style);
    bool is_fixed = (style.position == "fixed");

    // Calculate table X position based on align attribute
    double actual_table_w = 0;
    for (double w : col_widths) actual_table_w += w;

    std::string table_align = "left"; // default
    auto align_it = table_node->attrs.find("align");
    if (align_it != table_node->attrs.end()) {
        table_align = str_tolower(align_it->second);
    }

    double table_x = pdf->lMargin; // default: left
    if (table_align == "center") {
        table_x = pdf->lMargin + (max_w - actual_table_w) / 2.0;
        if (table_x < pdf->lMargin) table_x = pdf->lMargin;
    } else if (table_align == "right") {
        table_x = pdf->GetPageWidth() - pdf->rMargin - actual_table_w;
        if (table_x < pdf->lMargin) table_x = pdf->lMargin;
    }

    pdf->SetX(table_x);

    std::string saved_draw_color = pdf->DrawColor;
    if (has_border && !style.border_color.empty()) {
        std::istringstream iss(style.border_color);
        int r = 0, g = 0, b = 0;
        iss >> r >> g >> b;
        pdf->SetDrawColor(r, g, b);
    }

    // Find thead rows
    std::vector<std::shared_ptr<HTMLNode>> thead_rows;
    for (auto& sec : sections) {
        if (sec.tag == "thead") {
            thead_rows = sec.rows;
            break;
        }
    }

    // Lambda to calculate row height without rendering
    auto calc_row_height = [&](const std::shared_ptr<HTMLNode>& row, 
                                const std::map<int, std::string>& text_overrides = {}) -> double {
        std::vector<std::shared_ptr<HTMLNode>> cells;
        for (auto& c : row->children) {
            if (c->type == HTMLNodeType::ELEMENT && (c->tag == "td" || c->tag == "th")) {
                cells.push_back(c);
            }
        }

        double max_h = lh;
        for (int i = 0; i < num_cols; i++) {
            if (i >= (int)cells.size()) break;
            auto& cell = cells[i];
            std::string text;
            auto override_it = text_overrides.find(i);
            if (override_it != text_overrides.end()) {
                text = override_it->second;
            } else {
                text = trim_cell_text(node_to_text(cell));
            }
            
            CSSStyle cell_style = style;
            auto sit = stylesheet.find(cell->tag);
            if (sit != stylesheet.end())
                css_parser.merge_style(cell_style, sit->second);
            css_parser.merge_style(cell_style, cell->style);
            if (cell->tag == "th") cell_style.bold = true;
            apply_style(cell_style);
            
            double pad_l = cell_style.padding_left > 0 ? cell_style.padding_left / pdf->k : default_pad;
            double pad_r = cell_style.padding_right > 0 ? cell_style.padding_right / pdf->k : default_pad;
            double pad_t = cell_style.padding_top > 0 ? cell_style.padding_top / pdf->k : default_pad;
            double pad_b = cell_style.padding_bottom > 0 ? cell_style.padding_bottom / pdf->k : default_pad;
            
            double avail_w = col_widths[i] - pad_l - pad_r;
            if (avail_w < min_col_w - default_pad * 2) avail_w = min_col_w - default_pad * 2;

            int num_lines = pdf->GetMultiCellLines(avail_w, text);
            if (num_lines < 1) num_lines = 1;
            double cell_h = num_lines * lh + pad_t + pad_b;
            if (cell_h > max_h) max_h = cell_h;
        }
        return max_h;
    };

    // Lambda to render a single row
    // text_overrides: optional map of cell_index -> text to use instead of node content
    // is_cross_page: if true, disable vertical centering (all cells top-aligned)
    auto render_row = [&](const std::shared_ptr<HTMLNode>& row, 
                          const std::map<int, std::string>& text_overrides = {},
                          bool is_cross_page = false) {
        std::vector<std::shared_ptr<HTMLNode>> cells;
        for (auto& c : row->children) {
            if (c->type == HTMLNodeType::ELEMENT && (c->tag == "td" || c->tag == "th")) {
                cells.push_back(c);
            }
        }

        bool old_auto_page_break = pdf->AutoPageBreak;
        double old_b_margin = pdf->bMargin;
        pdf->SetAutoPageBreak(false);

        double x_start = pdf->GetX();
        double y_start = pdf->GetY();

        // First pass: calculate row height based on wrapped text
        double row_h = lh;
        for (int i = 0; i < num_cols; i++) {
            if (i >= (int)cells.size()) break;
            auto& cell = cells[i];
            std::string text;
            auto override_it = text_overrides.find(i);
            if (override_it != text_overrides.end()) {
                text = override_it->second;
            } else {
                text = trim_cell_text(node_to_text(cell));
            }

            CSSStyle cell_style = style;
            auto sit = stylesheet.find(cell->tag);
            if (sit != stylesheet.end())
                css_parser.merge_style(cell_style, sit->second);
            css_parser.merge_style(cell_style, cell->style);
            if (cell->tag == "th") cell_style.bold = true;
            apply_style(cell_style);

            double pad_l = cell_style.padding_left > 0 ? cell_style.padding_left / pdf->k : default_pad;
            double pad_r = cell_style.padding_right > 0 ? cell_style.padding_right / pdf->k : default_pad;
            double pad_t = cell_style.padding_top > 0 ? cell_style.padding_top / pdf->k : default_pad;
            double pad_b = cell_style.padding_bottom > 0 ? cell_style.padding_bottom / pdf->k : default_pad;

            double cell_w = col_widths[i] - pad_l - pad_r;
            if (cell_w < min_col_w - default_pad * 2) cell_w = min_col_w - default_pad * 2;


            int num_lines = pdf->GetMultiCellLines(cell_w, text);
            if (num_lines < 1) num_lines = 1;
            double cell_h = num_lines * lh + pad_t + pad_b;
            if (cell_h > row_h) row_h = cell_h;
        }

        // Second pass: render cells
        for (int i = 0; i < num_cols; i++) {
            if (i >= (int)cells.size()) break;
            auto& cell = cells[i];
            std::string text;
            auto override_it = text_overrides.find(i);
            if (override_it != text_overrides.end()) {
                text = override_it->second;
            } else {
                text = trim_cell_text(node_to_text(cell));
            }

            bool is_header = (cell->tag == "th");
            // Default alignment: th=center, td=left
            std::string align = is_header ? "C" : "L";

            // Check align attribute first
            auto align_it = cell->attrs.find("align");
            if (align_it != cell->attrs.end()) {
                std::string a = str_tolower(align_it->second);
                if (a == "center") align = "C";
                else if (a == "right") align = "R";
                else if (a == "left") align = "L";
            }

            CSSStyle cell_style = style;
            auto sit = stylesheet.find(cell->tag);
            if (sit != stylesheet.end())
                css_parser.merge_style(cell_style, sit->second);
            css_parser.merge_style(cell_style, cell->style);

            if (is_header) {
                cell_style.bold = true;
            }

            // Check CSS text-align (overrides default)
            if (!cell_style.text_align.empty()) {
                if (cell_style.text_align == "center") align = "C";
                else if (cell_style.text_align == "right") align = "R";
                else if (cell_style.text_align == "left") align = "L";
            }

            apply_style(cell_style);

            double pad_l = cell_style.padding_left > 0 ? cell_style.padding_left / pdf->k : default_pad;
            double pad_r = cell_style.padding_right > 0 ? cell_style.padding_right / pdf->k : default_pad;
            double pad_t = cell_style.padding_top > 0 ? cell_style.padding_top / pdf->k : default_pad;
            double pad_b = cell_style.padding_bottom > 0 ? cell_style.padding_bottom / pdf->k : default_pad;

            double x = x_start;
            for (int j = 0; j < i; j++) x += col_widths[j];
            pdf->SetXY(x, y_start);

            // Draw cell background/border
            if (has_border) {
                pdf->Rect(x, y_start, col_widths[i], row_h);
            }

            // Calculate text height for vertical alignment
            double cell_w = col_widths[i] - pad_l - pad_r;
            if (cell_w < min_col_w - default_pad * 2) cell_w = min_col_w - default_pad * 2;
            int num_lines = pdf->GetMultiCellLines(cell_w, text);
            if (num_lines < 1) num_lines = 1;
            double text_h = num_lines * lh;

            // Determine vertical position based on vertical-align
            // For cross-page rows, always use top alignment
            double text_y = y_start + pad_t;
            if (!is_cross_page && cell_style.vertical_align == "bottom") {
                text_y = y_start + row_h - pad_b - text_h;
                if (text_y < y_start + pad_t) text_y = y_start + pad_t;
            } else if (!is_cross_page && cell_style.vertical_align != "top") {
                // Default: vertically centered (only for non-cross-page rows)
                double avail_v = row_h - pad_t - pad_b;
                text_y = y_start + pad_t + (avail_v - text_h) / 2.0;
                if (text_y < y_start + pad_t) text_y = y_start + pad_t;
            }

            // Render text with wrapping
            pdf->SetXY(x + pad_l, text_y);
            pdf->MultiCell(col_widths[i] - pad_l - pad_r, lh, text, 0, align);
        }

        // After rendering all cells, determine correct Y position
        double expected_y = y_start + row_h;
        pdf->SetXY(x_start, expected_y);

        pdf->SetAutoPageBreak(old_auto_page_break, old_b_margin);
    };

    // Lambda to render thead
    auto render_thead = [&]() {
        for (auto& row : thead_rows) {
            render_row(row);
        }
    };

    // Collect tfoot rows
    std::vector<std::shared_ptr<HTMLNode>> tfoot_rows;
    for (auto& sec : sections) {
        if (sec.tag == "tfoot") {
            tfoot_rows = sec.rows;
            break;
        }
    }

    // Render sections: thead first, then tbody, tfoot last
    if (!thead_rows.empty()) {
        double space_avail = pdf->PageBreakTrigger - pdf->GetY();
        if (space_avail < lh * 3 && pdf->GetY() > pdf->tMargin + 1) {
            pdf->AddPage();
            pdf->SetX(table_x);
        }
        render_thead();
    }

    for (auto& sec : sections) {
        if (sec.tag == "thead" || sec.tag == "tfoot") {
            continue; // handled separately
        }

        for (auto& row : sec.rows) {
            // Get cells
            std::vector<std::shared_ptr<HTMLNode>> cells;
            for (auto& c : row->children) {
                if (c->type == HTMLNodeType::ELEMENT && (c->tag == "td" || c->tag == "th")) {
                    cells.push_back(c);
                }
            }
            
            // Get all cell texts
            std::vector<std::string> cell_texts;
            for (auto& cell : cells) {
                cell_texts.push_back(trim_cell_text(node_to_text(cell)));
            }
            
            // Loop until all content is rendered (handles cross-page)
            bool has_been_split = false;  // Track if this row has been split across pages
            while (true) {
                // Calculate available space
                double y_cur = pdf->GetY();
                // Use PageBreakTrigger to match Cell/MultiCell's auto-break check
                double page_avail = pdf->PageBreakTrigger - y_cur;
                
                // Calculate actual row height first
                std::map<int, std::string> check_overrides;
                for (int i = 0; i < (int)cell_texts.size() && i < num_cols; i++) {
                    check_overrides[i] = cell_texts[i];
                }
                double row_height = calc_row_height(row, check_overrides);
                
                double min_row_h = lh + default_pad * 2;
                if (page_avail < min_row_h) {
                    pdf->AddPage();
                    pdf->SetX(table_x);
                    if (is_fixed && !thead_rows.empty()) {
                        render_thead();
                    }
                    continue;
                }
                
                if (row_height <= page_avail) {
                    // Row fits entirely, render normally
                    render_row(row, check_overrides, has_been_split);
                    break;
                }
                
                // Row doesn't fit, need to split
                // Calculate max_lines for split logic
                double default_pad_h = default_pad * 2;
                int max_lines = (int)((page_avail - default_pad_h) / lh);
                if (max_lines < 1) max_lines = 1;
                
                // Split content: render what fits, save rest for next page
                std::map<int, std::string> text_overrides;
                std::map<int, std::string> remaining_texts;
                bool has_remaining = false;
                
                for (int i = 0; i < (int)cell_texts.size() && i < num_cols; i++) {
                    std::string& txt = cell_texts[i];
                    if (txt.empty()) {
                        text_overrides[i] = "";
                        continue;
                    }
                    
                    // Get cell style for width calculation
                    CSSStyle cell_style = style;
                    auto sit = stylesheet.find(cells[i]->tag);
                    if (sit != stylesheet.end())
                        css_parser.merge_style(cell_style, sit->second);
                    css_parser.merge_style(cell_style, cells[i]->style);
                    apply_style(cell_style);
                    
                    double pad_l = cell_style.padding_left > 0 ? cell_style.padding_left / pdf->k : default_pad;
                    double pad_r = cell_style.padding_right > 0 ? cell_style.padding_right / pdf->k : default_pad;
                    double cell_w = col_widths[i] - pad_l - pad_r;
                    if (cell_w < min_col_w - default_pad * 2) cell_w = min_col_w - default_pad * 2;
                    
                    std::string txt_remaining = txt;
                    std::string first_part = pdf->SplitTextByLines(cell_w, txt_remaining, max_lines);
                    
                    if (txt_remaining.empty()) {
                        text_overrides[i] = txt;
                    } else {
                        text_overrides[i] = first_part;
                        remaining_texts[i] = txt_remaining;
                        has_remaining = true;
                        has_been_split = true;
                    }
                }
                
                // Ensure all columns have overrides (fill missing with empty)
                for (int i = 0; i < num_cols; i++) {
                    if (text_overrides.find(i) == text_overrides.end()) {
                        text_overrides[i] = "";
                    }
                }
                
                // Render the row with split content (cross-page, so top-align all cells)
                render_row(row, text_overrides, true);
                
                if (!has_remaining) {
                    break;  // All content rendered
                }
                
                // Prepare for next page: 
                // - Cells with remaining content: update with remaining text
                // - Cells that are done: clear to empty (don't render again)
                for (int i = 0; i < (int)cell_texts.size(); i++) {
                    auto remaining_it = remaining_texts.find(i);
                    if (remaining_it != remaining_texts.end()) {
                        // This cell has remaining content
                        cell_texts[i] = remaining_it->second;
                    } else {
                        // This cell is done, clear it
                        cell_texts[i] = "";
                    }
                }
                
                // Add new page for remaining content
                pdf->AddPage();
                pdf->SetX(table_x);
                if (is_fixed && !thead_rows.empty()) {
                    render_thead();
                }
            }
        }
    }

    // Render tfoot at the end
    for (auto& row : tfoot_rows) {
        // Calculate actual row height first
        double row_height = calc_row_height(row);

        double y_cur = pdf->GetY();
        double page_avail = pdf->PageBreakTrigger - y_cur;

        // If row doesn't fit before page break trigger, move to next page
        if (row_height > page_avail && page_avail < lh * 2) {
            pdf->AddPage();
            pdf->SetX(table_x); // restore table X position
            if (is_fixed && !thead_rows.empty()) {
                render_thead();
            }
        }

        render_row(row);
    }

    if (has_border && !style.border_color.empty()) {
        pdf->DrawColor = saved_draw_color;
        pdf->_out(saved_draw_color);
    }

    apply_style(style);
    pdf->Ln(lh * 0.3);
}

// Render a single cell with cross-page support, background, and border
// (same logic as td rendering in render_table, but for standalone use)
void HTMLRenderer::render_td_cell(
    const std::string& raw_text, double cell_w, double lh,
    double pad_l, double pad_r, double pad_t, double pad_b,
    const CSSStyle& style)
{
    // Normalize \r\n -> \n
    std::string text;
    text.reserve(raw_text.size());
    for (size_t i = 0; i < raw_text.size(); i++) {
        if (raw_text[i] == '\r') {
            if (i + 1 < raw_text.size() && raw_text[i + 1] == '\n') i++;
            text += '\n';
        } else {
            text += raw_text[i];
        }
    }

    double tw = cell_w - pad_l - pad_r;
    if (tw < 10) tw = 10;

    bool has_bg = style.has_bg_color && !style.background_color.empty();
    bool has_border = style.border_width > 0 && style.border_style != "none";
    double bw = has_border ? style.border_width / pdf->k : 0;
    double radius = style.border_radius > 0 ? style.border_radius / pdf->k : 0;

    // Save and set colors
    std::string saved_fill = pdf->FillColor;
    std::string saved_draw = pdf->DrawColor;
    double old_lw = pdf->LineWidth;

    if (has_bg) {
        std::istringstream iss(style.background_color);
        int r = 0, g = 0, b = 0;
        iss >> r >> g >> b;
        pdf->SetFillColor(r, g, b);
    }
    if (has_border && !style.border_color.empty()) {
        std::istringstream iss(style.border_color);
        int r = 0, g = 0, b = 0;
        iss >> r >> g >> b;
        pdf->SetDrawColor(r, g, b);
    }

    std::string remaining = text;
    bool is_first = true;

    while (!remaining.empty()) {
        double y_cur = pdf->GetY();
        double page_avail = pdf->PageBreakTrigger - y_cur;

        // Check minimum space
        double min_h = pad_t + pad_b + lh + 2 * bw;
        if (page_avail < min_h) {
            pdf->AddPage();
            continue;
        }

        // Calculate max lines that fit
        double box_avail = page_avail - pad_t - pad_b - 2 * bw;
        int max_lines = (int)(box_avail / lh);
        if (max_lines < 1) max_lines = 1;

        // Split text
        std::string portion = pdf->SplitTextByLines(tw, remaining, max_lines);
        bool is_last = remaining.empty();

        // Calculate actual height
        int actual_lines = pdf->GetMultiCellLines(tw, portion);
        if (actual_lines < 1) actual_lines = 1;
        double portion_h = actual_lines * lh;
        double box_h = portion_h + pad_t + pad_b + 2 * bw;

        double box_x = pdf->GetX();
        double box_y = pdf->GetY();

        // Draw background
        if (has_bg) {
            if (radius > 0 && is_first && is_last) {
                draw_rounded_rect(box_x, box_y, cell_w, box_h, radius, true, false, 0);
            } else {
                pdf->Rect(box_x, box_y, cell_w, box_h, "F");
            }
        }

        // Draw border
        if (has_border) {
            pdf->SetLineWidth(bw);
            if (style.border_style == "dashed") {
                pdf->SetDash("3 3", 0);
            } else if (style.border_style == "dotted") {
                pdf->SetDash("1 2", 0);
            } else {
                pdf->SetDash("");
            }
            if (radius > 0 && is_first && is_last) {
                draw_rounded_rect(box_x, box_y, cell_w, box_h, radius, false, true, bw);
            } else {
                pdf->Rect(box_x, box_y, cell_w, box_h, "D");
            }
        }

        // Render text with MultiCell
        pdf->SetXY(box_x + bw + pad_l, box_y + bw + pad_t);
        pdf->MultiCell(tw, lh, portion, 0, "L");

        pdf->SetY(box_y + box_h);

        if (!is_last) {
            pdf->AddPage();
            is_first = false;
        }
    }

    // Restore colors
    if (has_bg) { pdf->FillColor = saved_fill; pdf->_out(saved_fill); }
    if (has_border) { pdf->DrawColor = saved_draw; pdf->SetLineWidth(old_lw); pdf->SetDash(""); pdf->_out(saved_draw); }
}

void HTMLRenderer::render_nodes(const std::vector<std::shared_ptr<HTMLNode>>& nodes,
                                CSSStyle current_style, const std::string& href,
                                double parent_width) {
    if (render_depth_ >= MAX_RENDER_DEPTH)
        return; // guard against deeply nested / malicious markup causing stack overflow
    render_depth_++;
    struct DepthGuard { int& d; ~DepthGuard() { d--; } } _dg{render_depth_};
    for (size_t idx = 0; idx < nodes.size(); idx++) {
        auto& node = nodes[idx];
        if (node->type == HTMLNodeType::TEXT) {
            // Skip whitespace-only text nodes that appear right before side-by-side divs
            // (HTML newlines between tags become space characters that add unwanted height)
            bool is_ws = !node->text.empty() && node->text.find_first_not_of(" \t\n\r") == std::string::npos;
            if (is_ws) {
                bool next_is_sbs = false;
                for (size_t j = idx + 1; j < nodes.size(); j++) {
                    if (nodes[j]->type == HTMLNodeType::TEXT) {
                        if (nodes[j]->text.find_first_not_of(" \t\n\r") != std::string::npos)
                            break; // non-whitespace text follows
                        continue; // more whitespace, keep looking
                    }
                    if (nodes[j]->type == HTMLNodeType::ELEMENT && nodes[j]->tag == "div") {
                        double total_pct = 0;
                        int child_count = 0;
                        for (auto& ch : nodes[j]->children) {
                            if (ch->type == HTMLNodeType::ELEMENT && ch->tag == "div") {
                                CSSStyle cs;
                                css_parser.merge_style(cs, ch->style);
                                if (cs.width_pct > 0) { child_count++; total_pct += cs.width_pct; }
                            }
                        }
                        if (child_count >= 2 && total_pct >= 99 && total_pct <= 101)
                            next_is_sbs = true;
                    }
                    break;
                }
                if (next_is_sbs) {
                    continue; // skip whitespace before side-by-side div
                }
            }
            write_flow_text(node->text, current_style, href);
        } else if (node->type == HTMLNodeType::BR) {
            flush_line_buffer(true);
            line_buf_lh_ = 0;
            line_buf_fs_mm_ = 0;
            line_buf_align_.clear();
        } else if (node->type == HTMLNodeType::ELEMENT) {
            CSSStyle new_style = current_style;
            // Clear non-inherited properties before applying element styles
            new_style.background_color.clear();
            new_style.background_image.clear();
            new_style.has_bg_color = false;
            new_style.border_style.clear();
            new_style.border_width = 0;
            new_style.border_color.clear();
            new_style.border_radius = 0;
            new_style.border_left_w = 0;
            new_style.border_left_color.clear();
            new_style.border_right_w = 0;
            new_style.border_right_color.clear();
            new_style.border_top_w = 0;
            new_style.border_top_color.clear();
            new_style.border_bottom_w = 0;
            new_style.border_bottom_color.clear();
            new_style.margin_top = 0;
            new_style.margin_bottom = 0;
            new_style.margin_left = 0;
            new_style.margin_right = 0;
            new_style.padding_top = 0;
            new_style.padding_bottom = 0;
            new_style.padding_left = 0;
            new_style.padding_right = 0;
            new_style.width = 0;
            new_style.width_pct = 0;
            new_style.height = 0;
            new_style.position.clear();
            new_style.page_break_before = false;
            new_style.justify_content.clear();
            // 1. Tag-level stylesheet (e.g., "div", "p")
            auto sit = stylesheet.find(node->tag);
            if (sit != stylesheet.end())
                css_parser.merge_style(new_style, sit->second);
            // 2. Class-based stylesheet (e.g., ".note", ".tip")
            auto class_it = node->attrs.find("class");
            if (class_it != node->attrs.end()) {
                std::istringstream cls_iss(class_it->second);
                std::string cls;
                while (cls_iss >> cls) {
                    std::string sel = "." + str_tolower(cls);
                    auto csit = stylesheet.find(sel);
                    if (csit != stylesheet.end())
                        css_parser.merge_style(new_style, csit->second);
                }
            }
            // 3. Inline style (highest priority)
            css_parser.merge_style(new_style, node->style);

            std::string tag = node->tag;
            bool is_block = false;
            double before_space = 0;
            double after_space = 0;

            double cur_lh = get_line_height_mm(new_style);

            // Skip head/title/meta/link tags
            if (tag == "head" || tag == "title" || tag == "meta" || tag == "link") {
                continue;
            }

            // Skip special page elements - handled separately in WriteHTML
            if (tag == "div") {
                if (has_class(node, "pagecover") ||
                    has_class(node, "pagecatalogue") ||
                    has_class(node, "cataloguecontent") ||
                    has_class(node, "pageheader") ||
                    has_class(node, "pagefooter")) {
                    continue;
                }
            }

            // Check for catalogue1-7 class on any element - set internal link target
            {
                auto class_it2 = node->attrs.find("class");
                if (class_it2 != node->attrs.end()) {
                    int toc_level = get_toc_level(class_it2->second);
                    if (toc_level > 0 && toc_match_index < (int)toc_entries.size()) {
                        auto& entry = toc_entries[toc_match_index];
                        if (entry.level == toc_level) {
                            if (entry.link_id == 0) {
                                entry.link_id = pdf->AddLink();
                            }
                            pdf->SetLink(entry.link_id, 0, pdf->PageNo());
                            entry.page_no = pdf->PageNo();
                            toc_match_index++;
                        }
                    }
                }
            }

            // Handle text-align for p and div tags
            // (text-align is handled by write_flow_text based on style.text_align)
            
            if (tag == "p") {
                is_block = true;
                before_space = cur_lh * 0.25;
                after_space = cur_lh * 0.25;
            } else if (tag == "div" || tag == "section" || tag == "article") {
                is_block = true;
                before_space = cur_lh * 0.3;
                after_space = cur_lh * 0.3;
                
                // Check for side-by-side child divs layout
                if (tag == "div") {
                    double pw = parent_width > 0 ? parent_width : (pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin);
                    
                    // Calculate this div's effective width
                    double div_width = 0;
                    if (new_style.width > 0) {
                        div_width = new_style.width / pdf->k;
                    } else if (new_style.width_pct > 0) {
                        div_width = pw * new_style.width_pct / 100.0;
                    }
                    if (div_width <= 0) div_width = pw;
                    
                    // Detect side-by-side child divs
                    std::vector<std::shared_ptr<HTMLNode>> child_divs;
                    double total_child_pct = 0;
                    for (auto& child : node->children) {
                        if (child->type == HTMLNodeType::ELEMENT && child->tag == "div") {
                            CSSStyle child_style;
                            css_parser.merge_style(child_style, child->style);
                            if (child_style.width_pct > 0) {
                                child_divs.push_back(child);
                                total_child_pct += child_style.width_pct;
                            }
                        }
                    }
                    
                    if (child_divs.size() >= 2 && total_child_pct >= 99 && total_child_pct <= 101) {
                        // Side-by-side layout detected
                        double saved_rm = pdf->rMargin;
                        double saved_lm = pdf->lMargin;
                        double y_before = pdf->GetY();
                        double x_base = pdf->lMargin;
                        
                        // Center the parent div if margin: 0 auto
                        if (new_style.margin_left_auto && new_style.margin_right_auto && div_width < pw) {
                            double center_offset = (pw - div_width) / 2.0;
                            if (center_offset > 0) x_base += center_offset;
                        }
                        
                        // Apply justify-content center offset for children
                        if (new_style.justify_content == "center") {
                            double total_children_w = 0;
                            for (auto& cd : child_divs) {
                                CSSStyle cs;
                                css_parser.merge_style(cs, cd->style);
                                total_children_w += div_width * cs.width_pct / 100.0;
                            }
                            double offset = (div_width - total_children_w) / 2.0;
                            if (offset > 0) x_base += offset;
                        }
                        
                        // Estimate content height to check if page break is needed
                        double max_child_h = 0;
                        std::string saved_family = pdf->FontFamily;
                        std::string saved_style_str = pdf->FontStyle;
                        double saved_size = pdf->FontSizePt;
                        for (auto& cd : child_divs) {
                            CSSStyle cs;
                            css_parser.merge_style(cs, cd->style);
                            double child_w = div_width * cs.width_pct / 100.0;
                            
                            CSSStyle est_style = new_style;
                            css_parser.merge_style(est_style, cd->style);
                            
                            std::string fam = est_style.font_family.empty() ? saved_family : est_style.font_family;
                            std::string st = "";
                            if (est_style.bold) st += "B";
                            if (est_style.italic) st += "I";
                            double sz = est_style.font_size > 0 ? est_style.font_size : saved_size;
                            try { pdf->SetFont(fam, st, sz); } catch (...) {}
                            
                            std::string text = node_to_text(cd);
                            if (!text.empty()) {
                                int lines = pdf->GetMultiCellLines(child_w, text);
                                double lh = get_line_height_mm(est_style);
                                double h = lines * lh;
                                if (h > max_child_h) max_child_h = h;
                            }
                        }
                        try { pdf->SetFont(saved_family, saved_style_str, saved_size); } catch (...) {}
                        
                        double block_lh = get_line_height_mm(new_style);
                        double total_h = max_child_h + block_lh;
                        
                        // If not enough space on current page, start a new page first
                        if (pdf->AutoPageBreak && !pdf->InHeader && !pdf->InFooter) {
                            if (y_before + total_h > pdf->PageBreakTrigger) {
                                pdf->AddPage();
                                y_before = pdf->GetY();
                            }
                        }
                        
                        // Constrain parent width
                        double new_rm = pdf->GetPageWidth() - x_base - div_width;
                        if (new_rm < 0) new_rm = 0;
                        pdf->SetRightMargin(new_rm);
                        pdf->SetLeftMargin(x_base);
                        
                        double cur_x = x_base;
                        double max_y = y_before;
                        
                        for (auto& cd : child_divs) {
                            CSSStyle cs;
                            css_parser.merge_style(cs, cd->style);
                            double child_w = div_width * cs.width_pct / 100.0;
                            
                            pdf->SetXY(cur_x, y_before);
                            double child_rm = pdf->GetPageWidth() - cur_x - child_w;
                            if (child_rm < 0) child_rm = 0;
                            pdf->SetLeftMargin(cur_x);
                            pdf->SetRightMargin(child_rm);
                            
                            line_buf_.clear();
                            line_buf_width_ = 0;
                            line_buf_lh_ = 0;
                            line_buf_fs_mm_ = 0;
                            line_buf_align_.clear();
                            
                            CSSStyle child_new_style = new_style;
                            css_parser.merge_style(child_new_style, cd->style);
                            apply_style(child_new_style);
                            render_nodes(cd->children, child_new_style, href, child_w);
                            flush_line_buffer(false);
                            
                            double child_y = pdf->GetY();
                            if (child_y > max_y) max_y = child_y;
                            
                            cur_x += child_w;
                        }
                        
                        // Restore margins
                        pdf->SetLeftMargin(saved_lm);
                        pdf->SetRightMargin(saved_rm);
                        pdf->SetX(saved_lm);
                        pdf->SetY(max_y);
                        
                        // write_flow_text doesn't call Ln(lh) after the last line,
                        // so we must consume one line height here to advance Y past the content
                        pdf->Ln(block_lh);
                        
                        // Skip normal block rendering
                        before_space = 0;
                        after_space = 0;
                        continue;
                    }
                }
            } else if (tag == "h1" || tag == "h2" || tag == "h3" ||
                       tag == "h4" || tag == "h5" || tag == "h6") {
                is_block = true;
                new_style.bold = true;
                // Size headings relative to the inherited (parent) font size, not
                // pdf->FontSizePt. The latter is live renderer state and still
                // holds the previous heading's size when two headings are
                // adjacent (e.g. h3 directly followed by h4), which would make
                // the ASCII sub-run be measured at the base size but drawn at
                // the leaked size, producing overlapping glyphs.
                double base_fs = current_style.font_size > 0 ? current_style.font_size : pdf->FontSizePt;
                if (tag == "h1") { new_style.font_size = base_fs * 2; before_space = cur_lh * 1.3; after_space = cur_lh * 1.3; }
                else if (tag == "h2") { new_style.font_size = base_fs * 1.5; before_space = cur_lh * 1.2; after_space = cur_lh * 1.2; }
                else if (tag == "h3") { new_style.font_size = base_fs * 1.17; before_space = cur_lh; after_space = cur_lh; }
                else if (tag == "h4") { new_style.font_size = base_fs; before_space = cur_lh * 0.8; after_space = cur_lh * 0.8; }
                else if (tag == "h5") { new_style.font_size = base_fs * 0.83; before_space = cur_lh * 0.7; after_space = cur_lh * 0.7; }
                else { new_style.font_size = base_fs * 0.67; before_space = cur_lh * 0.6; after_space = cur_lh * 0.6; }
            } else if (tag == "ul" || tag == "ol") {
                is_block = true;
                before_space = cur_lh * 0.3;
                after_space = cur_lh * 0.3;
            } else if (tag == "li") {
                is_block = true;
            } else if (tag == "blockquote") {
                is_block = true;
                new_style.margin_left = cur_lh * 4 * pdf->k;
                before_space = cur_lh * 0.5;
                after_space = cur_lh * 0.5;
            } else if (tag == "fieldset") {
                is_block = true;
                // Default border if not set by CSS
                if (new_style.border_width == 0) {
                    new_style.border_width = 1.5;
                    new_style.border_style = "solid";
                    new_style.border_color = "180 180 180";
                }
                if (new_style.padding_top == 0) new_style.padding_top = 10 * pdf->k;
                if (new_style.padding_bottom == 0) new_style.padding_bottom = 10 * pdf->k;
                if (new_style.padding_left == 0) new_style.padding_left = 10 * pdf->k;
                if (new_style.padding_right == 0) new_style.padding_right = 10 * pdf->k;
                before_space = cur_lh * 0.5;
                after_space = cur_lh * 0.5;
            } else if (tag == "legend") {
                // Legend is not rendered inline; its text is used as fieldset title
                is_block = true;
                // Skip rendering (content extracted by fieldset)
                continue;
            } else if (tag == "pre") {
                is_block = true;
                // Use the current program font (whatever was set via SetFont)
                std::string fam = str_tolower(new_style.font_family);
                if (fam == "courier" || fam == "helvetica" || fam == "times" || fam == "monospace" || fam.empty()) {
                    new_style.font_family = pdf->FontFamily;
                }
                // Default font size: 12px (=9pt)
                if (new_style.font_size == 0) new_style.font_size = 9;
                // Default line height: ~17px (=12.75pt)
                if (new_style.line_height == 0) new_style.line_height = 12.75;
                // Default background color
                if (new_style.background_color.empty()) {
                    new_style.background_color = "245 245 245";
                }
                new_style.has_bg_color = true;
                // Default padding
                if (new_style.padding_top == 0) new_style.padding_top = 8 * pdf->k;
                if (new_style.padding_bottom == 0) new_style.padding_bottom = 8 * pdf->k;
                if (new_style.padding_left == 0) new_style.padding_left = 10 * pdf->k;
                if (new_style.padding_right == 0) new_style.padding_right = 10 * pdf->k;
                // Default border
                if (new_style.border_width == 0) {
                    new_style.border_width = 0.75;  // ~1px
                    new_style.border_style = "solid";
                    new_style.border_color = "221 221 221";  // #ddd
                }
                if (new_style.border_radius == 0) {
                    new_style.border_radius = 4.5;  // ~6px
                }
                before_space = cur_lh * 0.5;
                after_space = cur_lh * 0.5;
            } else if (tag == "code") {
                // Block code if multi-line, inline if single-line
                std::string code_text = node_to_text(node);
                if (code_text.find('\n') != std::string::npos) {
                    is_block = true;
                    // Use program font
                    std::string fam = str_tolower(new_style.font_family);
                    if (fam == "courier" || fam == "helvetica" || fam == "times" || fam == "monospace" || fam.empty()) {
                        new_style.font_family = pdf->FontFamily;
                    }
                    if (new_style.font_size == 0) new_style.font_size = 9;
                    if (new_style.line_height == 0) new_style.line_height = 12.75;
                    if (new_style.background_color.empty()) {
                        new_style.background_color = "245 245 245";
                    }
                    new_style.has_bg_color = true;
                    if (new_style.padding_top == 0) new_style.padding_top = 8 * pdf->k;
                    if (new_style.padding_bottom == 0) new_style.padding_bottom = 8 * pdf->k;
                    if (new_style.padding_left == 0) new_style.padding_left = 10 * pdf->k;
                    if (new_style.padding_right == 0) new_style.padding_right = 10 * pdf->k;
                    if (new_style.border_width == 0) {
                        new_style.border_width = 0.75;
                        new_style.border_style = "solid";
                        new_style.border_color = "221 221 221";
                    }
                    if (new_style.border_radius == 0) {
                        new_style.border_radius = 4.5;
                    }
                    before_space = cur_lh * 0.5;
                    after_space = cur_lh * 0.5;
                } else {
                    // Inline code
                    if (new_style.background_color.empty())
                        new_style.background_color = "#f0f0f0";
                    new_style.font_family = "courier";
                    if (new_style.font_size == 0)
                        new_style.font_size = current_style.font_size * 0.9;
                }
            } else if (tag == "table") {
                is_block = true;
                before_space = cur_lh * 0.3;
                after_space = cur_lh * 0.3;
            } else if (tag == "hr") {
                flush_line_buffer(true);
                line_buf_lh_ = 0;
                line_buf_fs_mm_ = 0;
                line_buf_align_.clear();

                double lh = get_line_height_mm(new_style);
                double mt = new_style.margin_top > 0 ? new_style.margin_top / pdf->k : lh * 0.5;
                double mb = new_style.margin_bottom > 0 ? new_style.margin_bottom / pdf->k : lh * 0.5;
                double bw = new_style.border_width > 0 ? new_style.border_width / pdf->k : 0.2;
                double w_pct = new_style.width_pct;
                double w_px = new_style.width;
                double total_h = mt + bw + mb;

                if (pdf->GetY() + total_h > pdf->GetPageHeight() - pdf->bMargin) {
                    pdf->AddPage();
                } else {
                    pdf->Ln(mt);
                }

                double page_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
                double hr_w = page_w;
                if (w_px > 0) {
                    hr_w = w_px * 0.75 / pdf->k;
                } else if (w_pct > 0) {
                    hr_w = page_w * w_pct / 100.0;
                }
                if (hr_w > page_w) hr_w = page_w;

                double hr_x = pdf->lMargin;
                bool has_width = (w_px > 0 || w_pct > 0);
                if (new_style.text_align == "center" || (has_width && new_style.margin_left_auto && new_style.margin_right_auto)) {
                    hr_x = pdf->lMargin + (page_w - hr_w) / 2.0;
                } else if (new_style.text_align == "right") {
                    hr_x = pdf->GetPageWidth() - pdf->rMargin - hr_w;
                }

                std::string old_dc = pdf->DrawColor;
                double old_lw = pdf->LineWidth;

                if (!new_style.border_color.empty()) {
                    std::istringstream iss2(new_style.border_color);
                    int br = 0, bg = 0, bb = 0;
                    iss2 >> br >> bg >> bb;
                    pdf->SetDrawColor(br, bg, bb);
                }
                pdf->SetLineWidth(bw);

                if (new_style.border_style == "dashed") {
                    pdf->SetDash("3 3", 0);
                } else if (new_style.border_style == "dotted") {
                    pdf->SetDash("1 2", 0);
                } else {
                    pdf->SetDash("");
                }

                pdf->Line(hr_x, pdf->GetY(), hr_x + hr_w, pdf->GetY());

                pdf->DrawColor = old_dc;
                pdf->_out(old_dc);
                pdf->SetLineWidth(old_lw);
                pdf->SetDash("");

                pdf->Ln(bw + mb);
                continue;
            } else if (tag == "img") {
                auto src_it = node->attrs.find("src");
                if (src_it != node->attrs.end()) {
                    std::string src = src_it->second;
                    if (!is_safe_file_path(src)) {
                        std::cerr << "Warning: unsafe image path rejected: " << src << std::endl;
                        continue;
                    }

                    if (has_class(node, "pagefull")) {
                        render_pagefull_image(src);
                        continue;
                    }

                    double pad_left = new_style.padding_left / pdf->k;
                    double pad_right = new_style.padding_right / pdf->k;
                    double pad_top = new_style.padding_top / pdf->k;
                    double pad_bottom = new_style.padding_bottom / pdf->k;

                    bool is_svg = (src.size() > 4 && str_tolower(src.substr(src.size() - 4)) == ".svg");
                    double img_w = 0;
                    double img_h = 0;

                    if (is_svg) {
                        double intrinsic_w = 0, intrinsic_h = 0;
                        std::ifstream svg_file(pdf->_resolve_image_path(src));
                        if (svg_file.is_open()) {
                            std::stringstream ss;
                            ss << svg_file.rdbuf();
                            std::string svg_content = ss.str();
                            auto svg_nodes = parse_html(svg_content);
                            for (auto& n : svg_nodes) {
                                if (n->type == HTMLNodeType::ELEMENT && n->tag == "svg") {
                                    double svg_h = 0;
                                    double svg_w = 0;
                                    auto sh_it = n->attrs.find("height");
                                    if (sh_it != n->attrs.end()) { try { svg_h = std::stod(sh_it->second); } catch(...) {} }
                                    auto sw_it = n->attrs.find("width");
                                    if (sw_it != n->attrs.end()) { try { svg_w = std::stod(sw_it->second); } catch(...) {} }
                                    double px2mm = 0.75 / pdf->k;
                                    intrinsic_h = svg_h * px2mm;
                                    intrinsic_w = svg_w * px2mm;
                                    break;
                                }
                            }
                        }

                        // A size explicitly set on the <img> (width/height attribute
                        // or CSS style) takes priority; otherwise the SVG's own
                        // intrinsic width/height is used. A single specified
                        // dimension keeps the intrinsic aspect ratio.
                        double spec_w = 0, spec_h = 0;
                        auto aw_it = node->attrs.find("width");
                        if (aw_it != node->attrs.end()) { try { spec_w = std::stod(aw_it->second) / pdf->k; } catch(...) {} }
                        auto ah_it = node->attrs.find("height");
                        if (ah_it != node->attrs.end()) { try { spec_h = std::stod(ah_it->second) / pdf->k; } catch(...) {} }
                        if (new_style.width > 0) spec_w = new_style.width / pdf->k;
                        if (new_style.height > 0) spec_h = new_style.height / pdf->k;

                        if (spec_w > 0 && spec_h > 0) {
                            img_w = spec_w; img_h = spec_h;
                        } else if (spec_w > 0) {
                            img_w = spec_w;
                            img_h = (intrinsic_w > 0) ? spec_w * intrinsic_h / intrinsic_w : spec_w;
                        } else if (spec_h > 0) {
                            img_h = spec_h;
                            img_w = (intrinsic_h > 0) ? spec_h * intrinsic_w / intrinsic_h : spec_h;
                        } else {
                            img_w = intrinsic_w;
                            img_h = intrinsic_h;
                        }
                    } else {
                        auto w_it = node->attrs.find("width");
                        if (w_it != node->attrs.end())
                            try { img_w = std::stod(w_it->second); } catch(...) {}
                        auto h_it = node->attrs.find("height");
                        if (h_it != node->attrs.end())
                            try { img_h = std::stod(h_it->second); } catch(...) {}
                        if (img_w > 0) img_w /= pdf->k;
                        if (img_h > 0) img_h /= pdf->k;
                        
                        double real_w = 0, real_h = 0;
                        if (pdf->GetImageSize(src, real_w, real_h)) {
                            if (img_w == 0 && img_h == 0) {
                                img_w = real_w;
                                img_h = real_h;
                            } else if (img_w == 0) {
                                img_w = img_h * real_w / real_h;
                            } else if (img_h == 0) {
                                img_h = img_w * real_h / real_w;
                            }
                        }
                    }

                    double total_w = img_w + pad_left + pad_right;
                    double total_h = img_h + pad_top + pad_bottom;
                    double content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
                    
                    if (total_w > content_w) {
                        double ratio = content_w / total_w;
                        img_w *= ratio;
                        img_h *= ratio;
                        total_w = img_w + pad_left + pad_right;
                        total_h = img_h + pad_top + pad_bottom;
                    }

                    double img_cur_lh = get_line_height_mm(current_style);
                    bool is_inline = (total_h <= img_cur_lh * 3) && (total_w <= content_w * 0.5);

                    if (is_inline) {
                        double avail_w = content_w;
                        if (current_style.text_align != "center" && current_style.text_align != "right") {
                            avail_w = content_w - line_buf_width_;
                        }
                        if (total_w > avail_w + 0.001 && !line_buf_.empty()) {
                            flush_line_buffer(true);
                            line_buf_lh_ = img_cur_lh;
                            line_buf_fs_mm_ = (current_style.font_size > 0 ? current_style.font_size : pdf->FontSizePt) * 0.352778;
                            line_buf_align_ = current_style.text_align;
                        }
                        LineBufferSeg seg;
                        seg.type = LineBufferSegType::IMAGE;
                        seg.img_src = src;
                        seg.width = total_w;
                        seg.height = total_h;
                        seg.img_pad_left = pad_left;
                        seg.img_pad_right = pad_right;
                        seg.img_pad_top = pad_top;
                        seg.img_pad_bottom = pad_bottom;
                        seg.style = current_style;
                        line_buf_.push_back(seg);
                        line_buf_width_ += total_w;
                        if (line_buf_lh_ < total_h) line_buf_lh_ = total_h;
                        if (line_buf_align_.empty()) line_buf_align_ = current_style.text_align;
                    } else {
                        flush_line_buffer(true);
                        line_buf_lh_ = 0;
                        line_buf_fs_mm_ = 0;
                        line_buf_align_.clear();

                        if (is_svg) {
                            std::ifstream svg_file(src);
                            if (svg_file.is_open()) {
                                std::stringstream ss;
                                ss << svg_file.rdbuf();
                                std::string svg_content = ss.str();
                                auto svg_nodes = parse_html(svg_content);
                                for (auto& n : svg_nodes) {
                                    if (n->type == HTMLNodeType::ELEMENT && n->tag == "svg") {
                                        double svg_x = pdf->GetX();
                                        double svg_y = pdf->GetY();
                                        double avail_h = pdf->GetPageHeight() - svg_y - pdf->bMargin;
                                        if (img_h > avail_h) {
                                            pdf->AddPage();
                                            svg_x = pdf->GetX();
                                            svg_y = pdf->GetY();
                                        }
                                        SVGRenderer svg_renderer(pdf);
                                        svg_renderer.render(n, svg_x + pad_left, svg_y + pad_top, img_w, img_h);
                                        pdf->SetY(svg_y + img_h + pad_top + pad_bottom);
                                        pdf->SetX(pdf->lMargin);
                                        break;
                                    }
                                }
                            }
                        } else {
                            double x = pdf->lMargin;
                            double remaining_h = img_h;
                            double offset_y = 0;
                            
                            while (remaining_h > 0) {
                                double avail_h = pdf->GetPageHeight() - pdf->GetY() - pdf->bMargin;
                                if (avail_h <= 1) {
                                    pdf->AddPage();
                                    avail_h = pdf->GetPageHeight() - pdf->GetY() - pdf->bMargin;
                                }
                                
                                double draw_h = std::min(remaining_h, avail_h);
                                double y = pdf->GetY();
                                
                                double clip_x = x * pdf->k;
                                double clip_y = (pdf->h - (y + draw_h)) * pdf->k;
                                double clip_w = img_w * pdf->k;
                                double clip_h = draw_h * pdf->k;
                                
                                pdf->_out("q");
                                pdf->_out(fmt(clip_x) + " " + fmt(clip_y) + " " + fmt(clip_w) + " " + fmt(clip_h) + " re W n");
                                pdf->Image(src, x, y - offset_y, img_w, img_h);
                                pdf->_out("Q");
                                
                                pdf->SetY(y + draw_h);
                                offset_y += draw_h;
                                remaining_h -= draw_h;
                            }
                            
                            pdf->SetX(pdf->lMargin);
                        }
                    }
                }
                continue;
            } else if (tag == "svg") {
                double svg_x = pdf->GetX();
                double svg_y = pdf->GetY();
                double svg_h = 0;
                auto sh_it = node->attrs.find("height");
                if (sh_it != node->attrs.end()) { try { svg_h = std::stod(sh_it->second); } catch(...) {} }
                double px2mm = 0.75 / pdf->k;
                double svg_h_mm = svg_h * px2mm;

                double avail_h = pdf->GetPageHeight() - svg_y - pdf->bMargin;
                if (svg_h_mm > avail_h) {
                    pdf->AddPage();
                    svg_x = pdf->GetX();
                    svg_y = pdf->GetY();
                }

                SVGRenderer svg_renderer(pdf);
                svg_renderer.render(node, svg_x, svg_y);
                pdf->SetY(svg_y + svg_h_mm);
                pdf->SetX(pdf->lMargin);
                continue;
            }

            if (tag == "b" || tag == "strong") {
                new_style.bold = true;
            } else if (tag == "i" || tag == "em") {
                new_style.italic = true;
            } else if (tag == "u" || tag == "ins") {
                new_style.underline = true;
            } else if (tag == "s" || tag == "del" || tag == "strike") {
            } else if (tag == "a") {
                new_style.underline = true;
                new_style.color = "0 0 255";
                auto it = node->attrs.find("href");
                std::string link_url = (it != node->attrs.end()) ? it->second : href;
                apply_style(new_style);
                render_nodes(node->children, new_style, link_url);
                apply_style(current_style);
                continue;
            } else if (tag == "br") {
                double lh = get_line_height_mm(current_style);
                pdf->Ln(lh);
                continue;
            } else if (tag == "font") {
                auto it = node->attrs.find("color");
                if (it != node->attrs.end()) {
                    std::string c = it->second;
                    if (!c.empty() && c[0] == '#') {
                        std::string hex = c.substr(1);
                        if (hex.size() == 3) { std::string h; for (char ch : hex) { h += ch; h += ch; } hex = h; }
                        if (hex.size() == 6) {
                            try {
                                int r = std::stoi(hex.substr(0, 2), nullptr, 16);
                                int g = std::stoi(hex.substr(2, 2), nullptr, 16);
                                int b = std::stoi(hex.substr(4, 2), nullptr, 16);
                                new_style.color = std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
                            } catch (const std::exception&) {}
                        }
                    }
                }
                auto sz = node->attrs.find("size");
                if (sz != node->attrs.end()) {
                    try {
                        int s = std::stoi(sz->second);
                        double sizes[] = {8, 10, 12, 14, 18, 24, 36};
                        if (s >= 1 && s <= 7)
                            new_style.font_size = sizes[s - 1];
                    } catch (const std::exception&) {}
                }
                auto face = node->attrs.find("face");
                if (face != node->attrs.end())
                    new_style.font_family = face->second;
            }

            if (is_block) {
                if (has_class(node, "pagefull")) {
                    auto img_node = find_first_img(node);
                    if (img_node) {
                        auto src_it = img_node->attrs.find("src");
                        if (src_it != img_node->attrs.end()) {
                            std::string img_src = src_it->second;
                            if (is_safe_file_path(img_src)) {
                                render_pagefull_image(img_src);
                            }
                        }
                    }
                    continue;
                }

                double old_lm = pdf->lMargin;
                double old_rm = pdf->rMargin;
                flush_line_buffer(true);
                line_buf_lh_ = 0;
                line_buf_fs_mm_ = 0;
                line_buf_align_.clear();
                // Handle page-break-before: always
                if (new_style.page_break_before && !toc_pass1) {
                    pdf->AddPage();
                }
                if (new_style.margin_left > 0) {
                    double new_lm = old_lm + new_style.margin_left / pdf->k;
                    pdf->SetLeftMargin(new_lm);
                    pdf->SetX(new_lm);
                }
                if (new_style.margin_right > 0) {
                    double new_rm = old_rm + new_style.margin_right / pdf->k;
                    pdf->SetRightMargin(new_rm);
                }

                double avail_content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
                double effective_w = avail_content_w;
                if (new_style.width > 0) {
                    effective_w = new_style.width / pdf->k;
                } else if (new_style.width_pct > 0) {
                    double pw = parent_width > 0 ? parent_width : avail_content_w;
                    effective_w = pw * new_style.width_pct / 100.0;
                }
                if (effective_w > avail_content_w) effective_w = avail_content_w;

                bool has_auto_margin = (new_style.margin_left_auto && new_style.margin_right_auto);
                double width_offset = 0;
                if (tag != "table" && effective_w < avail_content_w) {
                    if (has_auto_margin) {
                        width_offset = (avail_content_w - effective_w) / 2.0;
                    }
                    double new_rm = pdf->GetPageWidth() - pdf->lMargin - effective_w - width_offset;
                    if (new_rm < 0) new_rm = 0;
                    pdf->SetRightMargin(new_rm);
                    if (width_offset > 0) {
                        double new_lm = pdf->lMargin + width_offset;
                        pdf->SetLeftMargin(new_lm);
                        pdf->SetX(new_lm);
                    }
                }

                bool has_bg = new_style.has_bg_color;
                bool has_bg_img = !new_style.background_image.empty();
                bool has_border = (new_style.border_width > 0 && new_style.border_style != "none") ||
                                  new_style.border_left_w > 0 || new_style.border_right_w > 0 ||
                                  new_style.border_top_w > 0 || new_style.border_bottom_w > 0;
                double pad_top = 0, pad_bottom = 0, pad_left = 0, pad_right = 0;
                double half_leading = 0;
                if (has_bg || has_bg_img || has_border) {
                    double default_pad = get_line_height_mm(new_style) * 0.3;
                    double lh_mm = get_line_height_mm(new_style);
                    double fs_mm = new_style.font_size * 0.352778;
                    half_leading = (lh_mm - fs_mm) / 2.0;
                    if (half_leading < 0) half_leading = 0;
                    bool has_css_padding = (new_style.padding_top > 0 || new_style.padding_bottom > 0 ||
                                            new_style.padding_left > 0 || new_style.padding_right > 0);
                    if (has_css_padding) {
                        pad_top = new_style.padding_top / pdf->k;
                        pad_bottom = new_style.padding_bottom / pdf->k;
                        pad_left = new_style.padding_left / pdf->k;
                        pad_right = new_style.padding_right / pdf->k;
                    } else {
                        pad_top = pad_bottom = pad_left = pad_right = default_pad;
                    }
                }

                bool is_empty_pbb = new_style.page_break_before && is_empty_node(node) && !has_bg && !has_bg_img && !has_border &&
                                    new_style.padding_top == 0 && new_style.padding_bottom == 0 &&
                                    new_style.padding_left == 0 && new_style.padding_right == 0;

                double mt = 0;
                double mb = 0;
                if (!is_empty_pbb) {
                    mt = new_style.margin_top > 0 ? new_style.margin_top / pdf->k : (has_bg || has_border ? half_leading + before_space : before_space);
                    if (!has_bg && !has_border && new_style.padding_top > 0)
                        mt += new_style.padding_top / pdf->k;
                    mb = new_style.margin_bottom > 0 ? new_style.margin_bottom / pdf->k : (has_bg || has_border ? half_leading + after_space : after_space);
                }
                if (mt > 0)
                    pdf->Ln(mt);

                double x_start = pdf->GetX();
                double y_start = pdf->GetY();
                double content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;

                apply_style(new_style);
                if (tag == "table") {
                    render_table(node, new_style);
                } else {
                    bool draw_box = (has_bg || has_bg_img || has_border) && (tag == "div" || tag == "p" || tag == "section" || tag == "article" || tag == "pre" || tag == "code" || tag == "fieldset");
                    double y_start_box = y_start;
                    double bw = 0;
                    double bw_top = 0, bw_bottom = 0, bw_left = 0, bw_right = 0;
                    double radius = 0;
                    
                    if (draw_box) {
                        bw = new_style.border_width > 0 ? new_style.border_width / pdf->k : 0;
                        radius = new_style.border_radius > 0 ? new_style.border_radius / pdf->k : 0;
                        // Individual border side widths (fallback to bw)
                        bw_top = new_style.border_top_w > 0 ? new_style.border_top_w / pdf->k : bw;
                        bw_bottom = new_style.border_bottom_w > 0 ? new_style.border_bottom_w / pdf->k : bw;
                        bw_left = new_style.border_left_w > 0 ? new_style.border_left_w / pdf->k : bw;
                        bw_right = new_style.border_right_w > 0 ? new_style.border_right_w / pdf->k : bw;
                    }
                    
                    // === Pre rendered as td cell with cross-page support ===
                    if ((tag == "pre" || tag == "code") && draw_box) {
                        std::string pre_text = node_to_text(node);
                        double pre_lh = get_line_height_mm(new_style);
                        render_td_cell(pre_text, content_w, pre_lh,
                                       pad_left, pad_right, pad_top, pad_bottom,
                                       new_style);
                    } else {
                    // === Normal single-page flow with dry-run ===
                    bool has_padding = (pad_top > 0 || pad_bottom > 0 || pad_left > 0 || pad_right > 0 || (draw_box && (bw_left > 0 || bw_right > 0 || bw_top > 0 || bw_bottom > 0)));
                    double old_lm2 = pdf->lMargin;
                    double old_rm2 = pdf->rMargin;
                    double extra_left = pad_left + (draw_box ? bw_left : 0);
                    double extra_right = pad_right + (draw_box ? bw_right : 0);
                    
                    if (has_padding) {
                        pdf->SetLeftMargin(old_lm2 + extra_left);
                        pdf->SetRightMargin(old_rm2 + extra_right);
                        pdf->SetX(old_lm2 + extra_left);
                        if (draw_box) {
                            pdf->SetY(y_start_box + bw_top + pad_top);
                        }
                    }
                    
                    double y_after_children;
                    
                    if (draw_box) {
                        // Use dry-run mode for precise height measurement
                        pdf->BeginDryRun();
                        double child_pw = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
                        render_nodes(node->children, new_style, href, child_pw);
                        flush_line_buffer(true);
                        line_buf_lh_ = 0;
                        line_buf_fs_mm_ = 0;
                        line_buf_align_.clear();
                        y_after_children = pdf->GetY();
                        pdf->EndDryRunAndFlush();
                    } else {
                        // No box needed, render directly
                        double child_pw = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
                        render_nodes(node->children, new_style, href, child_pw);
                        flush_line_buffer(true);
                        line_buf_lh_ = 0;
                        line_buf_fs_mm_ = 0;
                        line_buf_align_.clear();
                        y_after_children = pdf->GetY();
                    }
                    
                    if (has_padding) {
                        pdf->SetLeftMargin(old_lm2);
                        pdf->SetRightMargin(old_rm2);
                        pdf->SetX(old_lm2);
                    }
                    
                    // Step 2: Draw box with ACTUAL measured height
                    if (draw_box) {
                        double actual_content_h = y_after_children - (y_start_box + bw_top + pad_top);
                        if (actual_content_h < 0) actual_content_h = 0;
                        double total_h = actual_content_h + pad_top + pad_bottom + bw_top + bw_bottom;
                        
                        std::string saved_fill = pdf->FillColor;
                        std::string saved_draw = pdf->DrawColor;
                        double old_lw = pdf->LineWidth;

                        // === Cross-page box rendering ===
                        if (pdf->DryRunHasPageBreaks()) {
                            int num_pages = pdf->DryRunPageCount();
                            std::vector<double> page_heights;
                            std::vector<std::string> page_contents;
                            for (int i = 0; i < num_pages; i++) {
                                page_heights.push_back(pdf->DryRunPageHeight(i));
                                page_contents.push_back(pdf->DryRunPageContent(i));
                            }
                            // Pop the dry-run stack so subsequent drawing goes to real pages
                            while (pdf->dryrun_stack_.size() > 0) {
                                pdf->dryrun_stack_.pop_back();
                            }
                            pdf->debugmodel = false;
                            pdf->buffer_debug.clear();

                            int br = 0, bg = 0, bb = 0;
                            int lr = 0, lg = 0, lb = 0;
                            bool has_left_border = false;

                            if (has_bg && !new_style.background_color.empty()) {
                                std::istringstream iss(new_style.background_color);
                                iss >> br >> bg >> bb;
                            }
                            if (new_style.border_left_w > 0 && !new_style.border_left_color.empty()) {
                                std::istringstream iss(new_style.border_left_color);
                                iss >> lr >> lg >> lb;
                                has_left_border = true;
                            }

                            double blw = new_style.border_left_w / pdf->k;

                            for (int pi = 0; pi < num_pages; pi++) {
                                double page_content_h = page_heights[pi];
                                std::string page_content = page_contents[pi];

                                bool is_first = (pi == 0);
                                bool is_last = (pi == num_pages - 1);

                                double box_top_y, box_h;

                                if (is_first) {
                                    box_top_y = y_start_box;
                                    box_h = page_content_h + pad_bottom + bw_bottom;
                                } else if (is_last) {
                                    box_top_y = pdf->tMargin;
                                    box_h = page_content_h + pad_bottom + bw_bottom;
                                } else {
                                    box_top_y = pdf->tMargin;
                                    box_h = page_content_h;
                                }

                                if (has_bg || has_left_border) {
                                    if (has_bg && !new_style.background_color.empty()) {
                                        pdf->SetFillColor(br, bg, bb);
                                    }
                                    pdf->_out("q");
                                    pdf->_out(fmt(pdf->k) + " 0 0 " + fmt(-pdf->k) + " 0 " + fmt(pdf->h * pdf->k) + " cm");

                                    std::string path;
                                    double r = (radius > 0 && (is_first || is_last)) ?
                                        std::min(radius, std::min(content_w / 2, box_h / 2)) : 0;

                                    if (r > 0 && is_first && is_last) {
                                        double kb = 0.5522847498 * r;
                                        path += fmt(x_start + r) + " " + fmt(box_top_y) + " m ";
                                        path += fmt(x_start + content_w - r) + " " + fmt(box_top_y) + " l ";
                                        path += fmt(x_start + content_w - r + kb) + " " + fmt(box_top_y) + " " +
                                                fmt(x_start + content_w) + " " + fmt(box_top_y + r - kb) + " " +
                                                fmt(x_start + content_w) + " " + fmt(box_top_y + r) + " c ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h - r) + " l ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h - r + kb) + " " +
                                                fmt(x_start + content_w - r + kb) + " " + fmt(box_top_y + box_h) + " " +
                                                fmt(x_start + content_w - r) + " " + fmt(box_top_y + box_h) + " c ";
                                        path += fmt(x_start + r) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += fmt(x_start + r - kb) + " " + fmt(box_top_y + box_h) + " " +
                                                fmt(x_start) + " " + fmt(box_top_y + box_h - r + kb) + " " +
                                                fmt(x_start) + " " + fmt(box_top_y + box_h - r) + " c ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + r) + " l ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + r - kb) + " " +
                                                fmt(x_start + r - kb) + " " + fmt(box_top_y) + " " +
                                                fmt(x_start + r) + " " + fmt(box_top_y) + " c ";
                                        path += "h";
                                    } else if (r > 0 && is_first) {
                                        double kb = 0.5522847498 * r;
                                        path += fmt(x_start + r) + " " + fmt(box_top_y) + " m ";
                                        path += fmt(x_start + content_w - r) + " " + fmt(box_top_y) + " l ";
                                        path += fmt(x_start + content_w - r + kb) + " " + fmt(box_top_y) + " " +
                                                fmt(x_start + content_w) + " " + fmt(box_top_y + r - kb) + " " +
                                                fmt(x_start + content_w) + " " + fmt(box_top_y + r) + " c ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + r) + " l ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + r - kb) + " " +
                                                fmt(x_start + r - kb) + " " + fmt(box_top_y) + " " +
                                                fmt(x_start + r) + " " + fmt(box_top_y) + " c ";
                                        path += "h";
                                    } else if (r > 0 && is_last) {
                                        double kb = 0.5522847498 * r;
                                        path += fmt(x_start) + " " + fmt(box_top_y) + " m ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y) + " l ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h - r) + " l ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h - r + kb) + " " +
                                                fmt(x_start + content_w - r + kb) + " " + fmt(box_top_y + box_h) + " " +
                                                fmt(x_start + content_w - r) + " " + fmt(box_top_y + box_h) + " c ";
                                        path += fmt(x_start + r) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += fmt(x_start + r - kb) + " " + fmt(box_top_y + box_h) + " " +
                                                fmt(x_start) + " " + fmt(box_top_y + box_h - r + kb) + " " +
                                                fmt(x_start) + " " + fmt(box_top_y + box_h - r) + " c ";
                                        path += "h";
                                    } else {
                                        path += fmt(x_start) + " " + fmt(box_top_y) + " m ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y) + " l ";
                                        path += fmt(x_start + content_w) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += fmt(x_start) + " " + fmt(box_top_y + box_h) + " l ";
                                        path += "h";
                                    }

                                    if (has_left_border && r > 0) {
                                        // Use path as clip for left border alignment with rounded corners
                                        pdf->_out(path);
                                        pdf->_out("W n");
                                        if (has_bg && !new_style.background_color.empty()) {
                                            pdf->_out(path);
                                            pdf->_out("f");
                                        }
                                        pdf->SetFillColor(lr, lg, lb);
                                        // Use raw re operator: already in CTM point space
                                        pdf->_out(fmt(x_start) + " " + fmt(box_top_y) + " " +
                                                  fmt(blw) + " " + fmt(box_h) + " re f");
                                    } else {
                                        pdf->_out(path);
                                        if (has_bg && !new_style.background_color.empty()) {
                                            pdf->_out("f");
                                        } else {
                                            pdf->_out("n");
                                        }
                                        if (has_left_border) {
                                            pdf->SetFillColor(lr, lg, lb);
                                            // Use raw re operator: already in CTM point space
                                            pdf->_out(fmt(x_start) + " " + fmt(box_top_y) + " " +
                                                      fmt(blw) + " " + fmt(box_h) + " re f");
                                        }
                                    }
                                    pdf->_out("Q");
                                }

                                if (!page_content.empty()) {
                                    pdf->WriteCaptured(page_content);
                                }

                                pdf->FillColor = saved_fill;
                                pdf->_out(saved_fill);
                                pdf->ColorFlag = (pdf->FillColor != pdf->TextColor);

                                if (!is_last) {
                                    pdf->AddPage();
                                    apply_style(new_style);
                                }
                            }

                            double last_h = page_heights[num_pages - 1];
                            pdf->SetY(pdf->tMargin + last_h);

                            if (has_padding) {
                                pdf->SetLeftMargin(old_lm2);
                                pdf->SetRightMargin(old_rm2);
                                pdf->SetX(old_lm2);
                            }
                        } else {
                        // === Single-page box rendering (original) ===

                        if (has_bg && !new_style.background_color.empty()) {
                            std::istringstream iss(new_style.background_color);
                            int r = 0, g = 0, b = 0;
                            iss >> r >> g >> b;
                            pdf->SetFillColor(r, g, b);
                        }
                        if (has_border && !new_style.border_color.empty()) {
                            std::istringstream iss2(new_style.border_color);
                            int br = 0, bg = 0, bb = 0;
                            iss2 >> br >> bg >> bb;
                            pdf->SetDrawColor(br, bg, bb);
                        }
                        
                        // Load background image if needed
                        bool is_svg_bg = false;
                        std::shared_ptr<HTMLNode> svg_bg_node;
                        double bg_img_w = 0, bg_img_h = 0;
                        if (has_bg_img) {
                            std::string bg_file = new_style.background_image;
                            if (!is_safe_file_path(bg_file)) {
                                has_bg_img = false;
                            } else {
                                size_t dot = bg_file.find_last_of('.');
                                if (dot != std::string::npos && str_tolower(bg_file.substr(dot + 1)) == "svg") {
                                    is_svg_bg = true;
                                    std::ifstream svg_file(bg_file);
                                    if (svg_file.is_open()) {
                                        std::stringstream ss;
                                        ss << svg_file.rdbuf();
                                        std::string svg_content = ss.str();
                                        auto svg_nodes = parse_html(svg_content);
                                        for (auto& n : svg_nodes) {
                                            if (n->type == HTMLNodeType::ELEMENT && n->tag == "svg") {
                                                svg_bg_node = n;
                                                double sw = 100, sh = 100;
                                                auto w_it = n->attrs.find("width");
                                                auto h_it = n->attrs.find("height");
                                                if (w_it != n->attrs.end()) { try { sw = std::stod(w_it->second); } catch(...) {} }
                                                if (h_it != n->attrs.end()) { try { sh = std::stod(h_it->second); } catch(...) {} }
                                                double px2mm = 0.75 / pdf->k;
                                                bg_img_w = sw * px2mm;
                                                bg_img_h = sh * px2mm;
                                                break;
                                            }
                                        }
                                    }
                                    if (!svg_bg_node) has_bg_img = false;
                                } else {
                                    pdf->GetImageSize(bg_file, bg_img_w, bg_img_h);
                                    bg_img_w *= 0.75;
                                    bg_img_h *= 0.75;
                                }
                            }
                        }
                        
                        double draw_h = total_h;
                        bool needs_clip = has_bg || (has_bg_img && !is_svg_bg);
                        if (needs_clip) {
                            pdf->_out("q");
                            pdf->_out(fmt(pdf->k) + " 0 0 " + fmt(-pdf->k) + " 0 " + fmt(pdf->h * pdf->k) + " cm");
                            
                            std::string path;
                            if (radius > 0) {
                                double r = std::min(radius, std::min(content_w / 2, draw_h / 2));
                                double kb = 0.5522847498 * r;
                                path += fmt(x_start + r) + " " + fmt(y_start_box) + " m ";
                                path += fmt(x_start + content_w - r) + " " + fmt(y_start_box) + " l ";
                                path += fmt(x_start + content_w - r + kb) + " " + fmt(y_start_box) + " " +
                                        fmt(x_start + content_w) + " " + fmt(y_start_box + r - kb) + " " +
                                        fmt(x_start + content_w) + " " + fmt(y_start_box + r) + " c ";
                                path += fmt(x_start + content_w) + " " + fmt(y_start_box + draw_h - r) + " l ";
                                path += fmt(x_start + content_w) + " " + fmt(y_start_box + draw_h - r + kb) + " " +
                                        fmt(x_start + content_w - r + kb) + " " + fmt(y_start_box + draw_h) + " " +
                                        fmt(x_start + content_w - r) + " " + fmt(y_start_box + draw_h) + " c ";
                                path += fmt(x_start + r) + " " + fmt(y_start_box + draw_h) + " l ";
                                path += fmt(x_start + r - kb) + " " + fmt(y_start_box + draw_h) + " " +
                                        fmt(x_start) + " " + fmt(y_start_box + draw_h - r + kb) + " " +
                                        fmt(x_start) + " " + fmt(y_start_box + draw_h - r) + " c ";
                                path += fmt(x_start) + " " + fmt(y_start_box + r) + " l ";
                                path += fmt(x_start) + " " + fmt(y_start_box + r - kb) + " " +
                                        fmt(x_start + r - kb) + " " + fmt(y_start_box) + " " +
                                        fmt(x_start + r) + " " + fmt(y_start_box) + " c ";
                                path += "h";
                            } else {
                                path += fmt(x_start) + " " + fmt(y_start_box) + " m ";
                                path += fmt(x_start + content_w) + " " + fmt(y_start_box) + " l ";
                                path += fmt(x_start + content_w) + " " + fmt(y_start_box + draw_h) + " l ";
                                path += fmt(x_start) + " " + fmt(y_start_box + draw_h) + " l ";
                                path += "h";
                            }
                            
                            if (has_bg_img && !is_svg_bg) {
                                pdf->_out(path);
                                pdf->_out("W n");
                            }
                            if (has_bg) {
                                if (!(has_bg_img && !is_svg_bg)) {
                                    pdf->_out(path);
                                }
                                pdf->_out("f");
                            }
                            if (has_bg_img && !is_svg_bg) {
                                pdf->ImageRaw(new_style.background_image, x_start, y_start_box, bg_img_w, bg_img_h);
                            }
                            pdf->_out("Q");
                        }
                        
                        if (has_bg_img && is_svg_bg && svg_bg_node) {
                            SVGRenderer svg_renderer(pdf);
                            svg_renderer.render(svg_bg_node, x_start, y_start_box);
                        }
                        
                        // Draw border
                        if (has_border) {
                            bool has_side_borders = new_style.border_left_w > 0 || new_style.border_right_w > 0 ||
                                                  new_style.border_top_w > 0 || new_style.border_bottom_w > 0;
                            bool has_full_border = new_style.border_width > 0 && new_style.border_style != "none";
                            
                            // Full border (shorthand) - use draw_rounded_rect for radius case
                            if (has_full_border) {
                                if (radius > 0) {
                                    draw_rounded_rect(x_start, y_start_box, content_w, draw_h, radius, false, true, bw);
                                } else {
                                    pdf->SetLineWidth(bw);
                                    if (new_style.border_style == "dashed") {
                                        pdf->SetDash("3 3", 0);
                                    } else if (new_style.border_style == "dotted") {
                                        pdf->SetDash("1 2", 0);
                                    } else {
                                        pdf->SetDash("");
                                    }
                                    pdf->Rect(x_start, y_start_box, content_w, draw_h, "D");
                                }
                            }
                            
                            // Individual side borders with radius AND background: use rounded clip path
                            // to ensure borders align with rounded background corners
                            if (radius > 0 && has_side_borders && has_bg) {
                                pdf->_out("q");
                                pdf->_out(fmt(pdf->k) + " 0 0 " + fmt(-pdf->k) + " 0 " + fmt(pdf->h * pdf->k) + " cm");
                                double r = std::min(radius, std::min(content_w / 2, draw_h / 2));
                                double kb = 0.5522847498 * r;
                                std::string clip_path;
                                clip_path += fmt(x_start + r) + " " + fmt(y_start_box) + " m ";
                                clip_path += fmt(x_start + content_w - r) + " " + fmt(y_start_box) + " l ";
                                clip_path += fmt(x_start + content_w - r + kb) + " " + fmt(y_start_box) + " " +
                                             fmt(x_start + content_w) + " " + fmt(y_start_box + r - kb) + " " +
                                             fmt(x_start + content_w) + " " + fmt(y_start_box + r) + " c ";
                                clip_path += fmt(x_start + content_w) + " " + fmt(y_start_box + draw_h - r) + " l ";
                                clip_path += fmt(x_start + content_w) + " " + fmt(y_start_box + draw_h - r + kb) + " " +
                                             fmt(x_start + content_w - r + kb) + " " + fmt(y_start_box + draw_h) + " " +
                                             fmt(x_start + content_w - r) + " " + fmt(y_start_box + draw_h) + " c ";
                                clip_path += fmt(x_start + r) + " " + fmt(y_start_box + draw_h) + " l ";
                                clip_path += fmt(x_start + r - kb) + " " + fmt(y_start_box + draw_h) + " " +
                                             fmt(x_start) + " " + fmt(y_start_box + draw_h - r + kb) + " " +
                                             fmt(x_start) + " " + fmt(y_start_box + draw_h - r) + " c ";
                                clip_path += fmt(x_start) + " " + fmt(y_start_box + r) + " l ";
                                clip_path += fmt(x_start) + " " + fmt(y_start_box + r - kb) + " " +
                                             fmt(x_start + r - kb) + " " + fmt(y_start_box) + " " +
                                             fmt(x_start + r) + " " + fmt(y_start_box) + " c ";
                                clip_path += "h";
                                pdf->_out(clip_path);
                                pdf->_out("W n");
                                
                                // Individual side borders (filled rects in CTM point space)
                                auto draw_side_pt = [&](double side_w, const std::string& side_color,
                                                        double rx, double ry, double rw, double rh) {
                                    if (side_w <= 0) return;
                                    if (!side_color.empty()) {
                                        std::istringstream iss(side_color);
                                        int r = 0, g = 0, b = 0;
                                        iss >> r >> g >> b;
                                        pdf->SetFillColor(r, g, b);
                                    }
                                    pdf->_out(fmt(rx) + " " + fmt(ry) + " " +
                                              fmt(rw) + " " + fmt(rh) + " re f");
                                };
                                
                                double blw_pt = new_style.border_left_w / pdf->k;
                                double brw_pt = new_style.border_right_w / pdf->k;
                                double btw_pt = new_style.border_top_w / pdf->k;
                                double bbw_pt = new_style.border_bottom_w / pdf->k;
                                
                                if (new_style.border_left_w > 0)
                                    draw_side_pt(new_style.border_left_w, new_style.border_left_color,
                                                 x_start, y_start_box, blw_pt, draw_h);
                                if (new_style.border_right_w > 0)
                                    draw_side_pt(new_style.border_right_w, new_style.border_right_color,
                                                 x_start + content_w - brw_pt, y_start_box, brw_pt, draw_h);
                                if (new_style.border_top_w > 0)
                                    draw_side_pt(new_style.border_top_w, new_style.border_top_color,
                                                 x_start, y_start_box, content_w, btw_pt);
                                if (new_style.border_bottom_w > 0)
                                    draw_side_pt(new_style.border_bottom_w, new_style.border_bottom_color,
                                                 x_start, y_start_box + draw_h - bbw_pt, content_w, bbw_pt);
                                
                                pdf->_out("Q");
                            }
                            
                            // Individual side borders without radius or without background: simple filled rects
                            if ((radius <= 0 || !has_bg) && has_side_borders) {
                                auto draw_side = [&](double side_w, const std::string& side_color,
                                                     double x1, double y1, double x2, double y2) {
                                    if (side_w <= 0) return;
                                    double sw = side_w / pdf->k;
                                    if (!side_color.empty()) {
                                        std::istringstream iss(side_color);
                                        int r = 0, g = 0, b = 0;
                                        iss >> r >> g >> b;
                                        pdf->SetFillColor(r, g, b);
                                    }
                                    if (x1 == x2) {
                                        double bx = (x1 == x_start) ? x1 : x1 - sw;
                                        pdf->Rect(bx, y1, sw, std::abs(y2 - y1), "F");
                                    } else {
                                        double by = (y1 == y_start_box) ? y1 : y1 - sw;
                                        pdf->Rect(x1, by, std::abs(x2 - x1), sw, "F");
                                    }
                                };
                                
                                draw_side(new_style.border_left_w, new_style.border_left_color,
                                          x_start, y_start_box, x_start, y_start_box + draw_h);
                                draw_side(new_style.border_right_w, new_style.border_right_color,
                                          x_start + content_w, y_start_box, x_start + content_w, y_start_box + draw_h);
                                draw_side(new_style.border_top_w, new_style.border_top_color,
                                          x_start, y_start_box, x_start + content_w, y_start_box);
                                draw_side(new_style.border_bottom_w, new_style.border_bottom_color,
                                          x_start, y_start_box + draw_h, x_start + content_w, y_start_box + draw_h);
                            }
                        }
                        
                        // Restore colors
                        if (has_bg || (has_border && (new_style.border_left_w > 0 || new_style.border_right_w > 0 ||
                                                       new_style.border_top_w > 0 || new_style.border_bottom_w > 0))) {
                            pdf->FillColor = saved_fill;
                            pdf->_out(saved_fill);
                        }
                        if (has_border) {
                            pdf->DrawColor = saved_draw;
                            pdf->SetLineWidth(old_lw);
                            pdf->SetDash("");
                            pdf->_out(saved_draw);
                        }
                        // Recalculate ColorFlag after restoring fill color
                        // (SetFillColor during border drawing may have desynced it)
                        pdf->ColorFlag = (pdf->FillColor != pdf->TextColor);
                        
                        // Draw title on border (fieldset legend style)
                        // Source: title attribute OR <legend> child element
                        std::string box_title;
                        auto title_it = node->attrs.find("title");
                        if (title_it != node->attrs.end() && !title_it->second.empty()) {
                            box_title = title_it->second;
                        }
                        // Also check for <legend> child
                        if (box_title.empty()) {
                            for (auto& child : node->children) {
                                if (child->type == HTMLNodeType::ELEMENT && child->tag == "legend") {
                                    box_title = node_to_text(child);
                                    break;
                                }
                            }
                        }
                        if (!box_title.empty() && has_border) {
                            
                            // Title font: div font-size * 1.2
                            double title_fs = new_style.font_size * 1.2;
                            if (title_fs < 8) title_fs = 8;
                            apply_style(new_style);
                            pdf->SetFont(pdf->FontFamily, pdf->FontStyle, title_fs);
                            
                            // Measure title width
                            double text_w = pdf->GetStringWidth(box_title);
                            double pad = 2.0; // 2mm padding on each side
                            double title_w = text_w + pad * 2;
                            double title_x = x_start + 15; // ~1.5cm from left
                            
                            // Title Y: centered on top border line
                            double title_lh = title_fs * 0.352778 * 1.3;
                            double title_y = y_start_box - title_lh / 2.0;
                            
                            // Erase border at title position (fill with bg color)
                            if (has_bg && !new_style.background_color.empty()) {
                                std::istringstream iss3(new_style.background_color);
                                int tr = 0, tg = 0, tb = 0;
                                iss3 >> tr >> tg >> tb;
                                pdf->SetFillColor(tr, tg, tb);
                                pdf->Rect(title_x, y_start_box - bw - 0.3, title_w, bw * 2 + 0.6, "F");
                            } else {
                                pdf->SetFillColor(255, 255, 255);
                                pdf->Rect(title_x, y_start_box - bw - 0.3, title_w, bw * 2 + 0.6, "F");
                            }
                            
                            // Draw title text in border color
                            if (!new_style.border_color.empty()) {
                                std::istringstream iss4(new_style.border_color);
                                int tcr = 0, tcg = 0, tcb = 0;
                                iss4 >> tcr >> tcg >> tcb;
                                pdf->SetTextColor(tcr, tcg, tcb);
                            }
                            // Account for Cell's cMargin so text is exactly `pad` from left
                            pdf->SetXY(title_x + pad - pdf->cMargin, title_y);
                            pdf->Cell(title_w - pad * 2 + pdf->cMargin * 2, title_lh, box_title, 0, 0, "L");
                            
                            // Restore text color and font
                            apply_style(new_style);
                        }
                        
                        // Step 3: Write dry-run buffer content (on top of background)
                        pdf->FlushDryRunBuffer();
                        
                        // Set final Y position
                        pdf->SetY(y_start_box + total_h);
                        } // close single-page else branch
                    }
                } // close else (normal single-page flow)
                }
                if (mb > 0)
                    pdf->Ln(mb);

                if (new_style.margin_left > 0 || new_style.margin_right > 0 ||
                    new_style.width > 0 || new_style.width_pct > 0 ||
                    new_style.margin_left_auto || new_style.margin_right_auto) {
                    pdf->SetLeftMargin(old_lm);
                    pdf->SetRightMargin(old_rm);
                    pdf->SetX(old_lm);
                }
            } else {
                apply_style(new_style);
                render_nodes(node->children, new_style, href);
                apply_style(current_style);
            }
        }
    }
}

std::string HTMLRenderer::extract_style_tag(const std::string& html) {
    size_t start = 0;
    std::string result;
    std::string lower_html = str_tolower(html);
    while (true) {
        size_t st = lower_html.find("<style", start);
        if (st == std::string::npos) break;
        size_t gt = html.find('>', st);
        if (gt == std::string::npos) break;
        size_t end = lower_html.find("</style>", gt);
        if (end == std::string::npos) break;
        result += html.substr(gt + 1, end - gt - 1);
        start = end + 8;
    }
    return result;
}

void HTMLRenderer::add_stylesheet(const std::string& css) {
    auto parsed = css_parser.parse_stylesheet(css);
    for (auto& kv : parsed) {
        std::string sel = str_tolower(str_trim(kv.first));
        stylesheet[sel] = kv.second;
    }
}

void HTMLRenderer::extract_special_elements(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style) {
    find_special_elements(nodes, page_cover, page_catalogue, catalogue_content, page_header, page_footer, content_nodes);

    if (page_cover) {
        page_cover_style = style;
        auto sit = stylesheet.find("div");
        if (sit != stylesheet.end())
            css_parser.merge_style(page_cover_style, sit->second);
        css_parser.merge_style(page_cover_style, page_cover->style);
    }

    if (page_catalogue) {
        page_catalogue_style = style;
        auto sit = stylesheet.find("div");
        if (sit != stylesheet.end())
            css_parser.merge_style(page_catalogue_style, sit->second);
        css_parser.merge_style(page_catalogue_style, page_catalogue->style);
    }

    if (catalogue_content) {
        catalogue_content_style = style;
        auto sit = stylesheet.find("div");
        if (sit != stylesheet.end())
            css_parser.merge_style(catalogue_content_style, sit->second);
        css_parser.merge_style(catalogue_content_style, catalogue_content->style);
        auto title_it = catalogue_content->attrs.find("title");
        if (title_it != catalogue_content->attrs.end()) {
            toc_title = title_it->second;
        }
    }

    if (page_header) {
        page_header_style = style;
        auto sit = stylesheet.find("div");
        if (sit != stylesheet.end())
            css_parser.merge_style(page_header_style, sit->second);
        css_parser.merge_style(page_header_style, page_header->style);
    }

    if (page_footer) {
        page_footer_style = style;
        auto sit = stylesheet.find("div");
        if (sit != stylesheet.end())
            css_parser.merge_style(page_footer_style, sit->second);
        css_parser.merge_style(page_footer_style, page_footer->style);
    }
}

std::string HTMLRenderer::replace_placeholders(const std::string& text, int pagenum, int pagetotal) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find("{pagenum}", pos)) != std::string::npos) {
        result.replace(pos, 9, std::to_string(pagenum));
        pos += std::to_string(pagenum).length();
    }
    pos = 0;
    while ((pos = result.find("{pagetotal}", pos)) != std::string::npos) {
        result.replace(pos, 11, std::to_string(pagetotal));
        pos += std::to_string(pagetotal).length();
    }
    return result;
}

void HTMLRenderer::replace_placeholders_in_nodes(std::vector<std::shared_ptr<HTMLNode>>& nodes, int pagenum, int pagetotal) {
    for (auto& node : nodes) {
        if (node->type == HTMLNodeType::TEXT) {
            node->text = replace_placeholders(node->text, pagenum, pagetotal);
        } else if (node->type == HTMLNodeType::ELEMENT) {
            replace_placeholders_in_nodes(node->children, pagenum, pagetotal);
        }
    }
}

void HTMLRenderer::render_page_header() {
    if (no_header_footer & 1) return;
    if (!page_header) return;
    if (pdf->PageNo() <= pdf->GetPageNumberOffset()) return;

    bool saved_hf = pdf->GetShowHeaderFooter();
    pdf->SetShowHeaderFooter(false);

    double saved_y = pdf->GetY();
    double saved_x = pdf->GetX();
    
    // Save font state
    std::string saved_family = pdf->FontFamily;
    std::string saved_style_str = pdf->FontStyle;
    double saved_size = pdf->FontSizePt;
    std::string saved_text_color = pdf->TextColor;

    // Save line buffer state
    std::vector<LineBufferSeg> saved_line_buf = line_buf_;
    double saved_line_buf_width = line_buf_width_;
    double saved_line_buf_lh = line_buf_lh_;
    double saved_line_buf_fs_mm = line_buf_fs_mm_;
    std::string saved_line_buf_align = line_buf_align_;

    double header_y = 2;

    int display_page = pdf->PageNo() - pdf->GetPageNumberOffset();
    if (display_page < 1) display_page = 1;

    // First pass: render at lMargin to measure actual width
    // Temporarily disable text-align so content flows from left
    std::string saved_align = page_header_style.text_align;
    page_header_style.text_align.clear();

    line_buf_.clear();
    line_buf_width_ = 0;
    line_buf_lh_ = 0;
    line_buf_fs_mm_ = 0;
    line_buf_align_.clear();

    auto header_clone = clone_nodes(page_header);
    header_clone->attrs.erase("class");
    std::vector<std::shared_ptr<HTMLNode>> header_nodes;
    header_nodes.push_back(header_clone);
    replace_placeholders_in_nodes(header_nodes, display_page, total_pages);

    pdf->SetXY(pdf->lMargin, header_y);
    render_nodes(header_clone->children, page_header_style, "");
    flush_line_buffer(false);

    double final_x = pdf->GetX();
    double actual_width = final_x - pdf->lMargin;

    // Restore text-align
    page_header_style.text_align = saved_align;
    
    // Calculate correct X based on alignment
    double start_x = pdf->lMargin;
    if (!page_header_style.text_align.empty()) {
        if (page_header_style.text_align == "center") {
            start_x = pdf->lMargin + (pdf->w - pdf->lMargin - pdf->rMargin - actual_width) / 2.0;
        } else if (page_header_style.text_align == "right") {
            start_x = pdf->w - pdf->rMargin - actual_width;
        }
        if (start_x < pdf->lMargin) start_x = pdf->lMargin;
    }
    
    // Second pass: render at correct position
    // Clone nodes again since first pass modified them
    auto header_clone2 = clone_nodes(page_header);
    header_clone2->attrs.erase("class");
    std::vector<std::shared_ptr<HTMLNode>> header_nodes2;
    header_nodes2.push_back(header_clone2);
    replace_placeholders_in_nodes(header_nodes2, display_page, total_pages);
    
    // Reset line buffer for second pass
    line_buf_.clear();
    line_buf_width_ = 0;
    line_buf_lh_ = 0;
    line_buf_fs_mm_ = 0;
    line_buf_align_.clear();

    // Restore font state before second render
    try { pdf->SetFont(saved_family, saved_style_str, saved_size); } catch (...) {}
    pdf->TextColor = saved_text_color;
    
    pdf->SetXY(start_x, header_y);
    render_nodes(header_clone2->children, page_header_style, "");
    flush_line_buffer(false);

    // Restore all state
    try { pdf->SetFont(saved_family, saved_style_str, saved_size); } catch (...) {}
    pdf->TextColor = saved_text_color;
    pdf->SetY(saved_y);
    pdf->SetX(saved_x);
    pdf->SetShowHeaderFooter(saved_hf);

    // Restore line buffer state
    line_buf_ = saved_line_buf;
    line_buf_width_ = saved_line_buf_width;
    line_buf_lh_ = saved_line_buf_lh;
    line_buf_fs_mm_ = saved_line_buf_fs_mm;
    line_buf_align_ = saved_line_buf_align;
}

void HTMLRenderer::render_page_footer() {
    if (no_header_footer & 2) return;
    if (!page_footer) return;
    if (pdf->PageNo() <= pdf->GetPageNumberOffset()) return;

    bool saved_hf = pdf->GetShowHeaderFooter();
    pdf->SetShowHeaderFooter(false);

    double saved_y = pdf->GetY();
    double saved_x = pdf->GetX();
    
    // Save font state
    std::string saved_family = pdf->FontFamily;
    std::string saved_style_str = pdf->FontStyle;
    double saved_size = pdf->FontSizePt;
    std::string saved_text_color = pdf->TextColor;

    // Save line buffer state
    std::vector<LineBufferSeg> saved_line_buf = line_buf_;
    double saved_line_buf_width = line_buf_width_;
    double saved_line_buf_lh = line_buf_lh_;
    double saved_line_buf_fs_mm = line_buf_fs_mm_;
    std::string saved_line_buf_align = line_buf_align_;

    double footer_y = pdf->h - pdf->bMargin + 2;

    int display_page = pdf->PageNo() - pdf->GetPageNumberOffset();
    if (display_page < 1) display_page = 1;
    
    // First pass: render at lMargin to measure actual width
    // Temporarily disable text-align so content flows from left
    std::string saved_align = page_footer_style.text_align;
    page_footer_style.text_align.clear();

    line_buf_.clear();
    line_buf_width_ = 0;
    line_buf_lh_ = 0;
    line_buf_fs_mm_ = 0;
    line_buf_align_.clear();

    auto footer_clone = clone_nodes(page_footer);
    footer_clone->attrs.erase("class");
    std::vector<std::shared_ptr<HTMLNode>> footer_nodes;
    footer_nodes.push_back(footer_clone);
    replace_placeholders_in_nodes(footer_nodes, display_page, total_pages);

    pdf->SetXY(pdf->lMargin, footer_y);
    render_nodes(footer_clone->children, page_footer_style, "");
    flush_line_buffer(false);

    double final_x = pdf->GetX();
    double actual_width = final_x - pdf->lMargin;

    // Restore text-align
    page_footer_style.text_align = saved_align;
    
    // Calculate correct X based on alignment
    double start_x = pdf->lMargin;
    if (!page_footer_style.text_align.empty()) {
        if (page_footer_style.text_align == "center") {
            start_x = pdf->lMargin + (pdf->w - pdf->lMargin - pdf->rMargin - actual_width) / 2.0;
        } else if (page_footer_style.text_align == "right") {
            start_x = pdf->w - pdf->rMargin - actual_width;
        }
        if (start_x < pdf->lMargin) start_x = pdf->lMargin;
    }
    
    // Second pass: render at correct position
    auto footer_clone2 = clone_nodes(page_footer);
    footer_clone2->attrs.erase("class");
    std::vector<std::shared_ptr<HTMLNode>> footer_nodes2;
    footer_nodes2.push_back(footer_clone2);
    replace_placeholders_in_nodes(footer_nodes2, display_page, total_pages);
    
    // Reset line buffer for second pass
    line_buf_.clear();
    line_buf_width_ = 0;
    line_buf_lh_ = 0;
    line_buf_fs_mm_ = 0;
    line_buf_align_.clear();

    // Restore font state before second render
    try { pdf->SetFont(saved_family, saved_style_str, saved_size); } catch (...) {}
    pdf->TextColor = saved_text_color;
    
    pdf->SetXY(start_x, footer_y);
    render_nodes(footer_clone2->children, page_footer_style, "");
    flush_line_buffer(false);

    // Restore all state
    try { pdf->SetFont(saved_family, saved_style_str, saved_size); } catch (...) {}
    pdf->TextColor = saved_text_color;

    pdf->SetY(saved_y);
    pdf->SetX(saved_x);
    pdf->SetShowHeaderFooter(saved_hf);

    // Restore line buffer state
    line_buf_ = saved_line_buf;
    line_buf_width_ = saved_line_buf_width;
    line_buf_lh_ = saved_line_buf_lh;
    line_buf_fs_mm_ = saved_line_buf_fs_mm;
    line_buf_align_ = saved_line_buf_align;
}

void HTMLRenderer::WriteHTML(const std::string& html, double line_height) {
    if (!pdf->CurrentFont) {
        return;
    }

    std::string css = extract_style_tag(html);
    if (!css.empty())
        add_stylesheet(css);

    double fs = pdf->FontSizePt;
    double body_lh = 0;

    auto it = stylesheet.find("body");
    if (it != stylesheet.end()) {
        if (it->second.font_size > 0)
            fs = it->second.font_size;
        if (it->second.line_height > 0)
            body_lh = it->second.line_height / pdf->k;
    }

    if (line_height > 0)
        line_h = line_height;
    else if (body_lh > 0)
        line_h = body_lh;
    else
        line_h = fs * 1.2 / pdf->k;

    auto nodes = parse_html(html);
    root_nodes = nodes;

    CSSStyle style;
    style.font_size = fs;
    style.color = "0 0 0";
    css_parser.merge_style(style, base_style);
    auto bit = stylesheet.find("body");
    if (bit != stylesheet.end())
        css_parser.merge_style(style, bit->second);

    extract_special_elements(root_nodes, style);

    collect_toc_entries(content_nodes, style);
    bool has_toc = (catalogue_content != nullptr) && !toc_entries.empty();
    bool has_header = (page_header != nullptr);
    bool has_footer = (page_footer != nullptr);
    bool need_two_pass = has_toc || has_header || has_footer || page_cover || page_catalogue;

    render_header_footer = has_header || has_footer;
    if (render_header_footer) {
        pdf->page_header_cb = [this]() { render_page_header(); };
        pdf->page_footer_cb = [this]() { render_page_footer(); };
    }

    if (need_two_pass) {
        toc_pass1 = true;

        if (page_cover) {
            bool saved_hf = pdf->GetShowHeaderFooter();
            pdf->SetShowHeaderFooter(false);
            double saved_lm = pdf->lMargin;
            double saved_rm = pdf->rMargin;
            double saved_bm = pdf->bMargin;
            bool saved_auto = pdf->AutoPageBreak;

            double cover_margin = 5;
            pdf->SetLeftMargin(cover_margin);
            pdf->SetRightMargin(cover_margin);
            pdf->SetAutoPageBreak(false);
            pdf->AddPage();
            pdf->SetXY(cover_margin, cover_margin);

            if (!page_cover_style.background_image.empty()) {
                double page_w = pdf->GetPageWidth();
                double page_h = pdf->GetPageHeight();
                pdf->Image(page_cover_style.background_image, 0, 0, page_w, page_h);
            }

            apply_style(page_cover_style);
            render_nodes(page_cover->children, page_cover_style, "");

            pdf->SetLeftMargin(saved_lm);
            pdf->SetRightMargin(saved_rm);
            pdf->SetAutoPageBreak(saved_auto, saved_bm);
            pdf->SetShowHeaderFooter(saved_hf);
        }

        if (page_catalogue) {
            bool saved_hf = pdf->GetShowHeaderFooter();
            pdf->SetShowHeaderFooter(false);

            pdf->AddPage();
            toc_page_no = pdf->PageNo();
            
            if (!page_catalogue_style.background_image.empty()) {
                double page_w = pdf->GetPageWidth();
                double page_h = pdf->GetPageHeight();
                pdf->Image(page_catalogue_style.background_image, 0, 0, page_w, page_h);
            }

            apply_style(page_catalogue_style);
            render_nodes(page_catalogue->children, page_catalogue_style, "");

            pdf->SetShowHeaderFooter(saved_hf);
        }

        if (has_toc) {
            bool saved_hf = pdf->GetShowHeaderFooter();
            pdf->SetShowHeaderFooter(false);

            pdf->AddPage();
            if (!page_catalogue) toc_page_no = pdf->PageNo();
            
            const CSSStyle& toc_style = catalogue_content ? catalogue_content_style : page_catalogue_style;
            if (!toc_style.background_image.empty()) {
                double page_w = pdf->GetPageWidth();
                double page_h = pdf->GetPageHeight();
                pdf->Image(toc_style.background_image, 0, 0, page_w, page_h);
            }
            
            render_toc(toc_style);

            pdf->SetPageNumberOffset(pdf->PageNo());
            pdf->SetShowHeaderFooter(saved_hf);
        }

        pdf->SetShowHeaderFooter(render_header_footer);

        double header_h_mm = 0;
        double footer_h_mm = 0;
        if (page_header && page_header_style.height > 0) {
            double content_h = page_header_style.height / pdf->k;
            double cur_lh = get_line_height_mm(page_header_style);
            double fs_mm = page_header_style.font_size * 0.352778;
            double half_leading = (cur_lh - fs_mm) / 2.0;
            if (half_leading < 0) half_leading = 0;
            double after_space = cur_lh * 0.3;
            bool has_bg = page_header_style.has_bg_color && !page_header_style.background_color.empty();
            bool has_border = page_header_style.border_width > 0 && page_header_style.border_style != "none";
            double pad_t = 0, pad_b = 0, bw = 0;
            if (has_bg || has_border) {
                bool has_css_padding = (page_header_style.padding_top > 0 || page_header_style.padding_bottom > 0 ||
                                        page_header_style.padding_left > 0 || page_header_style.padding_right > 0);
                if (has_css_padding) {
                    pad_t = page_header_style.padding_top / pdf->k;
                    pad_b = page_header_style.padding_bottom / pdf->k;
                } else {
                    double default_pad = cur_lh * 0.3;
                    pad_t = pad_b = default_pad;
                }
                if (has_border)
                    bw = page_header_style.border_width / pdf->k;
            }
            double box_h = content_h + pad_t + pad_b + 2 * bw;
            double mb = page_header_style.margin_bottom > 0 ? page_header_style.margin_bottom / pdf->k : (has_bg || has_border ? half_leading + after_space : after_space);
            header_h_mm = box_h + mb;
        }
        if (page_footer && page_footer_style.height > 0) {
            double content_h = page_footer_style.height / pdf->k;
            double cur_lh = get_line_height_mm(page_footer_style);
            double fs_mm = page_footer_style.font_size * 0.352778;
            double half_leading = (cur_lh - fs_mm) / 2.0;
            if (half_leading < 0) half_leading = 0;
            double before_space = cur_lh * 0.3;
            bool has_bg = page_footer_style.has_bg_color && !page_footer_style.background_color.empty();
            bool has_border = page_footer_style.border_width > 0 && page_footer_style.border_style != "none";
            double pad_t = 0, pad_b = 0, bw = 0;
            if (has_bg || has_border) {
                bool has_css_padding = (page_footer_style.padding_top > 0 || page_footer_style.padding_bottom > 0 ||
                                        page_footer_style.padding_left > 0 || page_footer_style.padding_right > 0);
                if (has_css_padding) {
                    pad_t = page_footer_style.padding_top / pdf->k;
                    pad_b = page_footer_style.padding_bottom / pdf->k;
                } else {
                    double default_pad = cur_lh * 0.3;
                    pad_t = pad_b = default_pad;
                }
                if (has_border)
                    bw = page_footer_style.border_width / pdf->k;
            }
            double box_h = content_h + pad_t + pad_b + 2 * bw;
            double mt = page_footer_style.margin_top > 0 ? page_footer_style.margin_top / pdf->k : (has_bg || has_border ? half_leading + before_space : before_space);
            footer_h_mm = box_h + mt;
        }

        double hf_spacing = 5.0;
        double saved_tm = pdf->tMargin;
        double saved_bm = pdf->bMargin;
        bool saved_auto = pdf->AutoPageBreak;
        if (header_h_mm > 0)
            pdf->SetTopMargin(2 + header_h_mm + hf_spacing);
        if (footer_h_mm > 0)
            pdf->SetAutoPageBreak(true, 2 + footer_h_mm + hf_spacing);

        int before_content_page = pdf->PageNo();
        pdf->SetPageNumberOffset(before_content_page);
        pdf->AddPage();
        content_start_page = pdf->PageNo();

        toc_match_index = 0;
        render_nodes(content_nodes, style, "");

        pdf->SetTopMargin(saved_tm);
        pdf->SetAutoPageBreak(saved_auto, saved_bm);

        total_pages = pdf->PageNo() - pdf->GetPageNumberOffset();
        if (total_pages < 1) total_pages = 1;

        for (auto& e : toc_entries) {
            int rel_page = e.page_no - content_start_page + 1;
            if (rel_page < 1) rel_page = 1;
            e.page_no = rel_page;
        }

        toc_pass1 = false;

        auto saved_toc = toc_entries;

        // Save font state before clearing - needed for second pass
        std::string save_ff = pdf->FontFamily;
        std::string save_fs = pdf->FontStyle;
        double save_fpt = pdf->FontSizePt;
        double save_fsz = pdf->FontSize;

        pdf->pages.clear();
        pdf->PageLinks.clear();
        pdf->PageInfo_vec.clear();
        pdf->links.clear();
        pdf->page = 0;
        pdf->state = 0;
        pdf->n = 2;
        pdf->xPos = 0;
        pdf->yPos = 0;

        // Restore font state so SetFont early-return works correctly
        pdf->FontFamily = save_ff;
        pdf->FontStyle = save_fs;
        pdf->FontSizePt = save_fpt;
        pdf->FontSize = save_fsz;
        pdf->FontPage = 0; // force Tf re-emit on second-pass pages

        toc_entries = saved_toc;
        for (auto& e : toc_entries) e.link_id = 0;
    }

    // Second (and optional third) pass: render final PDF
    bool toc_retried = false;
    
    for (int pass = 0; pass < 2; pass++) {
        if (pass > 0) {
            // Clear pages for retry
            pdf->pages.clear();
            pdf->PageLinks.clear();
            pdf->PageInfo_vec.clear();
            pdf->links.clear();
            pdf->page = 0;
            pdf->state = 0;
            pdf->n = 2;
            pdf->xPos = 0;
            pdf->yPos = 0;
            for (auto& e : toc_entries) e.link_id = 0;
            toc_match_index = 0;
            // Re-enable footer for this pass (disabled at end of previous pass)
            if (has_footer)
                pdf->show_footer = true;
        }

        // Save initial TOC page numbers (relative) before rendering
        std::vector<int> initial_toc_pages;
        if (need_two_pass && has_toc && !toc_retried) {
            for (auto& e : toc_entries) {
                initial_toc_pages.push_back(e.page_no);
            }
        }

    if (need_two_pass && page_cover) {
        bool saved_hf = pdf->GetShowHeaderFooter();
        pdf->SetShowHeaderFooter(false);
        double saved_lm = pdf->lMargin;
        double saved_rm = pdf->rMargin;
        double saved_bm = pdf->bMargin;
        bool saved_auto = pdf->AutoPageBreak;

        double cover_margin = 5;
        pdf->SetLeftMargin(cover_margin);
        pdf->SetRightMargin(cover_margin);
        pdf->SetAutoPageBreak(false);
        pdf->AddPage();
        pdf->SetXY(cover_margin, cover_margin);

        if (!page_cover_style.background_image.empty()) {
            double page_w = pdf->GetPageWidth();
            double page_h = pdf->GetPageHeight();
            pdf->Image(page_cover_style.background_image, 0, 0, page_w, page_h);
        }

        apply_style(page_cover_style);
        render_nodes(page_cover->children, page_cover_style, "");

        pdf->SetLeftMargin(saved_lm);
        pdf->SetRightMargin(saved_rm);
        pdf->SetAutoPageBreak(saved_auto, saved_bm);
        pdf->SetShowHeaderFooter(saved_hf);
    }

    // Calculate header/footer heights for second pass
    double header_h_mm2 = 0;
    double footer_h_mm2 = 0;
    if (page_header && page_header_style.height > 0) {
        double content_h = page_header_style.height / pdf->k;
        double cur_lh = get_line_height_mm(page_header_style);
        double fs_mm = page_header_style.font_size * 0.352778;
        double half_leading = (cur_lh - fs_mm) / 2.0;
        if (half_leading < 0) half_leading = 0;
        double after_space = cur_lh * 0.3;
        bool has_bg = page_header_style.has_bg_color && !page_header_style.background_color.empty();
        bool has_border = page_header_style.border_width > 0 && page_header_style.border_style != "none";
        double pad_t = 0, pad_b = 0, bw = 0;
        if (has_bg || has_border) {
            bool has_css_padding = (page_header_style.padding_top > 0 || page_header_style.padding_bottom > 0 ||
                                    page_header_style.padding_left > 0 || page_header_style.padding_right > 0);
            if (has_css_padding) {
                pad_t = page_header_style.padding_top / pdf->k;
                pad_b = page_header_style.padding_bottom / pdf->k;
            } else {
                double default_pad = cur_lh * 0.3;
                pad_t = pad_b = default_pad;
            }
            if (has_border)
                bw = page_header_style.border_width / pdf->k;
        }
        double box_h = content_h + pad_t + pad_b + 2 * bw;
        double mb = page_header_style.margin_bottom > 0 ? page_header_style.margin_bottom / pdf->k : (has_bg || has_border ? half_leading + after_space : after_space);
        header_h_mm2 = box_h + mb;
    }
    if (page_footer && page_footer_style.height > 0) {
        double content_h = page_footer_style.height / pdf->k;
        double cur_lh = get_line_height_mm(page_footer_style);
        double fs_mm = page_footer_style.font_size * 0.352778;
        double half_leading = (cur_lh - fs_mm) / 2.0;
        if (half_leading < 0) half_leading = 0;
        double before_space = cur_lh * 0.3;
        bool has_bg = page_footer_style.has_bg_color && !page_footer_style.background_color.empty();
        bool has_border = page_footer_style.border_width > 0 && page_footer_style.border_style != "none";
        double pad_t = 0, pad_b = 0, bw = 0;
        if (has_bg || has_border) {
            bool has_css_padding = (page_footer_style.padding_top > 0 || page_footer_style.padding_bottom > 0 ||
                                    page_footer_style.padding_left > 0 || page_footer_style.padding_right > 0);
            if (has_css_padding) {
                pad_t = page_footer_style.padding_top / pdf->k;
                pad_b = page_footer_style.padding_bottom / pdf->k;
            } else {
                double default_pad = cur_lh * 0.3;
                pad_t = pad_b = default_pad;
            }
            if (has_border)
                bw = page_footer_style.border_width / pdf->k;
        }
        double box_h = content_h + pad_t + pad_b + 2 * bw;
        double mt = page_footer_style.margin_top > 0 ? page_footer_style.margin_top / pdf->k : (has_bg || has_border ? half_leading + before_space : before_space);
        footer_h_mm2 = box_h + mt;
    }
    double hf_spacing2 = 5.0;

    double toc_page_no_local = 0;

    if (need_two_pass && page_catalogue) {
        bool saved_hf = pdf->GetShowHeaderFooter();
        pdf->SetShowHeaderFooter(false);

        if (render_header_footer) {
            if (header_h_mm2 > 0)
                pdf->SetTopMargin(2 + header_h_mm2 + hf_spacing2);
            if (footer_h_mm2 > 0)
                pdf->SetAutoPageBreak(true, 2 + footer_h_mm2 + hf_spacing2);
        }

        pdf->AddPage();
        toc_page_no_local = pdf->PageNo();
        
        if (!page_catalogue_style.background_image.empty()) {
            double page_w = pdf->GetPageWidth();
            double page_h = pdf->GetPageHeight();
            pdf->Image(page_catalogue_style.background_image, 0, 0, page_w, page_h);
        }

        apply_style(page_catalogue_style);
        render_nodes(page_catalogue->children, page_catalogue_style, "");

        pdf->SetShowHeaderFooter(saved_hf);
    }

    if (need_two_pass && has_toc) {
        bool saved_hf = pdf->GetShowHeaderFooter();
        pdf->SetShowHeaderFooter(false);

        if (render_header_footer) {
            if (header_h_mm2 > 0)
                pdf->SetTopMargin(2 + header_h_mm2 + hf_spacing2);
            if (footer_h_mm2 > 0)
                pdf->SetAutoPageBreak(true, 2 + footer_h_mm2 + hf_spacing2);
        }

        pdf->AddPage();
        if (!page_catalogue) toc_page_no_local = pdf->PageNo();
        
        const CSSStyle& toc_style_pass2 = catalogue_content ? catalogue_content_style : page_catalogue_style;
        if (!toc_style_pass2.background_image.empty()) {
            double page_w = pdf->GetPageWidth();
            double page_h = pdf->GetPageHeight();
            pdf->Image(toc_style_pass2.background_image, 0, 0, page_w, page_h);
        }
        
        render_toc(toc_style_pass2);

        pdf->SetPageNumberOffset(pdf->PageNo());
        pdf->SetShowHeaderFooter(saved_hf);
    }

    pdf->SetShowHeaderFooter(render_header_footer);

    // Apply header/footer margins for content pages
    if (render_header_footer) {
        if (header_h_mm2 > 0)
            pdf->SetTopMargin(2 + header_h_mm2 + hf_spacing2);
        if (footer_h_mm2 > 0)
            pdf->SetAutoPageBreak(true, 2 + footer_h_mm2 + hf_spacing2);
    }

    if (need_two_pass) {
        int before_content_page = pdf->PageNo();
        pdf->SetPageNumberOffset(before_content_page);
        pdf->AddPage();
        content_start_page = pdf->PageNo();
    } else {
        pdf->AddPage();
    }

    toc_match_index = 0;
    render_nodes(need_two_pass ? content_nodes : root_nodes, style, "");

        if (need_two_pass && has_toc && !toc_retried) {
            // Check if TOC page numbers match actual page numbers
            bool need_retry = false;
            for (size_t i = 0; i < toc_entries.size() && i < initial_toc_pages.size(); i++) {
                int actual_rel = toc_entries[i].page_no - pdf->GetPageNumberOffset();
                if (actual_rel < 1) actual_rel = 1;
                if (actual_rel != initial_toc_pages[i]) {
                    need_retry = true;
                    break;
                }
            }
            
            if (need_retry) {
                // Update toc entries with correct relative page numbers
                for (auto& e : toc_entries) {
                    int actual_rel = e.page_no - pdf->GetPageNumberOffset();
                    if (actual_rel < 1) actual_rel = 1;
                    e.page_no = actual_rel;
                }
                toc_retried = true;
                continue;  // retry the pass
            }
        }
        
        toc_page_no = toc_page_no_local;
        break;
    }

    if (!toc_title.empty() && toc_page_no > 0) {
        pdf->AddOutline(toc_title, toc_page_no, 0);
    }
    for (auto& e : toc_entries) {
        int actual_page = e.page_no + pdf->GetPageNumberOffset();
        if (e.link_id > 0 && e.link_id <= (int)pdf->links.size()) {
            actual_page = pdf->links[e.link_id - 1].first;
        }
        pdf->AddOutline(e.title, actual_page, e.level - 1);
    }

    // Explicitly render last page's footer.
    // HTMLRenderer destructor will clear page_footer_cb before Close() is called,
    // so Close() → Footer() would fall through to the default footer (wrong position).
    // Render it here while callbacks are still active, then disable to prevent duplicate.
    if (render_header_footer && has_footer) {
        pdf->InFooter = true;
        render_page_footer();
        pdf->InFooter = false;
        pdf->show_footer = false;  // Prevent Close() from re-rendering
    }
}

// --- TOC methods ---

int HTMLRenderer::get_toc_level(const std::string& class_attr) {
    std::istringstream iss(class_attr);
    std::string c;
    while (iss >> c) {
        if (c == "catalogue1") return 1;
        if (c == "catalogue2") return 2;
        if (c == "catalogue3") return 3;
        if (c == "catalogue4") return 4;
        if (c == "catalogue5") return 5;
        if (c == "catalogue6") return 6;
        if (c == "catalogue7") return 7;
    }
    return 0;
}

void HTMLRenderer::collect_toc_entries(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style) {
    for (auto& n : nodes) {
        if (n->type == HTMLNodeType::ELEMENT) {
            auto class_it = n->attrs.find("class");
            if (class_it != n->attrs.end()) {
                int level = get_toc_level(class_it->second);
                if (level > 0) {
                    TOCEntry entry;
                    entry.title = str_trim(node_to_text(n));
                    entry.level = level;
                    entry.page_no = 1; // will be updated by estimate_pages
                    entry.link_id = 0; // will be set during rendering
                    toc_entries.push_back(entry);
                }
            }
            collect_toc_entries(n->children, style);
        }
    }
}

void HTMLRenderer::estimate_pages(const std::vector<std::shared_ptr<HTMLNode>>& nodes, const CSSStyle& style,
                                   double& y_pos, int& page_no) {
    double page_h = pdf->GetPageHeight();
    double bMargin = pdf->bMargin;
    double tMargin = pdf->tMargin;
    double avail_h = page_h - tMargin - bMargin;
    (void)avail_h;

    for (auto& n : nodes) {
        if (n->type == HTMLNodeType::TEXT) {
            std::string text = str_trim(n->text);
            if (text.empty()) continue;
            double lh = get_line_height_mm(style);
            double content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;
            int lines = pdf->GetMultiCellLines(content_w, text);
            if (lines < 1) lines = 1;
            y_pos += lines * lh;
        } else if (n->type == HTMLNodeType::ELEMENT) {
            CSSStyle new_style = style;
            auto sit = stylesheet.find(n->tag);
            if (sit != stylesheet.end())
                css_parser.merge_style(new_style, sit->second);
            css_parser.merge_style(new_style, n->style);

            double cur_lh = get_line_height_mm(new_style);
            std::string tag = n->tag;

            // Skip non-visible tags
            if (tag == "head" || tag == "title" || tag == "meta" || tag == "link")
                continue;

            // Check for cataloguecontent - skip its children for estimation
            auto class_it = n->attrs.find("class");
            if (class_it != n->attrs.end() && class_it->second.find("cataloguecontent") != std::string::npos)
                continue;

            bool is_block = false;
            double before_space = 0, after_space = 0;

            if (tag == "p") { is_block = true; before_space = cur_lh * 0.5; after_space = cur_lh * 0.5; }
            else if (tag == "div" || tag == "section" || tag == "article") { is_block = true; before_space = cur_lh * 0.3; after_space = cur_lh * 0.3; }
            else if (tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "h5" || tag == "h6") {
                is_block = true;
                if (tag == "h1") { before_space = cur_lh * 1.3; after_space = cur_lh * 1.3; }
                else if (tag == "h2") { before_space = cur_lh * 1.2; after_space = cur_lh * 1.2; }
                else if (tag == "h3") { before_space = cur_lh; after_space = cur_lh; }
                else { before_space = cur_lh * 0.8; after_space = cur_lh * 0.8; }
            }
            else if (tag == "table") { is_block = true; before_space = cur_lh * 0.3; after_space = cur_lh * 0.3; }
            else if (tag == "hr") {
                is_block = true;
                double bw = new_style.border_width > 0 ? new_style.border_width / pdf->k : 0.2;
                double mt = new_style.margin_top > 0 ? new_style.margin_top / pdf->k : cur_lh * 0.5;
                double mb = new_style.margin_bottom > 0 ? new_style.margin_bottom / pdf->k : cur_lh * 0.5;
                before_space = mt;
                after_space = mb + bw;
            }
            else if (tag == "ul" || tag == "ol") { is_block = true; before_space = cur_lh * 0.3; after_space = cur_lh * 0.3; }
            else if (tag == "li") { is_block = true; }

            if (is_block) {
                y_pos += before_space;

                // Check for catalogue entry
                int toc_level = 0;
                if (class_it != n->attrs.end())
                    toc_level = get_toc_level(class_it->second);
                if (toc_level > 0) {
                    // Match TOC entry by text and level
                    std::string node_text = str_trim(node_to_text(n));
                    for (auto& entry : toc_entries) {
                        if (entry.title == node_text && entry.level == toc_level) {
                            entry.page_no = page_no;
                            break;
                        }
                    }
                }

                // Estimate content height
                if (tag == "table" || tag == "img") {
                    // Rough estimate for tables and images
                    y_pos += cur_lh * 3; // assume ~3 rows
                } else {
                    estimate_pages(n->children, new_style, y_pos, page_no);
                }
                y_pos += after_space;
            } else {
                estimate_pages(n->children, new_style, y_pos, page_no);
            }

            // Check page break
            while (y_pos > page_h - bMargin) {
                y_pos -= (page_h - bMargin - tMargin);
                page_no++;
            }
        } else if (n->type == HTMLNodeType::BR) {
            double lh = get_line_height_mm(style);
            y_pos += lh;
            if (y_pos > page_h - bMargin) {
                y_pos = tMargin;
                page_no++;
            }
        }
    }
}

void HTMLRenderer::render_toc(const CSSStyle& style) {
    if (toc_entries.empty()) return;

    double lh = get_line_height_mm(style);
    double content_w = pdf->GetPageWidth() - pdf->lMargin - pdf->rMargin;

    // TOC title
    CSSStyle title_style = style;
    title_style.bold = true;
    title_style.font_size = style.font_size * 1.5;
    apply_style(title_style);
    double title_lh = get_line_height_mm(title_style);

    double y = pdf->GetY();
    double page_h = pdf->GetPageHeight();
    double bMargin = pdf->bMargin;

    if (y + title_lh > page_h - bMargin) {
        pdf->AddPage();
        if (!style.background_image.empty()) {
            double pw = pdf->GetPageWidth();
            double ph = pdf->GetPageHeight();
            pdf->Image(style.background_image, 0, 0, pw, ph);
        }
        y = pdf->GetY();
    }

    // Render TOC title
    std::string title_text = !toc_title.empty() ? toc_title : "目录";
    double title_x = pdf->lMargin + (content_w - pdf->GetStringWidth(title_text)) / 2;
    pdf->Text(title_x, y + title_lh * 0.7, title_text);
    pdf->Ln(title_lh * 1.5);

    // Render each entry
    for (size_t i = 0; i < toc_entries.size(); i++) {
        auto& entry = toc_entries[i];

        // Create internal link
        entry.link_id = pdf->AddLink();

        double indent = (entry.level - 1) * 10.0; // 10mm per level
        double entry_lh = lh;
        double fs = style.font_size;

        // Level 1: bold, larger font
        CSSStyle entry_style = style;
        if (entry.level == 1) {
            entry_style.bold = true;
            entry_style.font_size = fs;
        } else {
            entry_style.font_size = fs * 0.9;
        }

        y = pdf->GetY();
        if (y + entry_lh > page_h - bMargin) {
            pdf->AddPage();
            if (!style.background_image.empty()) {
                double pw = pdf->GetPageWidth();
                double ph = pdf->GetPageHeight();
                pdf->Image(style.background_image, 0, 0, pw, ph);
            }
            y = pdf->GetY();
        }

        double x = pdf->lMargin + indent;
        apply_style(entry_style);

        // Title text
        std::string entry_title_text = entry.title;
        double page_num_w = pdf->GetStringWidth(std::to_string(entry.page_no));
        double dots_area = content_w - indent - pdf->GetStringWidth(entry_title_text) - page_num_w - 5;

        // Build dots
        std::string dots;
        double dot_w = pdf->GetStringWidth(".");
        if (dot_w > 0 && dots_area > 0) {
            int num_dots = (int)(dots_area / dot_w);
            if (num_dots < 3) num_dots = 3;
            for (int d = 0; d < num_dots; d++) dots += ".";
        }

        // Render: indent + title + dots + page_number
        double text_y = y + entry_lh * 0.7;
        pdf->SetX(x);
        pdf->Text(x, text_y, entry_title_text);

        double dots_x = x + pdf->GetStringWidth(entry_title_text) + 2;
        pdf->Text(dots_x, text_y, dots);

        double num_x = pdf->lMargin + content_w - page_num_w;
        pdf->Text(num_x, text_y, std::to_string(entry.page_no));

        // Add clickable link area
        double link_w = content_w - indent;
        pdf->Link(x, y, link_w, entry_lh, entry.link_id);

        pdf->SetY(y + entry_lh);
    }

    pdf->Ln(lh * 0.5);
}

}
