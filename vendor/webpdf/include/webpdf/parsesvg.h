#ifndef PARSESVG_H
#define PARSESVG_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "webpdf.h"
#include "parsehtml.h"

namespace pz {

class SVGRenderer {
public:
    SVGRenderer(webpdf* pdf);
    ~SVGRenderer() = default;

    // Render an SVG node at page position (x, y) in mm.
    // If target_w_mm / target_h_mm are > 0 they override the SVG's intrinsic
    // width/height so the drawing is scaled to the requested box; when both are
    // 0 the SVG's own width/height attributes are used.
    void render(const std::shared_ptr<HTMLNode>& svg_node, double x, double y,
                double target_w_mm = 0, double target_h_mm = 0);

private:
    webpdf* pdf;
    double origin_x;
    double origin_y;
    double svg_width;
    double svg_height;
    double ctm_scale_x;
    double ctm_scale_y;
    int render_depth_ = 0; // recursion guard for render_children/render_element
    static const int MAX_RENDER_DEPTH = 512;

    void render_children(const std::vector<std::shared_ptr<HTMLNode>>& children);
    void render_element(const std::shared_ptr<HTMLNode>& elem);

    void draw_rect(const std::map<std::string, std::string>& attrs);
    void draw_circle(const std::map<std::string, std::string>& attrs);
    void draw_ellipse(const std::map<std::string, std::string>& attrs);
    void draw_line(const std::map<std::string, std::string>& attrs);
    void draw_polygon(const std::map<std::string, std::string>& attrs, bool closed);
    void draw_path(const std::map<std::string, std::string>& attrs);
    void draw_svg_text(const std::shared_ptr<HTMLNode>& elem);

    void apply_element_transform(const std::map<std::string, std::string>& attrs);
    void fill_and_stroke(const std::map<std::string, std::string>& attrs, bool has_path);

    static std::string str_trim(const std::string& s);
    static std::string str_tolower(const std::string& s);
    static double parse_num(const std::string& s, double default_val = 0);
    static double parse_gradient_coord(const std::string& s, double default_val = 0);
    static void parse_color(const std::string& s, int& r, int& g, int& b, bool& valid);
    static std::string fmt(double v, int decimals = 2);
    static std::vector<double> parse_num_list(const std::string& s);

    struct GradientStop {
        double offset;
        int r, g, b;
        double opacity;
    };
    struct GradientDef {
        std::string id;
        std::vector<GradientStop> stops;
        bool is_linear = true;
        double x1 = 0, y1 = 0, x2 = 1, y2 = 0;
        double cx = 0, cy = 0, r = 0, fx = 0, fy = 0;
        std::string gradient_units = "objectBoundingBox";
    };
    std::map<std::string, GradientDef> gradients;
    std::map<std::string, int> gradient_shading_ids;
    void collect_defs(const std::vector<std::shared_ptr<HTMLNode>>& children);
    bool get_gradient_color(const std::string& id, int& r, int& g, int& b);
    bool is_gradient_fill(const std::map<std::string, std::string>& attrs, std::string& grad_id);
    void fill_gradient(const std::string& grad_id, double bx, double by, double bw, double bh);
};

}

#endif
