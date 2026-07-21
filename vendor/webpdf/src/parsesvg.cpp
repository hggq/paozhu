#define _USE_MATH_DEFINES
#include "parsesvg.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace pz {

std::string SVGRenderer::fmt(double v, int decimals) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << v;
    return oss.str();
}

std::string SVGRenderer::str_trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string SVGRenderer::str_tolower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

double SVGRenderer::parse_num(const std::string& s, double default_val) {
    std::string t = str_trim(s);
    if (t.empty()) return default_val;
    size_t i = 0;
    while (i < t.size() && (std::isdigit((unsigned char)t[i]) || t[i] == '.' || t[i] == '-' || t[i] == '+' || t[i] == 'e' || t[i] == 'E'))
        i++;
    if (i == 0) return default_val;
    try {
        return std::stod(t.substr(0, i));
    } catch (...) {
        return default_val;
    }
}

// Parse a gradient coordinate that may contain a percentage (e.g., "100%" → 1.0)
double SVGRenderer::parse_gradient_coord(const std::string& s, double default_val) {
    std::string t = str_trim(s);
    if (t.empty()) return default_val;
    if (!t.empty() && t.back() == '%') {
        try {
            return std::stod(t.substr(0, t.size() - 1)) / 100.0;
        } catch (...) {
            return default_val;
        }
    }
    return parse_num(t, default_val);
}

static double parse_length_mm(const std::string& s, double default_mm) {
    std::string t;
    for (char c : s) t += std::tolower((unsigned char)c);
    size_t start = t.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return default_mm;
    size_t end = t.find_last_not_of(" \t\n\r");
    t = t.substr(start, end - start + 1);
    if (t.empty()) return default_mm;
    
    size_t i = 0;
    while (i < t.size() && (std::isdigit((unsigned char)t[i]) || t[i] == '.' || t[i] == '-' || t[i] == '+' || t[i] == 'e' || t[i] == 'E'))
        i++;
    if (i == 0) return default_mm;
    double val = 0;
    try {
        val = std::stod(t.substr(0, i));
    } catch (...) {
        return default_mm;
    }
    if (val == 0) return default_mm;
    
    if (t.find("mm") != std::string::npos) {
        return val;
    } else if (t.find("cm") != std::string::npos) {
        return val * 10.0;
    } else if (t.find("in") != std::string::npos) {
        return val * 25.4;
    } else if (t.find("pt") != std::string::npos) {
        return val * 25.4 / 72.0;
    } else if (t.find("px") != std::string::npos) {
        return val * 25.4 / 96.0;
    } else {
        return val * 25.4 / 96.0;
    }
}

std::vector<double> SVGRenderer::parse_num_list(const std::string& s) {
    std::vector<double> result;
    std::string t = s;
    std::replace(t.begin(), t.end(), ',', ' ');
    std::istringstream iss(t);
    double val;
    while (iss >> val)
        result.push_back(val);
    return result;
}

void SVGRenderer::parse_color(const std::string& s, int& r, int& g, int& b, bool& valid) {
    valid = false;
    r = g = b = 0;
    std::string v = str_trim(s);
    if (v.empty() || v == "none" || v == "null") return;

    if (v[0] == '#') {
        std::string hex = v.substr(1);
        if (hex.size() == 3) {
            std::string full;
            for (char c : hex) { full += c; full += c; }
            hex = full;
        }
        if (hex.size() == 6) {
            try {
                r = std::stoi(hex.substr(0, 2), nullptr, 16);
                g = std::stoi(hex.substr(2, 2), nullptr, 16);
                b = std::stoi(hex.substr(4, 2), nullptr, 16);
                valid = true;
            } catch (const std::exception&) {
                r = g = b = 0;
                valid = false;
            }
        }
        return;
    }

    if (v.substr(0, 4) == "rgb(") {
        std::string inner = v.substr(4);
        size_t paren = inner.find(')');
        if (paren != std::string::npos) inner = inner.substr(0, paren);
        std::replace(inner.begin(), inner.end(), ',', ' ');
        std::istringstream iss(inner);
        iss >> r >> g >> b;
        valid = true;
        return;
    }

    std::string lower = str_tolower(v);
    if (lower == "black")   { r = 0;   g = 0;   b = 0;   valid = true; }
    else if (lower == "white")   { r = 255; g = 255; b = 255; valid = true; }
    else if (lower == "red")     { r = 255; g = 0;   b = 0;   valid = true; }
    else if (lower == "green")   { r = 0;   g = 128; b = 0;   valid = true; }
    else if (lower == "blue")    { r = 0;   g = 0;   b = 255; valid = true; }
    else if (lower == "yellow")  { r = 255; g = 255; b = 0;   valid = true; }
    else if (lower == "cyan")    { r = 0;   g = 255; b = 255; valid = true; }
    else if (lower == "magenta") { r = 255; g = 0;   b = 255; valid = true; }
    else if (lower == "gray" || lower == "grey") { r = 128; g = 128; b = 128; valid = true; }
    else if (lower == "orange")  { r = 255; g = 165; b = 0;   valid = true; }
    else if (lower == "purple")  { r = 128; g = 0;   b = 128; valid = true; }
    else if (lower == "brown")   { r = 165; g = 42;  b = 42;  valid = true; }
    else if (lower == "pink")    { r = 255; g = 192; b = 203; valid = true; }
    else if (lower == "navy")    { r = 0;   g = 0;   b = 128; valid = true; }
    else if (lower == "teal")    { r = 0;   g = 128; b = 128; valid = true; }
}

SVGRenderer::SVGRenderer(webpdf* p) : pdf(p), origin_x(0), origin_y(0), svg_width(0), svg_height(0) {}

void SVGRenderer::render(const std::shared_ptr<HTMLNode>& svg_node, double x, double y,
                         double target_w_mm, double target_h_mm) {
    origin_x = x;
    origin_y = y;

    double w_num = 100.0;
    double h_num = 100.0;
    double w_mm = 100.0 * 25.4 / 96.0;
    double h_mm = 100.0 * 25.4 / 96.0;
    double vb_w = 0.0;
    double vb_h = 0.0;
    double vb_x = 0.0;
    double vb_y = 0.0;

    auto w_it = svg_node->attrs.find("width");
    auto h_it = svg_node->attrs.find("height");
    if (w_it != svg_node->attrs.end()) {
        w_num = parse_num(w_it->second, w_num);
        w_mm = parse_length_mm(w_it->second, w_mm);
    }
    if (h_it != svg_node->attrs.end()) {
        h_num = parse_num(h_it->second, h_num);
        h_mm = parse_length_mm(h_it->second, h_mm);
    }

    // A caller-supplied box overrides the SVG's intrinsic width/height so the
    // drawing is scaled to the requested size (viewBox mapping is preserved).
    if (target_w_mm > 0) w_mm = target_w_mm;
    if (target_h_mm > 0) h_mm = target_h_mm;

    auto vb_it = svg_node->attrs.find("viewbox");
    if (vb_it != svg_node->attrs.end()) {
        auto nums = parse_num_list(vb_it->second);
        if (nums.size() >= 4) {
            vb_x = nums[0];
            vb_y = nums[1];
            vb_w = nums[2];
            vb_h = nums[3];
        }
    }

    if (vb_w == 0 || vb_h == 0) {
        vb_w = w_num;
        vb_h = h_num;
    }

    double scale_x = (pdf->k * w_mm) / vb_w;
    double scale_y = (pdf->k * h_mm) / vb_h;
    ctm_scale_x = scale_x;
    ctm_scale_y = scale_y;

    svg_width = vb_w;
    svg_height = vb_h;

    std::string saved_fill = pdf->FillColor;
    std::string saved_draw = pdf->DrawColor;
    double saved_lw = pdf->LineWidth;

    pdf->_out("q");
    double e = pdf->k * origin_x - scale_x * vb_x;
    double f = pdf->k * (pdf->h - origin_y) + scale_y * vb_y;
    pdf->_out(fmt(scale_x) + " 0 0 " + fmt(-scale_y) + " " + fmt(e) + " " + fmt(f) + " cm");

    gradients.clear();
    collect_defs(svg_node->children);

    render_children(svg_node->children);

    pdf->_out("Q");

    pdf->FillColor = saved_fill;
    pdf->DrawColor = saved_draw;
    pdf->LineWidth = saved_lw;
    pdf->_out(pdf->FillColor);
    pdf->_out(pdf->DrawColor);
}

void SVGRenderer::render_children(const std::vector<std::shared_ptr<HTMLNode>>& children) {
    if (render_depth_ >= MAX_RENDER_DEPTH)
        return; // guard against deeply nested <g> groups causing stack overflow
    render_depth_++;
    struct DepthGuard { int& d; ~DepthGuard() { d--; } } _dg{render_depth_};
    for (auto& child : children) {
        if (child->type == HTMLNodeType::ELEMENT) {
            render_element(child);
        }
    }
}

static void parse_svg_style(const std::string& style, std::map<std::string, std::string>& style_props) {
    std::string s = style;
    size_t pos = 0;
    while (pos < s.size()) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == ';')) pos++;
        if (pos >= s.size()) break;
        size_t colon = s.find(':', pos);
        if (colon == std::string::npos) break;
        std::string name_part = s.substr(pos, colon - pos);
        while (!name_part.empty() && (name_part.front() == ' ' || name_part.front() == '\t'))
            name_part.erase(name_part.begin());
        while (!name_part.empty() && (name_part.back() == ' ' || name_part.back() == '\t'))
            name_part.pop_back();
        std::string name;
        for (char c : name_part) name += std::tolower((unsigned char)c);
        size_t semicolon = s.find(';', colon + 1);
        std::string value;
        if (semicolon == std::string::npos) {
            value = s.substr(colon + 1);
            pos = s.size();
        } else {
            value = s.substr(colon + 1, semicolon - colon - 1);
            pos = semicolon + 1;
        }
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
            value.erase(value.begin());
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
            value.pop_back();
        if (!name.empty()) {
            style_props[name] = value;
        }
    }
}

void SVGRenderer::collect_defs(const std::vector<std::shared_ptr<HTMLNode>>& children) {
    for (auto& child : children) {
        if (child->type != HTMLNodeType::ELEMENT) continue;
        std::string tag = str_tolower(child->tag);
        if (tag == "defs" || tag == "g") {
            collect_defs(child->children);
        } else if (tag == "lineargradient" || tag == "radialgradient") {
            GradientDef grad;
            grad.is_linear = (tag == "lineargradient");
            auto id_it = child->attrs.find("id");
            if (id_it == child->attrs.end()) continue;
            grad.id = str_tolower(id_it->second);
            auto href_it = child->attrs.find("xlink:href");
            if (href_it == child->attrs.end()) {
                href_it = child->attrs.find("href");
            }
            std::string href_id;
            if (href_it != child->attrs.end()) {
                std::string href = href_it->second;
                if (href.size() > 1 && href[0] == '#') {
                    href_id = str_tolower(href.substr(1));
                }
            }
            auto gu_it = child->attrs.find("gradientunits");
            if (gu_it != child->attrs.end()) {
                grad.gradient_units = str_tolower(gu_it->second);
            }
            if (grad.is_linear) {
                auto x1_it = child->attrs.find("x1");
                if (x1_it != child->attrs.end()) grad.x1 = parse_gradient_coord(x1_it->second, 0);
                auto y1_it = child->attrs.find("y1");
                if (y1_it != child->attrs.end()) grad.y1 = parse_gradient_coord(y1_it->second, 0);
                auto x2_it = child->attrs.find("x2");
                if (x2_it != child->attrs.end()) grad.x2 = parse_gradient_coord(x2_it->second, 1);
                auto y2_it = child->attrs.find("y2");
                if (y2_it != child->attrs.end()) grad.y2 = parse_gradient_coord(y2_it->second, 0);
            } else {
                auto cx_it = child->attrs.find("cx");
                if (cx_it != child->attrs.end()) grad.cx = parse_gradient_coord(cx_it->second, 0);
                auto cy_it = child->attrs.find("cy");
                if (cy_it != child->attrs.end()) grad.cy = parse_gradient_coord(cy_it->second, 0);
                auto r_it = child->attrs.find("r");
                if (r_it != child->attrs.end()) grad.r = parse_gradient_coord(r_it->second, 0);
                auto fx_it = child->attrs.find("fx");
                if (fx_it != child->attrs.end()) grad.fx = parse_gradient_coord(fx_it->second, 0);
                auto fy_it = child->attrs.find("fy");
                if (fy_it != child->attrs.end()) grad.fy = parse_gradient_coord(fy_it->second, 0);
            }
            for (auto& c : child->children) {
                if (c->type != HTMLNodeType::ELEMENT) continue;
                std::string ctag = str_tolower(c->tag);
                if (ctag == "stop") {
                    GradientStop stop;
                    stop.offset = 0;
                    stop.r = 0; stop.g = 0; stop.b = 0;
                    stop.opacity = 1.0;
                    auto off_it = c->attrs.find("offset");
                    if (off_it != c->attrs.end()) {
                        std::string off = off_it->second;
                        if (!off.empty() && off.back() == '%') {
                            try { stop.offset = std::stod(off.substr(0, off.size()-1)) / 100.0; } catch(...) {}
                        } else {
                            try { stop.offset = std::stod(off); } catch(...) {}
                        }
                    }
                    auto style_it = c->attrs.find("style");
                    std::string color_str;
                    std::string opacity_str;
                    if (style_it != c->attrs.end()) {
                        std::map<std::string, std::string> style_props;
                        parse_svg_style(style_it->second, style_props);
                        auto sc_it = style_props.find("stop-color");
                        if (sc_it != style_props.end()) color_str = sc_it->second;
                        auto so_it = style_props.find("stop-opacity");
                        if (so_it != style_props.end()) opacity_str = so_it->second;
                    }
                    auto sc_it = c->attrs.find("stop-color");
                    if (sc_it != c->attrs.end()) color_str = sc_it->second;
                    auto so_it = c->attrs.find("stop-opacity");
                    if (so_it != c->attrs.end()) opacity_str = so_it->second;
                    if (!color_str.empty()) {
                        int r=0,g=0,b=0; bool valid=false;
                        parse_color(color_str, r, g, b, valid);
                        if (valid) { stop.r = r; stop.g = g; stop.b = b; }
                    }
                    if (!opacity_str.empty()) {
                        try { stop.opacity = std::stod(opacity_str); } catch(...) {}
                    }
                    grad.stops.push_back(stop);
                }
            }
            if (grad.stops.empty() && !href_id.empty()) {
                auto it = gradients.find(href_id);
                if (it != gradients.end()) {
                    grad.stops = it->second.stops;
                }
            }
            if (!grad.stops.empty()) {
                gradients[grad.id] = grad;
            }
        }
    }
}

bool SVGRenderer::get_gradient_color(const std::string& id, int& r, int& g, int& b) {
    auto it = gradients.find(str_tolower(id));
    if (it == gradients.end() || it->second.stops.empty()) return false;
    const auto& stop = it->second.stops[0];
    r = stop.r;
    g = stop.g;
    b = stop.b;
    return true;
}

bool SVGRenderer::is_gradient_fill(const std::map<std::string, std::string>& attrs, std::string& grad_id) {
    std::map<std::string, std::string> effective_attrs = attrs;
    auto style_it = attrs.find("style");
    if (style_it != attrs.end()) {
        std::map<std::string, std::string> style_props;
        parse_svg_style(style_it->second, style_props);
        for (auto& p : style_props) {
            if (effective_attrs.find(p.first) == effective_attrs.end()) {
                effective_attrs[p.first] = p.second;
            }
        }
    }
    auto fill_it = effective_attrs.find("fill");
    if (fill_it == effective_attrs.end()) return false;
    std::string fill_val = str_tolower(str_trim(fill_it->second));
    if (fill_val.substr(0, 4) != "url(") return false;
    size_t lp = fill_val.find('(');
    size_t rp = fill_val.find(')', lp);
    if (lp == std::string::npos || rp == std::string::npos) return false;
    std::string ref = fill_val.substr(lp + 1, rp - lp - 1);
    if (!ref.empty() && ref.front() == '#') ref = ref.substr(1);
    grad_id = ref;
    return gradients.find(str_tolower(ref)) != gradients.end();
}

void SVGRenderer::fill_gradient(const std::string& grad_id, double bx, double by, double bw, double bh) {
    std::string key = str_tolower(grad_id) + "|" + std::to_string(bx) + "|" + std::to_string(by) + "|" + std::to_string(bw) + "|" + std::to_string(bh);
    auto cache_it = gradient_shading_ids.find(key);
    int shading_id = -1;
    if (cache_it != gradient_shading_ids.end()) {
        shading_id = cache_it->second;
    } else {
        auto it = gradients.find(str_tolower(grad_id));
        if (it == gradients.end() || it->second.stops.empty()) return;
        const GradientDef& grad = it->second;

        std::vector<pz::GradientStop> pdf_stops;
        for (auto& s : grad.stops) {
            pz::GradientStop gs;
            gs.offset = s.offset;
            // PDF shading doesn't support per-stop opacity.
            // Blend color with white (assumed background) based on opacity
            // so stop-color:#ff6600;stop-opacity:0 becomes white (255,255,255)
            double alpha = s.opacity;
            gs.r = s.r * alpha + 255.0 * (1.0 - alpha);
            gs.g = s.g * alpha + 255.0 * (1.0 - alpha);
            gs.b = s.b * alpha + 255.0 * (1.0 - alpha);
            gs.opacity = 1.0;
            pdf_stops.push_back(gs);
        }

        if (grad.is_linear) {
            // Linear (axial) gradient
            double svg_x0, svg_y0, svg_x1, svg_y1;
            if (grad.gradient_units == "userspaceonuse") {
                svg_x0 = grad.x1;
                svg_y0 = grad.y1;
                svg_x1 = grad.x2;
                svg_y1 = grad.y2;
            } else {
                svg_x0 = bx + grad.x1 * bw;
                svg_y0 = by + grad.y1 * bh;
                svg_x1 = bx + grad.x2 * bw;
                svg_y1 = by + grad.y2 * bh;
            }
            shading_id = pdf->AddAxialShading(svg_x0, svg_y0, svg_x1, svg_y1, pdf_stops);
        } else {
            // Radial gradient
            double cx, cy, r, fx, fy;
            if (grad.gradient_units == "userspaceonuse") {
                cx = grad.cx;
                cy = grad.cy;
                r = grad.r;
                fx = grad.fx;
                fy = grad.fy;
            } else {
                cx = bx + grad.cx * bw;
                cy = by + grad.cy * bh;
                r = grad.r * std::max(bw, bh);
                fx = bx + grad.fx * bw;
                fy = by + grad.fy * bh;
            }
            // fx/fy default to cx/cy if not set (fx=0,fy=0 means they weren't set)
            if (fx == 0 && fy == 0 && grad.fx == 0 && grad.fy == 0) {
                fx = cx;
                fy = cy;
            }
            // PDF Type 3 radial shading: inner circle (fx,fy,r0) -> outer circle (cx,cy,r)
            shading_id = pdf->AddRadialShading(fx, fy, 0, cx, cy, r, pdf_stops);
        }
        gradient_shading_ids[key] = shading_id;
    }

    if (shading_id >= 0) {
        pdf->ShadeFill(shading_id);
    }
}

void SVGRenderer::render_element(const std::shared_ptr<HTMLNode>& elem) {
    std::string tag = str_tolower(elem->tag);

    if (tag == "g") {
        // Group: save state, apply transform, render children, restore
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        render_children(elem->children);
        pdf->_out("Q");
    } else if (tag == "rect") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_rect(elem->attrs);
        pdf->_out("Q");
    } else if (tag == "circle") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_circle(elem->attrs);
        pdf->_out("Q");
    } else if (tag == "ellipse") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_ellipse(elem->attrs);
        pdf->_out("Q");
    } else if (tag == "line") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_line(elem->attrs);
        pdf->_out("Q");
    } else if (tag == "polygon") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_polygon(elem->attrs, true);
        pdf->_out("Q");
    } else if (tag == "polyline") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_polygon(elem->attrs, false);
        pdf->_out("Q");
    } else if (tag == "path") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_path(elem->attrs);
        pdf->_out("Q");
    } else if (tag == "text") {
        pdf->_out("q");
        apply_element_transform(elem->attrs);
        draw_svg_text(elem);
        pdf->_out("Q");
    }
    // Skip: title, desc, text, etc.
}

void SVGRenderer::apply_element_transform(const std::map<std::string, std::string>& attrs) {
    auto it = attrs.find("transform");
    if (it == attrs.end()) return;

    std::string transform = str_trim(it->second);
    std::string lower = str_tolower(transform);

    // Parse transform functions: matrix(a,b,c,d,e,f), translate(tx,ty), scale(sx,sy), rotate(angle,cx,cy)
    size_t pos = 0;
    while (pos < lower.size()) {
        size_t paren_open = lower.find('(', pos);
        if (paren_open == std::string::npos) break;
        std::string func = str_trim(lower.substr(pos, paren_open - pos));
        size_t paren_close = lower.find(')', paren_open);
        if (paren_close == std::string::npos) break;
        std::string args_str = transform.substr(paren_open + 1, paren_close - paren_open - 1);
        std::vector<double> args = parse_num_list(args_str);

        double a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;

        if (func == "matrix" && args.size() >= 6) {
            a = args[0]; b = args[1]; c = args[2]; d = args[3]; e = args[4]; f = args[5];
        } else if (func == "translate") {
            e = args.size() >= 1 ? args[0] : 0;
            f = args.size() >= 2 ? args[1] : 0;
        } else if (func == "scale") {
            a = args.size() >= 1 ? args[0] : 1;
            d = args.size() >= 2 ? args[1] : a;
        } else if (func == "rotate") {
            double angle = (args.size() >= 1 ? args[0] : 0) * M_PI / 180.0;
            double cos_a = std::cos(angle);
            double sin_a = std::sin(angle);
            a = cos_a; b = sin_a; c = -sin_a; d = cos_a;
            if (args.size() >= 3) {
                double cx = args[1], cy = args[2];
                e = cx - cx * cos_a + cy * sin_a;
                f = cy - cx * sin_a - cy * cos_a;
            }
        } else if (func == "skewx") {
            double angle = (args.size() >= 1 ? args[0] : 0) * M_PI / 180.0;
            c = std::tan(angle);
        } else if (func == "skewy") {
            double angle = (args.size() >= 1 ? args[0] : 0) * M_PI / 180.0;
            b = std::tan(angle);
        }

        pdf->_out(fmt(a) + " " + fmt(b) + " " + fmt(c) + " " + fmt(d) + " " + fmt(e) + " " + fmt(f) + " cm");

        pos = paren_close + 1;
        while (pos < lower.size() && (lower[pos] == ' ' || lower[pos] == ',' || lower[pos] == ';'))
            pos++;
    }
}

void SVGRenderer::fill_and_stroke(const std::map<std::string, std::string>& attrs, bool has_path) {
    bool do_fill = true;
    bool do_stroke = false;

    std::map<std::string, std::string> effective_attrs = attrs;
    auto style_it = attrs.find("style");
    if (style_it != attrs.end()) {
        std::map<std::string, std::string> style_props;
        parse_svg_style(style_it->second, style_props);
        for (auto& p : style_props) {
            if (effective_attrs.find(p.first) == effective_attrs.end()) {
                effective_attrs[p.first] = p.second;
            }
        }
    }

    auto fill_it = effective_attrs.find("fill");
    if (fill_it != effective_attrs.end()) {
        std::string fill_val = str_tolower(str_trim(fill_it->second));
        if (fill_val == "none" || fill_val == "null") {
            do_fill = false;
        } else if (fill_val.substr(0, 4) == "url(") {
            // Gradient fill is handled by the caller (draw_rect, draw_path, etc.)
            // via clipping + shading. Here we skip the fill to avoid overwriting
            // the gradient with a solid color.
            do_fill = false;
        } else {
            int r, g, b;
            bool valid;
            parse_color(fill_it->second, r, g, b, valid);
            if (valid) {
                if (r == g && g == b)
                    pdf->_out(fmt(r / 255.0, 3) + " g");
                else
                    pdf->_out(fmt(r / 255.0, 3) + " " + fmt(g / 255.0, 3) + " " + fmt(b / 255.0, 3) + " rg");
            }
        }
    } else {
        pdf->_out("0.000 g");
    }

    auto stroke_it = effective_attrs.find("stroke");
    if (stroke_it != effective_attrs.end()) {
        std::string stroke_val = str_tolower(str_trim(stroke_it->second));
        if (stroke_val != "none" && stroke_val != "null") {
            do_stroke = true;
            if (stroke_val.substr(0, 4) == "url(") {
                size_t lp = stroke_val.find('(');
                size_t rp = stroke_val.find(')', lp);
                if (lp != std::string::npos && rp != std::string::npos) {
                    std::string ref = stroke_val.substr(lp + 1, rp - lp - 1);
                    if (!ref.empty() && ref.front() == '#') ref = ref.substr(1);
                    int r=0,g=0,b=0;
                    if (get_gradient_color(ref, r, g, b)) {
                        if (r == g && g == b)
                            pdf->_out(fmt(r / 255.0, 3) + " G");
                        else
                            pdf->_out(fmt(r / 255.0, 3) + " " + fmt(g / 255.0, 3) + " " + fmt(b / 255.0, 3) + " RG");
                    } else {
                        do_stroke = false;
                    }
                }
            } else {
                int r, g, b;
                bool valid;
                parse_color(stroke_it->second, r, g, b, valid);
                if (valid) {
                    if (r == g && g == b)
                        pdf->_out(fmt(r / 255.0, 3) + " G");
                    else
                        pdf->_out(fmt(r / 255.0, 3) + " " + fmt(g / 255.0, 3) + " " + fmt(b / 255.0, 3) + " RG");
                }
            }
        }
    }

    double stroke_width = 1.0;
    auto sw_it = effective_attrs.find("stroke-width");
    if (sw_it != effective_attrs.end())
        stroke_width = parse_num(sw_it->second, 1.0);

    if (do_stroke)
        pdf->_out(fmt(stroke_width) + " w");

    if (has_path) {
        if (do_fill && do_stroke)
            pdf->_out("B");
        else if (do_fill)
            pdf->_out("f");
        else if (do_stroke)
            pdf->_out("S");
    }
}

void SVGRenderer::draw_rect(const std::map<std::string, std::string>& attrs) {
    double x = 0, y = 0, w = 0, h = 0;
    auto it = attrs.find("x"); if (it != attrs.end()) x = parse_num(it->second);
    it = attrs.find("y"); if (it != attrs.end()) y = parse_num(it->second);
    it = attrs.find("width"); if (it != attrs.end()) w = parse_num(it->second);
    it = attrs.find("height"); if (it != attrs.end()) h = parse_num(it->second);

    if (w <= 0 || h <= 0) return;

    std::string grad_id;
    bool has_grad = is_gradient_fill(attrs, grad_id);

    if (has_grad) {
        pdf->_out("q");
        pdf->_out(fmt(x) + " " + fmt(y) + " " + fmt(w) + " " + fmt(h) + " re");
        pdf->_out("W n");
        fill_gradient(grad_id, x, y, w, h);
        pdf->_out("Q");
        bool do_stroke = false;
        std::map<std::string, std::string> effective_attrs = attrs;
        auto style_it = attrs.find("style");
        if (style_it != attrs.end()) {
            std::map<std::string, std::string> style_props;
            parse_svg_style(style_it->second, style_props);
            for (auto& p : style_props) {
                if (effective_attrs.find(p.first) == effective_attrs.end()) {
                    effective_attrs[p.first] = p.second;
                }
            }
        }
        auto stroke_it = effective_attrs.find("stroke");
        if (stroke_it != effective_attrs.end()) {
            std::string stroke_val = str_tolower(str_trim(stroke_it->second));
            if (stroke_val != "none" && stroke_val != "null") {
                do_stroke = true;
            }
        }
        if (do_stroke) {
            pdf->_out(fmt(x) + " " + fmt(y) + " " + fmt(w) + " " + fmt(h) + " re");
            fill_and_stroke(attrs, true);
        }
    } else {
        pdf->_out(fmt(x) + " " + fmt(y) + " " + fmt(w) + " " + fmt(h) + " re");
        fill_and_stroke(attrs, true);
    }
}

void SVGRenderer::draw_circle(const std::map<std::string, std::string>& attrs) {
    double cx = 0, cy = 0, r = 0;
    auto it = attrs.find("cx"); if (it != attrs.end()) cx = parse_num(it->second);
    it = attrs.find("cy"); if (it != attrs.end()) cy = parse_num(it->second);
    it = attrs.find("r"); if (it != attrs.end()) r = parse_num(it->second);

    if (r <= 0) return;

    // Copy original attrs and add rx/ry for draw_ellipse
    auto ellipse_attrs = attrs;
    ellipse_attrs["cx"] = std::to_string(cx);
    ellipse_attrs["cy"] = std::to_string(cy);
    ellipse_attrs["rx"] = std::to_string(r);
    ellipse_attrs["ry"] = std::to_string(r);
    draw_ellipse(ellipse_attrs);
}

void SVGRenderer::draw_ellipse(const std::map<std::string, std::string>& attrs) {
    double cx = 0, cy = 0, rx = 0, ry = 0;
    auto it = attrs.find("cx"); if (it != attrs.end()) cx = parse_num(it->second);
    it = attrs.find("cy"); if (it != attrs.end()) cy = parse_num(it->second);
    it = attrs.find("rx"); if (it != attrs.end()) rx = parse_num(it->second);
    it = attrs.find("ry"); if (it != attrs.end()) ry = parse_num(it->second);

    if (rx <= 0 || ry <= 0) return;

    double k = 0.5522847498;
    double kx = rx * k;
    double ky = ry * k;

    std::string path;
    path += fmt(cx + rx) + " " + fmt(cy) + " m ";
    path += fmt(cx + rx) + " " + fmt(cy + ky) + " " +
            fmt(cx + kx) + " " + fmt(cy + ry) + " " +
            fmt(cx) + " " + fmt(cy + ry) + " c ";
    path += fmt(cx - kx) + " " + fmt(cy + ry) + " " +
            fmt(cx - rx) + " " + fmt(cy + ky) + " " +
            fmt(cx - rx) + " " + fmt(cy) + " c ";
    path += fmt(cx - rx) + " " + fmt(cy - ky) + " " +
            fmt(cx - kx) + " " + fmt(cy - ry) + " " +
            fmt(cx) + " " + fmt(cy - ry) + " c ";
    path += fmt(cx + kx) + " " + fmt(cy - ry) + " " +
            fmt(cx + rx) + " " + fmt(cy - ky) + " " +
            fmt(cx + rx) + " " + fmt(cy) + " c ";
    path += "h";

    std::string grad_id;
    bool has_grad = is_gradient_fill(attrs, grad_id);

    if (has_grad) {
        double bx = cx - rx;
        double by = cy - ry;
        double bw = 2 * rx;
        double bh = 2 * ry;
        pdf->_out("q");
        pdf->_out(path);
        pdf->_out("W n");
        fill_gradient(grad_id, bx, by, bw, bh);
        pdf->_out("Q");
        bool do_stroke = false;
        std::map<std::string, std::string> effective_attrs = attrs;
        auto style_it = attrs.find("style");
        if (style_it != attrs.end()) {
            std::map<std::string, std::string> style_props;
            parse_svg_style(style_it->second, style_props);
            for (auto& p : style_props) {
                if (effective_attrs.find(p.first) == effective_attrs.end()) {
                    effective_attrs[p.first] = p.second;
                }
            }
        }
        auto stroke_it = effective_attrs.find("stroke");
        if (stroke_it != effective_attrs.end()) {
            std::string stroke_val = str_tolower(str_trim(stroke_it->second));
            if (stroke_val != "none" && stroke_val != "null") {
                do_stroke = true;
            }
        }
        if (do_stroke) {
            pdf->_out(path);
            fill_and_stroke(attrs, true);
        }
    } else {
        pdf->_out(path);
        fill_and_stroke(attrs, true);
    }
}

void SVGRenderer::draw_line(const std::map<std::string, std::string>& attrs) {
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    auto it = attrs.find("x1"); if (it != attrs.end()) x1 = parse_num(it->second);
    it = attrs.find("y1"); if (it != attrs.end()) y1 = parse_num(it->second);
    it = attrs.find("x2"); if (it != attrs.end()) x2 = parse_num(it->second);
    it = attrs.find("y2"); if (it != attrs.end()) y2 = parse_num(it->second);

    pdf->_out(fmt(x1) + " " + fmt(y1) + " m " + fmt(x2) + " " + fmt(y2) + " l");

    // Lines only support stroke
    bool do_stroke = true;
    auto stroke_it = attrs.find("stroke");
    if (stroke_it != attrs.end()) {
        std::string sv = str_tolower(str_trim(stroke_it->second));
        if (sv == "none" || sv == "null") do_stroke = false;
        else {
            int r, g, b; bool valid;
            parse_color(stroke_it->second, r, g, b, valid);
            if (valid) {
                if (r == g && g == b)
                    pdf->_out(fmt(r / 255.0, 3) + " G");
                else
                    pdf->_out(fmt(r / 255.0, 3) + " " + fmt(g / 255.0, 3) + " " + fmt(b / 255.0, 3) + " RG");
            }
        }
    } else {
        do_stroke = false; // Default stroke for line is none in SVG... actually default is none? No, SVG default stroke is none.
        // Actually for <line>, if no stroke is specified, nothing is drawn
    }

    double sw = 1.0;
    auto sw_it = attrs.find("stroke-width");
    if (sw_it != attrs.end()) sw = parse_num(sw_it->second, 1.0);
    if (do_stroke) pdf->_out(fmt(sw) + " w");

    if (do_stroke)
        pdf->_out("S");
}

void SVGRenderer::draw_polygon(const std::map<std::string, std::string>& attrs, bool closed) {
    auto it = attrs.find("points");
    if (it == attrs.end()) return;

    std::vector<double> pts = parse_num_list(it->second);
    if (pts.size() < 4) return;

    std::string path;
    path += fmt(pts[0]) + " " + fmt(pts[1]) + " m ";
    double min_x = pts[0], max_x = pts[0], min_y = pts[1], max_y = pts[1];
    for (size_t i = 2; i + 1 < pts.size(); i += 2) {
        path += fmt(pts[i]) + " " + fmt(pts[i + 1]) + " l ";
        if (pts[i] < min_x) min_x = pts[i];
        if (pts[i] > max_x) max_x = pts[i];
        if (pts[i+1] < min_y) min_y = pts[i+1];
        if (pts[i+1] > max_y) max_y = pts[i+1];
    }
    if (closed)
        path += "h";

    std::string grad_id;
    bool has_grad = is_gradient_fill(attrs, grad_id);
    if (has_grad) {
        double bw = max_x - min_x;
        double bh = max_y - min_y;
        if (bw <= 0) bw = 1;
        if (bh <= 0) bh = 1;
        pdf->_out("q");
        pdf->_out(path);
        pdf->_out("W n");
        fill_gradient(grad_id, min_x, min_y, bw, bh);
        pdf->_out("Q");

        bool do_stroke = false;
        std::map<std::string, std::string> effective_attrs = attrs;
        auto style_it = attrs.find("style");
        if (style_it != attrs.end()) {
            std::map<std::string, std::string> style_props;
            parse_svg_style(style_it->second, style_props);
            for (auto& p : style_props) {
                if (effective_attrs.find(p.first) == effective_attrs.end()) {
                    effective_attrs[p.first] = p.second;
                }
            }
        }
        auto stroke_it = effective_attrs.find("stroke");
        if (stroke_it != effective_attrs.end()) {
            std::string stroke_val = str_tolower(str_trim(stroke_it->second));
            if (stroke_val != "none" && stroke_val != "null") {
                do_stroke = true;
            }
        }
        if (do_stroke) {
            pdf->_out(path);
            fill_and_stroke(attrs, true);
        }
    } else {
        pdf->_out(path);
        fill_and_stroke(attrs, true);
    }
}

void SVGRenderer::draw_path(const std::map<std::string, std::string>& attrs) {
    auto it = attrs.find("d");
    if (it == attrs.end()) return;

    std::string d = it->second;
    std::string path;
    double cur_x = 0, cur_y = 0;
    double start_x = 0, start_y = 0;
    double prev_ctrl_x = 0, prev_ctrl_y = 0;
    bool prev_cubic = false, prev_quad = false;

    size_t pos = 0;
    while (pos < d.size()) {
        while (pos < d.size() && std::isspace((unsigned char)d[pos])) pos++;
        if (pos >= d.size()) break;

        char cmd = d[pos];
        bool is_upper = std::isupper((unsigned char)cmd);
        pos++;

        // Read numbers
        std::vector<double> nums;
        while (pos < d.size()) {
            while (pos < d.size() && (std::isspace((unsigned char)d[pos]) || d[pos] == ',')) pos++;
            if (pos >= d.size()) break;
            if (std::isalpha((unsigned char)d[pos])) break;
            std::string num_str;
            while (pos < d.size() && (std::isdigit((unsigned char)d[pos]) || d[pos] == '.' || d[pos] == '-' || d[pos] == '+' || d[pos] == 'e' || d[pos] == 'E')) {
                num_str += d[pos];
                pos++;
            }
            if (!num_str.empty()) {
                try { nums.push_back(std::stod(num_str)); } catch (...) {}
            } else {
                // Unrecognized character: avoid infinite loop, let outer loop handle it
                break;
            }
        }

        char lower_cmd = std::tolower((unsigned char)cmd);

        switch (lower_cmd) {
            case 'm': { // moveto
                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                    double x = nums[i], y = nums[i + 1];
                    if (!is_upper) { x += cur_x; y += cur_y; }
                    cur_x = x; cur_y = y;
                    if (i == 0) { start_x = x; start_y = y; }
                    path += fmt(x) + " " + fmt(y) + (i == 0 ? " m " : " l ");
                }
                prev_cubic = prev_quad = false;
                break;
            }
            case 'l': { // lineto
                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                    double x = nums[i], y = nums[i + 1];
                    if (!is_upper) { x += cur_x; y += cur_y; }
                    cur_x = x; cur_y = y;
                    path += fmt(x) + " " + fmt(y) + " l ";
                }
                prev_cubic = prev_quad = false;
                break;
            }
            case 'h': { // horizontal lineto
                for (size_t i = 0; i < nums.size(); i++) {
                    double x = nums[i];
                    if (!is_upper) x += cur_x;
                    cur_x = x;
                    path += fmt(x) + " " + fmt(cur_y) + " l ";
                }
                prev_cubic = prev_quad = false;
                break;
            }
            case 'v': { // vertical lineto
                for (size_t i = 0; i < nums.size(); i++) {
                    double y = nums[i];
                    if (!is_upper) y += cur_y;
                    cur_y = y;
                    path += fmt(cur_x) + " " + fmt(y) + " l ";
                }
                prev_cubic = prev_quad = false;
                break;
            }
            case 'c': { // cubic bezier
                for (size_t i = 0; i + 5 < nums.size(); i += 6) {
                    double x1 = nums[i], y1 = nums[i+1], x2 = nums[i+2], y2 = nums[i+3], x = nums[i+4], y = nums[i+5];
                    if (!is_upper) {
                        x1 += cur_x; y1 += cur_y; x2 += cur_x; y2 += cur_y; x += cur_x; y += cur_y;
                    }
                    path += fmt(x1) + " " + fmt(y1) + " " + fmt(x2) + " " + fmt(y2) + " " + fmt(x) + " " + fmt(y) + " c ";
                    prev_ctrl_x = x2; prev_ctrl_y = y2;
                    cur_x = x; cur_y = y;
                }
                prev_cubic = true; prev_quad = false;
                break;
            }
            case 's': { // smooth cubic
                for (size_t i = 0; i + 3 < nums.size(); i += 4) {
                    double x2 = nums[i], y2 = nums[i+1], x = nums[i+2], y = nums[i+3];
                    if (!is_upper) { x2 += cur_x; y2 += cur_y; x += cur_x; y += cur_y; }
                    double x1, y1;
                    if (prev_cubic) {
                        x1 = 2 * cur_x - prev_ctrl_x;
                        y1 = 2 * cur_y - prev_ctrl_y;
                    } else {
                        x1 = cur_x; y1 = cur_y;
                    }
                    path += fmt(x1) + " " + fmt(y1) + " " + fmt(x2) + " " + fmt(y2) + " " + fmt(x) + " " + fmt(y) + " c ";
                    prev_ctrl_x = x2; prev_ctrl_y = y2;
                    cur_x = x; cur_y = y;
                }
                prev_cubic = true; prev_quad = false;
                break;
            }
            case 'q': { // quadratic bezier
                for (size_t i = 0; i + 3 < nums.size(); i += 4) {
                    double x1 = nums[i], y1 = nums[i+1], x = nums[i+2], y = nums[i+3];
                    if (!is_upper) { x1 += cur_x; y1 += cur_y; x += cur_x; y += cur_y; }
                    // Convert to cubic
                    double cx1 = cur_x + 2.0/3.0 * (x1 - cur_x);
                    double cy1 = cur_y + 2.0/3.0 * (y1 - cur_y);
                    double cx2 = x + 2.0/3.0 * (x1 - x);
                    double cy2 = y + 2.0/3.0 * (y1 - y);
                    path += fmt(cx1) + " " + fmt(cy1) + " " + fmt(cx2) + " " + fmt(cy2) + " " + fmt(x) + " " + fmt(y) + " c ";
                    prev_ctrl_x = x1; prev_ctrl_y = y1;
                    cur_x = x; cur_y = y;
                }
                prev_quad = true; prev_cubic = false;
                break;
            }
            case 't': { // smooth quadratic
                for (size_t i = 0; i + 1 < nums.size(); i += 2) {
                    double x = nums[i], y = nums[i+1];
                    if (!is_upper) { x += cur_x; y += cur_y; }
                    double x1, y1;
                    if (prev_quad) {
                        x1 = 2 * cur_x - prev_ctrl_x;
                        y1 = 2 * cur_y - prev_ctrl_y;
                    } else {
                        x1 = cur_x; y1 = cur_y;
                    }
                    double cx1 = cur_x + 2.0/3.0 * (x1 - cur_x);
                    double cy1 = cur_y + 2.0/3.0 * (y1 - cur_y);
                    double cx2 = x + 2.0/3.0 * (x1 - x);
                    double cy2 = y + 2.0/3.0 * (y1 - y);
                    path += fmt(cx1) + " " + fmt(cy1) + " " + fmt(cx2) + " " + fmt(cy2) + " " + fmt(x) + " " + fmt(y) + " c ";
                    prev_ctrl_x = x1; prev_ctrl_y = y1;
                    cur_x = x; cur_y = y;
                }
                prev_quad = true; prev_cubic = false;
                break;
            }
            case 'a': { // arc - convert to cubic bezier curves
                for (size_t i = 0; i + 6 < nums.size(); i += 7) {
                    double rx = nums[i], ry = nums[i+1];
                    double x_axis_rot = nums[i+2];
                    int large_arc_flag = (int)nums[i+3];
                    int sweep_flag = (int)nums[i+4];
                    double x = nums[i+5], y = nums[i+6];
                    if (!is_upper) { x += cur_x; y += cur_y; }
                    
                    double x1 = cur_x, y1 = cur_y;
                    double x2 = x, y2 = y;
                    
                    if (rx == 0 || ry == 0) {
                        path += fmt(x2) + " " + fmt(y2) + " l ";
                        cur_x = x2; cur_y = y2;
                        prev_cubic = prev_quad = false;
                        continue;
                    }
                    
                    double phi = x_axis_rot * M_PI / 180.0;
                    double cos_phi = std::cos(phi);
                    double sin_phi = std::sin(phi);
                    
                    double dx = (x1 - x2) / 2.0;
                    double dy = (y1 - y2) / 2.0;
                    double x1_prime = cos_phi * dx + sin_phi * dy;
                    double y1_prime = -sin_phi * dx + cos_phi * dy;
                    
                    rx = std::fabs(rx);
                    ry = std::fabs(ry);
                    
                    double scale = (x1_prime * x1_prime) / (rx * rx) + (y1_prime * y1_prime) / (ry * ry);
                    if (scale > 1) {
                        rx *= std::sqrt(scale);
                        ry *= std::sqrt(scale);
                    }
                    
                    double rx_sq = rx * rx;
                    double ry_sq = ry * ry;
                    double x1_prime_sq = x1_prime * x1_prime;
                    double y1_prime_sq = y1_prime * y1_prime;

                    // Center parameterization (W3C SVG impl. notes F.6.5):
                    // coef = sqrt( (rx^2 ry^2 - rx^2 y1'^2 - ry^2 x1'^2)
                    //              / (rx^2 y1'^2 + ry^2 x1'^2) )
                    double num = rx_sq * ry_sq - rx_sq * y1_prime_sq - ry_sq * x1_prime_sq;
                    if (num < 0) num = 0;
                    double den = rx_sq * y1_prime_sq + ry_sq * x1_prime_sq;
                    double coef = (den > 0) ? std::sqrt(num / den) : 0;
                    if (large_arc_flag == sweep_flag) coef = -coef;

                    double cx_prime = coef * (rx * y1_prime / ry);
                    double cy_prime = coef * (-ry * x1_prime / rx);

                    double cx = cos_phi * cx_prime - sin_phi * cy_prime + (x1 + x2) / 2.0;
                    double cy = sin_phi * cx_prime + cos_phi * cy_prime + (y1 + y2) / 2.0;

                    // Signed angle between two vectors (u -> v)
                    auto vec_angle = [](double ux, double uy, double vx, double vy) {
                        double len = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
                        if (len == 0) return 0.0;
                        double c = (ux * vx + uy * vy) / len;
                        if (c < -1) c = -1;
                        if (c > 1) c = 1;
                        double a = std::acos(c);
                        if (ux * vy - uy * vx < 0) a = -a;
                        return a;
                    };

                    double ux = (x1_prime - cx_prime) / rx;
                    double uy = (y1_prime - cy_prime) / ry;
                    double vx = (-x1_prime - cx_prime) / rx;
                    double vy = (-y1_prime - cy_prime) / ry;

                    double theta1 = vec_angle(1, 0, ux, uy);
                    double delta_theta = vec_angle(ux, uy, vx, vy);
                    if (sweep_flag == 0 && delta_theta > 0) delta_theta -= 2 * M_PI;
                    else if (sweep_flag == 1 && delta_theta < 0) delta_theta += 2 * M_PI;

                    int num_segments = (int)std::ceil(std::fabs(delta_theta) / (M_PI / 2));
                    if (num_segments == 0) num_segments = 1;
                    double seg = delta_theta / num_segments;
                    double alpha = 4.0 / 3.0 * std::tan(seg / 4.0);

                    // Map a point in unrotated ellipse-local coords to page coords
                    auto map_x = [&](double ex, double ey) { return cx + cos_phi * ex - sin_phi * ey; };
                    auto map_y = [&](double ex, double ey) { return cy + sin_phi * ex + cos_phi * ey; };

                    double t_ang = theta1;
                    for (int j = 0; j < num_segments; j++) {
                        double t_next = t_ang + seg;
                        double cos1 = std::cos(t_ang), sin1 = std::sin(t_ang);
                        double cos2 = std::cos(t_next), sin2 = std::sin(t_next);

                        // Ellipse-local endpoints and tangents
                        double e1x = rx * cos1, e1y = ry * sin1;
                        double e2x = rx * cos2, e2y = ry * sin2;
                        double d1x = -rx * sin1, d1y = ry * cos1;
                        double d2x = -rx * sin2, d2y = ry * cos2;

                        double cp1x = map_x(e1x + alpha * d1x, e1y + alpha * d1y);
                        double cp1y = map_y(e1x + alpha * d1x, e1y + alpha * d1y);
                        double cp2x = map_x(e2x - alpha * d2x, e2y - alpha * d2y);
                        double cp2y = map_y(e2x - alpha * d2x, e2y - alpha * d2y);
                        double x3 = map_x(e2x, e2y);
                        double y3 = map_y(e2x, e2y);

                        path += fmt(cp1x) + " " + fmt(cp1y) + " " +
                                fmt(cp2x) + " " + fmt(cp2y) + " " +
                                fmt(x3) + " " + fmt(y3) + " c ";

                        t_ang = t_next;
                    }

                    cur_x = x2;
                    cur_y = y2;
                    prev_cubic = true;
                    prev_quad = false;
                }
                break;
            }
            case 'z': { // close path
                path += "h ";
                cur_x = start_x; cur_y = start_y;
                prev_cubic = prev_quad = false;
                break;
            }
        }
    }

    if (!path.empty()) {
        std::string grad_id;
        bool has_grad = is_gradient_fill(attrs, grad_id);
        if (has_grad) {
            double min_x = cur_x, min_y = cur_y, max_x = cur_x, max_y = cur_y;
            bool started = false;
            size_t ppos = 0;
            while (ppos < path.size()) {
                while (ppos < path.size() && std::isspace((unsigned char)path[ppos])) ppos++;
                if (ppos >= path.size()) break;
                size_t space1 = path.find(' ', ppos);
                if (space1 == std::string::npos) break;
                std::string x_str = path.substr(ppos, space1 - ppos);
                ppos = space1 + 1;
                while (ppos < path.size() && std::isspace((unsigned char)path[ppos])) ppos++;
                if (ppos >= path.size()) break;
                size_t space2 = path.find(' ', ppos);
                if (space2 == std::string::npos) break;
                std::string y_str = path.substr(ppos, space2 - ppos);
                ppos = space2 + 1;
                while (ppos < path.size() && std::isspace((unsigned char)path[ppos])) ppos++;
                if (ppos >= path.size()) break;
                char cmd = path[ppos];
                ppos++;
                double x = 0, y = 0;
                try { x = std::stod(x_str); y = std::stod(y_str); } catch(...) {}
                if (!started || cmd == 'm') {
                    if (!started) {
                        min_x = max_x = x;
                        min_y = max_y = y;
                        started = true;
                    }
                }
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
            double bx = min_x;
            double by = min_y;
            double bw = max_x - min_x;
            double bh = max_y - min_y;
            if (bw <= 0) bw = 1;
            if (bh <= 0) bh = 1;

            pdf->_out("q");
            pdf->_out(path);
            pdf->_out("W n");
            fill_gradient(grad_id, bx, by, bw, bh);
            pdf->_out("Q");

            bool do_stroke = false;
            std::map<std::string, std::string> effective_attrs = attrs;
            auto style_it = attrs.find("style");
            if (style_it != attrs.end()) {
                std::map<std::string, std::string> style_props;
                parse_svg_style(style_it->second, style_props);
                for (auto& p : style_props) {
                    if (effective_attrs.find(p.first) == effective_attrs.end()) {
                        effective_attrs[p.first] = p.second;
                    }
                }
            }
            auto stroke_it = effective_attrs.find("stroke");
            if (stroke_it != effective_attrs.end()) {
                std::string stroke_val = str_tolower(str_trim(stroke_it->second));
                if (stroke_val != "none" && stroke_val != "null") {
                    do_stroke = true;
                }
            }
            if (do_stroke) {
                pdf->_out(path);
                fill_and_stroke(attrs, true);
            }
        } else {
            pdf->_out(path);
            fill_and_stroke(attrs, true);
        }
    }
}

void SVGRenderer::draw_svg_text(const std::shared_ptr<HTMLNode>& elem) {
    // Parse position
    double x = 0, y = 0;
    auto it = elem->attrs.find("x"); if (it != elem->attrs.end()) x = parse_num(it->second);
    it = elem->attrs.find("y"); if (it != elem->attrs.end()) y = parse_num(it->second);

    // Merge style attribute into effective attrs
    std::map<std::string, std::string> ea = elem->attrs;
    auto style_it = elem->attrs.find("style");
    if (style_it != elem->attrs.end()) {
        std::map<std::string, std::string> sp;
        parse_svg_style(style_it->second, sp);
        for (auto& p : sp) {
            if (ea.find(p.first) == ea.end())
                ea[p.first] = p.second;
        }
    }

    // Font size (SVG user units = points in text mode)
    double font_size = 12;
    it = ea.find("font-size"); if (it != ea.end()) font_size = parse_num(it->second, 12);

    // Font family - use current font if not specified (supports Chinese)
    std::string family;
    it = ea.find("font-family");
    if (it != ea.end()) {
        family = str_trim(it->second);
        // Remove quotes
        if (family.size() >= 2 && (family.front() == '\'' || family.front() == '"'))
            family = family.substr(1, family.size() - 2);
    }
    if (family.empty() && pdf->FontFamily.size() > 0)
        family = pdf->FontFamily;
    if (family.empty()) family = "Helvetica";

    // Font weight / style
    std::string fstyle = "";
    it = ea.find("font-weight");
    if (it != ea.end() && (it->second == "bold" || it->second == "700")) fstyle += "B";
    it = ea.find("font-style");
    if (it != ea.end() && it->second == "italic") fstyle += "I";

    // Save and set font
    std::string saved_family = pdf->FontFamily;
    std::string saved_style = pdf->FontStyle;
    double saved_size = pdf->FontSizePt;
    pdf->SetFont(family, fstyle, font_size);

    // Text color from fill attribute
    int tr = 0, tg = 0, tb = 0;
    bool color_valid = false;
    it = ea.find("fill");
    if (it != ea.end()) {
        std::string fv = str_tolower(str_trim(it->second));
        if (fv != "none") parse_color(it->second, tr, tg, tb, color_valid);
    }
    std::string saved_tc = pdf->TextColor;
    if (color_valid) {
        pdf->SetTextColor(tr, tg, tb);
    }

    // Extract text content from children (TEXT nodes)
    std::string text_content;
    for (auto& child : elem->children) {
        if (child->type == HTMLNodeType::TEXT) {
            text_content += child->text;
        }
    }
    text_content = str_trim(text_content);

    if (!text_content.empty()) {
        // text-anchor: start (default), middle, end
        std::string anchor = "start";
        it = ea.find("text-anchor");
        if (it != ea.end()) anchor = str_tolower(str_trim(it->second));

        double adj_x = x;
        if (anchor == "middle" || anchor == "end") {
            double w_mm = pdf->GetStringWidth(text_content);
            double w_svg = w_mm * pdf->k / ctm_scale_x;
            if (anchor == "middle") adj_x -= w_svg / 2.0;
            else adj_x -= w_svg;
        }

        // Render text: combine position + y-flip in a single Tm
        // (Tm replaces the text matrix entirely, so Td + Tm loses the Td position)
        std::string txt_encoded = pdf->_utf8_to_utf16be(text_content);
        std::string s;
        if (pdf->ColorFlag)
            s += "q " + pdf->TextColor + " ";
        s += "BT ";
        s += "1 0 0 -1 " + fmt(adj_x) + " " + fmt(y) + " Tm ";
        s += txt_encoded;
        s += " Tj ET";
        if (pdf->ColorFlag)
            s += " Q";
        pdf->_out(s);
    }

    // Restore state
    pdf->TextColor = saved_tc;
    pdf->SetFont(saved_family, saved_style, saved_size);
}

}
