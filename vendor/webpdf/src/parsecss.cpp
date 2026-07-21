#include "parsecss.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace pz {

CSSParser::CSSParser() {}

std::string CSSParser::_trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string CSSParser::_to_lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return r;
}

double CSSParser::_parse_length(const std::string& val, double base) {
    std::string v = _trim(val);
    if (v.empty()) return 0;
    double num = 0;
    std::string unit;
    size_t i = 0;
    while (i < v.size() && (std::isdigit((unsigned char)v[i]) || v[i] == '.' || v[i] == '-' || v[i] == '+'))
        i++;
    if (i == 0) return 0;
    try {
        num = std::stod(v.substr(0, i));
    } catch (const std::exception&) {
        return 0;
    }
    unit = _to_lower(v.substr(i));
    if (unit == "px") return num * 0.75;
    if (unit == "pt") return num;
    if (unit == "em") return num * base;
    if (unit == "%") return num * base / 100.0;
    if (unit == "mm") return num;
    if (unit == "cm") return num * 10;
    return num;
}

std::string CSSParser::_parse_color(const std::string& val) {
    std::string v = _trim(val);
    if (v.empty()) return "";
    if (v[0] == '#') {
        std::string hex = v.substr(1);
        if (hex.size() == 3) {
            std::string full;
            for (char c : hex) { full += c; full += c; }
            hex = full;
        }
        if (hex.size() == 6) {
            try {
                int r = std::stoi(hex.substr(0, 2), nullptr, 16);
                int g = std::stoi(hex.substr(2, 2), nullptr, 16);
                int b = std::stoi(hex.substr(4, 2), nullptr, 16);
                return std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
            } catch (const std::exception&) {
                return "";
            }
        }
    }
    return v;
}

void CSSParser::_apply_property(CSSStyle& style, const std::string& prop, const std::string& val) {
    std::string p = _to_lower(prop);
    std::string v = _trim(val);

    if (p == "font-family") {
        style.font_family = v;
        if (!style.font_family.empty() && style.font_family.front() == '\'')
            style.font_family = style.font_family.substr(1, style.font_family.size() - 2);
    } else if (p == "font-size") {
        style.font_size = _parse_length(v);
    } else if (p == "font-weight") {
        style.font_weight = _to_lower(v);
        if (style.font_weight == "bold" || style.font_weight == "700" || style.font_weight == "800" || style.font_weight == "900")
            style.bold = true;
    } else if (p == "font-style") {
        style.font_style = _to_lower(v);
        if (style.font_style == "italic" || style.font_style == "oblique")
            style.italic = true;
    } else if (p == "text-decoration") {
        std::string td = _to_lower(v);
        if (td.find("underline") != std::string::npos)
            style.underline = true;
    } else if (p == "color") {
        style.color = _parse_color(v);
    } else if (p == "background-image") {
        std::string lower_v = _to_lower(v);
        size_t url_pos = lower_v.find("url(");
        if (url_pos != std::string::npos) {
            size_t url_start = url_pos + 4;
            size_t url_end = v.find(')', url_start);
            if (url_end != std::string::npos) {
                std::string url = _trim(v.substr(url_start, url_end - url_start));
                if (!url.empty() && (url.front() == '"' || url.front() == '\'')) {
                    url = url.substr(1, url.size() - 2);
                }
                style.background_image = url;
            }
        }
    } else if (p == "background-color") {
        style.background_color = _parse_color(v);
        style.has_bg_color = !style.background_color.empty();
    } else if (p == "background") {
        std::string lower_v = _to_lower(v);
        size_t url_pos = lower_v.find("url(");
        if (url_pos != std::string::npos) {
            size_t url_start = url_pos + 4;
            size_t url_end = v.find(')', url_start);
            if (url_end != std::string::npos) {
                std::string url = _trim(v.substr(url_start, url_end - url_start));
                if (!url.empty() && (url.front() == '"' || url.front() == '\'')) {
                    url = url.substr(1, url.size() - 2);
                }
                style.background_image = url;
            }
        }
        std::string color_part = v;
        if (url_pos != std::string::npos) {
            size_t url_end = v.find(')', url_pos);
            if (url_end != std::string::npos) {
                color_part = _trim(v.substr(0, url_pos) + v.substr(url_end + 1));
            }
        }
        if (!color_part.empty()) {
            std::string c = _parse_color(color_part);
            if (!c.empty()) {
                style.background_color = c;
                style.has_bg_color = true;
            }
        }
    } else if (p == "text-align") {
        style.text_align = _to_lower(v);
    } else if (p == "line-height") {
        style.line_height = _parse_length(v, 12.0);
    } else if (p == "margin-top") {
        style.margin_top = _parse_length(v);
    } else if (p == "margin-bottom") {
        style.margin_bottom = _parse_length(v);
    } else if (p == "margin-left") {
        if (_to_lower(_trim(v)) == "auto") {
            style.margin_left_auto = true;
        } else {
            style.margin_left = _parse_length(v);
        }
    } else if (p == "margin-right") {
        if (_to_lower(_trim(v)) == "auto") {
            style.margin_right_auto = true;
        } else {
            style.margin_right = _parse_length(v);
        }
    } else if (p == "margin") {
        std::istringstream iss(v);
        std::vector<std::string> parts;
        std::string part;
        while (iss >> part) parts.push_back(part);
        
        if (parts.size() == 1) {
            std::string lower = _to_lower(_trim(parts[0]));
            if (lower == "auto") {
                style.margin_left_auto = style.margin_right_auto = true;
            } else {
                double m = _parse_length(parts[0]);
                style.margin_top = style.margin_bottom = style.margin_left = style.margin_right = m;
            }
        } else if (parts.size() == 2) {
            double mv = _parse_length(parts[0]);
            std::string lower = _to_lower(_trim(parts[1]));
            if (lower == "auto") {
                style.margin_top = style.margin_bottom = mv;
                style.margin_left_auto = style.margin_right_auto = true;
            } else {
                double mh = _parse_length(parts[1]);
                style.margin_top = style.margin_bottom = mv;
                style.margin_left = style.margin_right = mh;
            }
        } else if (parts.size() == 3) {
            style.margin_top = _parse_length(parts[0]);
            std::string lower = _to_lower(_trim(parts[1]));
            if (lower == "auto") {
                style.margin_left_auto = style.margin_right_auto = true;
            } else {
                double ml = _parse_length(parts[1]);
                style.margin_left = style.margin_right = ml;
            }
            style.margin_bottom = _parse_length(parts[2]);
        } else if (parts.size() == 4) {
            style.margin_top = _parse_length(parts[0]);
            std::string mr_lower = _to_lower(_trim(parts[1]));
            if (mr_lower == "auto") {
                style.margin_right_auto = true;
            } else {
                style.margin_right = _parse_length(parts[1]);
            }
            style.margin_bottom = _parse_length(parts[2]);
            std::string ml_lower = _to_lower(_trim(parts[3]));
            if (ml_lower == "auto") {
                style.margin_left_auto = true;
            } else {
                style.margin_left = _parse_length(parts[3]);
            }
        }
    } else if (p == "padding-top") {
        style.padding_top = _parse_length(v);
    } else if (p == "padding-bottom") {
        style.padding_bottom = _parse_length(v);
    } else if (p == "padding-left") {
        style.padding_left = _parse_length(v);
    } else if (p == "padding-right") {
        style.padding_right = _parse_length(v);
    } else if (p == "padding") {
        std::istringstream iss(v);
        std::vector<std::string> parts;
        std::string part;
        while (iss >> part) parts.push_back(part);
        if (parts.size() == 1) {
            double p2 = _parse_length(parts[0]);
            style.padding_top = style.padding_bottom = style.padding_left = style.padding_right = p2;
        } else if (parts.size() == 2) {
            double pv = _parse_length(parts[0]);
            double ph = _parse_length(parts[1]);
            style.padding_top = style.padding_bottom = pv;
            style.padding_left = style.padding_right = ph;
        } else if (parts.size() == 3) {
            style.padding_top = _parse_length(parts[0]);
            style.padding_left = style.padding_right = _parse_length(parts[1]);
            style.padding_bottom = _parse_length(parts[2]);
        } else if (parts.size() == 4) {
            style.padding_top = _parse_length(parts[0]);
            style.padding_right = _parse_length(parts[1]);
            style.padding_bottom = _parse_length(parts[2]);
            style.padding_left = _parse_length(parts[3]);
        }
    } else if (p == "width") {
        std::string tv = _trim(v);
        if (!tv.empty() && tv.back() == '%') {
            std::string num_str = tv.substr(0, tv.size()-1);
            try { style.width_pct = std::stod(num_str); } catch(...) {}
        } else {
            style.width = _parse_length(v);
        }
    } else if (p == "height") {
        style.height = _parse_length(v);
    } else if (p == "justify-content") {
        style.justify_content = _to_lower(_trim(v));
    } else if (p == "border-style") {
        style.border_style = _to_lower(v);
        if (style.border_style != "none" && style.border_width == 0) {
            style.border_width = 1;
        }
    } else if (p == "border-width") {
        style.border_width = _parse_length(v);
    } else if (p == "border-color") {
        style.border_color = _parse_color(v);
    } else if (p == "border") {
        std::vector<std::string> parts;
        std::string v_trimmed = _trim(v);
        size_t pos = 0;
        while (pos < v_trimmed.size()) {
            while (pos < v_trimmed.size() && v_trimmed[pos] == ' ') pos++;
            if (pos >= v_trimmed.size()) break;
            size_t end = v_trimmed.find(' ', pos);
            if (end == std::string::npos) end = v_trimmed.size();
            parts.push_back(v_trimmed.substr(pos, end - pos));
            pos = end;
        }
        for (auto& part : parts) {
            std::string p_lower = _to_lower(part);
            if (p_lower.find("px") != std::string::npos || p_lower.find("pt") != std::string::npos || 
                (p_lower.size() > 0 && isdigit(p_lower[0]))) {
                style.border_width = _parse_length(part);
            } else if (p_lower == "solid" || p_lower == "dashed" || p_lower == "dotted" || p_lower == "none") {
                style.border_style = p_lower;
            } else {
                std::string c = _parse_color(part);
                if (!c.empty())
                    style.border_color = c;
            }
        }
        if (style.border_style != "none" && !style.border_style.empty() && style.border_width == 0) {
            style.border_width = 1;
        }
    } else if (p == "border-radius") {
        style.border_radius = _parse_length(v);
    } else if (p == "border-left" || p == "border-right" || p == "border-top" || p == "border-bottom") {
        // Parse individual border side: "6px solid #f44336"
        double bw = 0;
        std::string bs;
        std::string bc;
        std::vector<std::string> parts;
        std::string v_trimmed = _trim(v);
        size_t pos = 0;
        while (pos < v_trimmed.size()) {
            while (pos < v_trimmed.size() && v_trimmed[pos] == ' ') pos++;
            if (pos >= v_trimmed.size()) break;
            size_t end = v_trimmed.find(' ', pos);
            if (end == std::string::npos) end = v_trimmed.size();
            parts.push_back(v_trimmed.substr(pos, end - pos));
            pos = end;
        }
        for (auto& part : parts) {
            std::string p_lower = _to_lower(part);
            if (p_lower.find("px") != std::string::npos || p_lower.find("pt") != std::string::npos || 
                (p_lower.size() > 0 && isdigit(p_lower[0]))) {
                bw = _parse_length(part);
            } else if (p_lower == "solid" || p_lower == "dashed" || p_lower == "dotted" || p_lower == "none") {
                bs = p_lower;
            } else {
                std::string c = _parse_color(part);
                if (!c.empty()) bc = c;
            }
        }
        if (p == "border-left") { style.border_left_w = bw; style.border_left_color = bc; }
        else if (p == "border-right") { style.border_right_w = bw; style.border_right_color = bc; }
        else if (p == "border-top") { style.border_top_w = bw; style.border_top_color = bc; }
        else if (p == "border-bottom") { style.border_bottom_w = bw; style.border_bottom_color = bc; }
    } else if (p == "border-left-width") {
        style.border_left_w = _parse_length(v);
    } else if (p == "border-right-width") {
        style.border_right_w = _parse_length(v);
    } else if (p == "border-top-width") {
        style.border_top_w = _parse_length(v);
    } else if (p == "border-bottom-width") {
        style.border_bottom_w = _parse_length(v);
    } else if (p == "border-left-color") {
        style.border_left_color = _parse_color(v);
    } else if (p == "border-right-color") {
        style.border_right_color = _parse_color(v);
    } else if (p == "border-top-color") {
        style.border_top_color = _parse_color(v);
    } else if (p == "border-bottom-color") {
        style.border_bottom_color = _parse_color(v);
    } else if (p == "position") {
        style.position = _to_lower(v);
    } else if (p == "page-break-before") {
        if (_to_lower(v) == "always")
            style.page_break_before = true;
    } else if (p == "vertical-align") {
        style.vertical_align = _to_lower(v);
    } else if (p == "text-indent") {
        style.text_indent = _parse_length(v);
    } else if (p == "letter-spacing") {
        style.letter_spacing = _parse_length(v);
    } else if (p == "word-spacing") {
        style.word_spacing = _parse_length(v);
    } else if (p == "white-space") {
        style.white_space = _to_lower(v);
    } else if (p == "overflow") {
        style.overflow = _to_lower(v);
    } else if (p == "text-transform") {
        style.text_transform = _to_lower(v);
    } else if (p == "direction") {
        style.direction = _to_lower(v);
    } else if (p == "font-variant") {
        style.font_variant = _to_lower(v);
        if (style.font_variant == "small-caps")
            style.small_caps = true;
    } else if (p == "text-shadow") {
        style.text_shadow = v;
    } else if (p == "display") {
        style.display = _to_lower(v);
    } else if (p == "opacity") {
        try { style.opacity = std::stod(v); } catch(...) {}
    } else if (p == "top") {
        style.top = _parse_length(v);
    } else if (p == "right") {
        style.right = _parse_length(v);
    } else if (p == "bottom") {
        style.bottom = _parse_length(v);
    } else if (p == "left") {
        style.left = _parse_length(v);
    } else if (p == "float") {
        style.float_val = _to_lower(v);
    } else if (p == "clear") {
        style.clear_val = _to_lower(v);
    } else if (p == "z-index") {
        try { style.z_index = std::stoi(v); } catch(...) {}
    } else if (p == "list-style-type") {
        style.list_style_type = _to_lower(v);
    } else if (p == "list-style-position") {
        style.list_style_position = _to_lower(v);
    } else if (p == "table-layout") {
        style.table_layout = _to_lower(v);
    } else if (p == "border-collapse") {
        style.border_collapse = _to_lower(v);
    } else if (p == "caption-side") {
        style.caption_side = _to_lower(v);
    } else if (p == "empty-cells") {
        style.empty_cells = _to_lower(v);
    } else if (p == "quotes") {
        style.quotes = v;
    } else if (p == "content") {
        style.content = v;
    } else if (p == "background-repeat") {
        style.background_repeat = _to_lower(v);
    } else if (p == "background-position") {
        style.background_position = v;
    } else if (p == "background-size") {
        style.background_size = v;
    } else if (p == "background-attachment") {
        style.background_attachment = _to_lower(v);
    }
}

CSSStyle CSSParser::parse_inline(const std::string& style_str) {
    CSSStyle style;
    std::string s = style_str;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t sc = s.find(';', pos);
        std::string decl;
        if (sc == std::string::npos) {
            decl = s.substr(pos);
            pos = s.size();
        } else {
            decl = s.substr(pos, sc - pos);
            pos = sc + 1;
        }
        size_t co = decl.find(':');
        if (co != std::string::npos) {
            std::string prop = _trim(decl.substr(0, co));
            std::string val = _trim(decl.substr(co + 1));
            if (!prop.empty())
                _apply_property(style, prop, val);
        }
    }
    return style;
}

std::map<std::string, CSSStyle> CSSParser::parse_stylesheet(const std::string& css) {
    std::map<std::string, CSSStyle> styles;
    std::string s = css;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t ob = s.find('{', pos);
        if (ob == std::string::npos) break;
        std::string selector = _trim(s.substr(pos, ob - pos));
        size_t cb = s.find('}', ob);
        if (cb == std::string::npos) break;
        std::string body = s.substr(ob + 1, cb - ob - 1);
        CSSStyle style = parse_inline(body);
        if (!selector.empty()) {
            std::istringstream ss(selector);
            std::string sel;
            while (std::getline(ss, sel, ',')) {
                std::string s2 = _trim(sel);
                if (!s2.empty())
                    styles[s2] = style;
            }
        }
        pos = cb + 1;
    }
    return styles;
}

void CSSParser::merge_style(CSSStyle& dest, const CSSStyle& src) {
    if (!src.font_family.empty()) dest.font_family = src.font_family;
    if (src.font_size > 0) dest.font_size = src.font_size;
    if (!src.font_weight.empty()) dest.font_weight = src.font_weight;
    if (src.bold) dest.bold = true;
    if (!src.font_style.empty()) dest.font_style = src.font_style;
    if (src.italic) dest.italic = true;
    if (src.underline) dest.underline = true;
    if (!src.color.empty()) dest.color = src.color;
    if (src.has_bg_color) { dest.background_color = src.background_color; dest.has_bg_color = true; }
    if (!src.background_image.empty()) dest.background_image = src.background_image;
    if (!src.text_align.empty()) dest.text_align = src.text_align;
    if (src.line_height > 0) dest.line_height = src.line_height;
    if (src.margin_top > 0) dest.margin_top = src.margin_top;
    if (src.margin_bottom > 0) dest.margin_bottom = src.margin_bottom;
    if (src.margin_left > 0) dest.margin_left = src.margin_left;
    if (src.margin_right > 0) dest.margin_right = src.margin_right;
    if (src.padding_top > 0) dest.padding_top = src.padding_top;
    if (src.padding_bottom > 0) dest.padding_bottom = src.padding_bottom;
    if (src.padding_left > 0) dest.padding_left = src.padding_left;
    if (src.padding_right > 0) dest.padding_right = src.padding_right;
    if (src.width > 0) dest.width = src.width;
    if (src.width_pct > 0) dest.width_pct = src.width_pct;
    if (src.height > 0) dest.height = src.height;
    if (!src.border_style.empty()) dest.border_style = src.border_style;
    if (src.border_width > 0) dest.border_width = src.border_width;
    if (!src.border_color.empty()) dest.border_color = src.border_color;
    if (src.border_radius > 0) dest.border_radius = src.border_radius;
    // Individual border sides
    if (src.border_left_w > 0) dest.border_left_w = src.border_left_w;
    if (src.border_right_w > 0) dest.border_right_w = src.border_right_w;
    if (src.border_top_w > 0) dest.border_top_w = src.border_top_w;
    if (src.border_bottom_w > 0) dest.border_bottom_w = src.border_bottom_w;
    if (!src.border_left_color.empty()) dest.border_left_color = src.border_left_color;
    if (!src.border_right_color.empty()) dest.border_right_color = src.border_right_color;
    if (!src.border_top_color.empty()) dest.border_top_color = src.border_top_color;
    if (!src.border_bottom_color.empty()) dest.border_bottom_color = src.border_bottom_color;
    if (!src.position.empty()) dest.position = src.position;
    if (src.page_break_before) dest.page_break_before = true;
    if (!src.vertical_align.empty()) dest.vertical_align = src.vertical_align;
    if (!src.justify_content.empty()) dest.justify_content = src.justify_content;
    if (src.margin_left_auto) dest.margin_left_auto = true;
    if (src.margin_right_auto) dest.margin_right_auto = true;
}

}
