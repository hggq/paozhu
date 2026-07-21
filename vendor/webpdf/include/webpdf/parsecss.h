#ifndef PARSECSS_H
#define PARSECSS_H

#include <string>
#include <map>
#include <vector>

namespace pz {

struct CSSStyle {
    std::string font_family;
    std::string font_style;
    std::string font_weight;
    double font_size = 0;
    std::string color;
    std::string background_color;
    std::string background_image;
    std::string text_align;
    double line_height = 0;
    double margin_top = 0;
    double margin_bottom = 0;
    double margin_left = 0;
    double margin_right = 0;
    double padding_top = 0;
    double padding_bottom = 0;
    double padding_left = 0;
    double padding_right = 0;
    double width = 0;
    double width_pct = 0; // percentage width (0-100)
    double height = 0;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool has_bg_color = false;
    std::string border_style;
    double border_width = 0;
    std::string border_color;
    double border_radius = 0;
    // Individual border sides
    double border_top_w = 0;
    double border_bottom_w = 0;
    double border_left_w = 0;
    double border_right_w = 0;
    std::string border_top_color;
    std::string border_bottom_color;
    std::string border_left_color;
    std::string border_right_color;
    std::string position;
    bool page_break_before = false;
    std::string vertical_align; // top, middle, bottom
    std::string justify_content; // center, flex-start, flex-end
    bool margin_left_auto = false;
    bool margin_right_auto = false;

    double text_indent = 0;
    double letter_spacing = 0;
    double word_spacing = 0;
    std::string white_space;
    std::string overflow;
    std::string text_transform;
    std::string direction;
    std::string font_variant;
    bool small_caps = false;
    std::string text_shadow;
    std::string display;
    double opacity = 1.0;
    double top = 0;
    double right = 0;
    double bottom = 0;
    double left = 0;
    std::string float_val;
    std::string clear_val;
    int z_index = 0;
    std::string list_style_type;
    std::string list_style_position;
    std::string table_layout;
    std::string border_collapse;
    std::string caption_side;
    std::string empty_cells;
    std::string quotes;
    std::string content;
    std::string background_repeat;
    std::string background_position;
    std::string background_size;
    std::string background_attachment;

    void clear() {
        font_family.clear();
        font_style.clear();
        font_weight.clear();
        font_size = 0;
        color.clear();
        background_color.clear();
        background_image.clear();
        text_align.clear();
        line_height = 0;
        margin_top = margin_bottom = margin_left = margin_right = 0;
        padding_top = padding_bottom = padding_left = padding_right = 0;
        width = height = 0;
        width_pct = 0;
        bold = italic = underline = false;
        has_bg_color = false;
        border_style.clear();
        border_width = 0;
        border_color.clear();
        border_radius = 0;
        border_top_w = border_bottom_w = border_left_w = border_right_w = 0;
        border_top_color.clear();
        border_bottom_color.clear();
        border_left_color.clear();
        border_right_color.clear();
        position.clear();
        page_break_before = false;
        vertical_align.clear();
        justify_content.clear();
        margin_left_auto = false;
        margin_right_auto = false;
        text_indent = 0;
        letter_spacing = 0;
        word_spacing = 0;
        white_space.clear();
        overflow.clear();
        text_transform.clear();
        direction.clear();
        font_variant.clear();
        small_caps = false;
        text_shadow.clear();
        display.clear();
        opacity = 1.0;
        top = right = bottom = left = 0;
        float_val.clear();
        clear_val.clear();
        z_index = 0;
        list_style_type.clear();
        list_style_position.clear();
        table_layout.clear();
        border_collapse.clear();
        caption_side.clear();
        empty_cells.clear();
        quotes.clear();
        content.clear();
        background_repeat.clear();
        background_position.clear();
        background_size.clear();
        background_attachment.clear();
    }
};

class CSSParser {
public:
    CSSParser();
    ~CSSParser() = default;

    CSSStyle parse_inline(const std::string& style_str);
    std::map<std::string, CSSStyle> parse_stylesheet(const std::string& css);

    void merge_style(CSSStyle& dest, const CSSStyle& src);

private:
    std::string _trim(const std::string& s);
    std::string _to_lower(const std::string& s);
    double _parse_length(const std::string& val, double base = 12.0);
    std::string _parse_color(const std::string& val);
    void _apply_property(CSSStyle& style, const std::string& prop, const std::string& val);
};

}

#endif
