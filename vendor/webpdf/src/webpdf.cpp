/*******************************************************************************
* FPDF                                                                         *
*                                                                              *
* Version: 1.9                                                                 *
* Date:    2026-05-31                                                          *
* Author:  Olivier Plathey                                                     *
* Edit: Paozhu web framework transformation fpdf.php                           *
*******************************************************************************/
#include "webpdf.h"
#include "ttfparser.h"
#include "parsehtml.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <zlib.h>
#include <ctime>
#include <filesystem>

namespace pz {

#define WEBPDF_VERSION "1.9"

std::string webpdf::_str_tolower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

std::string webpdf::_str_toupper(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return res;
}

static std::string sprintf_str(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t needed = std::vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);
    if (needed > 0) {
        std::string buf(needed + 1, '\0');
        va_start(ap, fmt);
        std::vsnprintf(&buf[0], buf.size(), fmt, ap);
        va_end(ap);
        buf.resize(needed);
        return buf;
    }
    va_end(ap);
    return "";
}

static bool validate_path(const std::string& path) {
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
    if (path.size() >= 2 && path[0] == ':' && path[1] == '\\') return false;
    if (path.size() >= 2 && path[0] == '/' && path[1] == '/') return false;
    return true;
}

static std::string canonicalize_path(const std::string& base, const std::string& file) {
#ifdef _WIN32
    std::string normalized_base = base;
    std::string normalized_file = file;
    std::replace(normalized_base.begin(), normalized_base.end(), '/', '\\');
    std::replace(normalized_file.begin(), normalized_file.end(), '/', '\\');
    std::string full = normalized_base;
    if (!full.empty() && full.back() != '\\') full += '\\';
    full += normalized_file;
    std::string result;
    std::vector<std::string> parts;
    std::string part;
    for (char c : full) {
        if (c == '\\') {
            if (!part.empty()) {
                if (part == "..") {
                    if (!parts.empty()) parts.pop_back();
                } else if (part != ".") {
                    parts.push_back(part);
                }
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
        } else if (part != ".") {
            parts.push_back(part);
        }
    }
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) result += '\\';
        result += parts[i];
    }
    // Preserve a leading UNC/root backslash lost during segment splitting.
    if (!full.empty() && full[0] == '\\')
        result = "\\" + result;
    return result;
#else
    std::string normalized_base = base;
    std::string normalized_file = file;
    std::replace(normalized_base.begin(), normalized_base.end(), '\\', '/');
    std::replace(normalized_file.begin(), normalized_file.end(), '\\', '/');
    std::string full = normalized_base;
    if (!full.empty() && full.back() != '/') full += '/';
    full += normalized_file;
    std::string result;
    std::vector<std::string> parts;
    std::string part;
    for (char c : full) {
        if (c == '/') {
            if (!part.empty()) {
                if (part == "..") {
                    if (!parts.empty()) parts.pop_back();
                } else if (part != ".") {
                    parts.push_back(part);
                }
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
        } else if (part != ".") {
            parts.push_back(part);
        }
    }
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) result += '/';
        result += parts[i];
    }
    // Preserve the leading '/' of an absolute path, which is otherwise lost
    // because the split above discards the empty segment before it.
    if (!full.empty() && full[0] == '/')
        result = "/" + result;
    return result;
#endif
}

[[maybe_unused]] static bool is_valid_utf8(const unsigned char* s, size_t len) {
    size_t i = 0;
    while (i < len) {
        unsigned char c = s[i];
        if (c < 0x80) {
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= len || (s[i+1] & 0xC0) != 0x80) return false;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= len || (s[i+1] & 0xC0) != 0x80 || (s[i+2] & 0xC0) != 0x80) return false;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= len || (s[i+1] & 0xC0) != 0x80 || (s[i+2] & 0xC0) != 0x80 || (s[i+3] & 0xC0) != 0x80) return false;
            uint32_t cp = ((c & 0x07) << 18) | ((s[i+1] & 0x3F) << 12) | ((s[i+2] & 0x3F) << 6) | (s[i+3] & 0x3F);
            if (cp < 0x10000 || cp > 0x10FFFF) return false;
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

static uint32_t utf8_decode_char(const unsigned char*& s, size_t& len) {
    if (len == 0) return 0;
    unsigned char c = s[0];
    if (c < 0x80) {
        s++;
        len--;
        return c;
    }
    uint32_t cp = 0;
    int seq = 0;
    if ((c & 0xE0) == 0xC0) {
        cp = c & 0x1F;
        seq = 2;
    } else if ((c & 0xF0) == 0xE0) {
        cp = c & 0x0F;
        seq = 3;
    } else if ((c & 0xF8) == 0xF0) {
        cp = c & 0x07;
        seq = 4;
    } else {
        s++;
        len--;
        return 0;
    }
    for (int i = 1; i < seq && (size_t)i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) {
            s += i;
            len -= i;
            return 0;
        }
        cp = (cp << 6) | (b & 0x3F);
    }
    s += seq;
    len -= seq;
    if (seq == 4 && (cp < 0x10000 || cp > 0x10FFFF)) return 0;
    return cp;
}

[[maybe_unused]] static size_t utf8_char_length(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

[[maybe_unused]] static std::string utf8_to_utf16be(const std::string& s) {
    std::string result;
    result.reserve(s.size() * 2);
    const unsigned char* p = (const unsigned char*)s.data();
    size_t len = s.size();
    while (len > 0) {
        uint32_t cp = utf8_decode_char(p, len);
        if (cp == 0) continue;
        if (cp < 0x10000) {
            result += (char)((cp >> 8) & 0xFF);
            result += (char)(cp & 0xFF);
        } else {
            uint32_t surr = cp - 0x10000;
            uint16_t high = (uint16_t)(0xD800 + (surr >> 10));
            uint16_t low = (uint16_t)(0xDC00 + (surr & 0x3FF));
            result += (char)((high >> 8) & 0xFF);
            result += (char)(high & 0xFF);
            result += (char)((low >> 8) & 0xFF);
            result += (char)(low & 0xFF);
        }
    }
    return result;
}

std::string webpdf::_format_double(double val, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << val;
    return oss.str();
}

std::string webpdf::_read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw WebPDFException("Cannot open file: " + path);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string data(size, '\0');
    if (!file.read(&data[0], size))
        throw WebPDFException("Cannot read file: " + path);
    return data;
}

std::string webpdf::_resolve_image_path(const std::string& file) const {
    if (file.empty())
        return file;
    if (imagepath.empty())
        return file;
    if (file[0] == '/' || file[0] == '\\')
        return file;
    if (file.size() > 1 && file[1] == ':')
        return file;
    if (file.find("://") != std::string::npos || file.compare(0, 5, "data:") == 0)
        return file;
    if (!validate_path(file))
        return file;
    return canonicalize_path(imagepath, file);
}

void webpdf::Error(const std::string& msg) {
    throw WebPDFException(msg);
}

webpdf::webpdf(const std::string& orientation, const std::string& unit, const std::string& size) {
    page = 0;
    n = 2;
    state = 0;
    compress = true;
    k = 1.0;
    CurRotation = 0;
    wPt = 0;
    hPt = 0;
    w = 0;
    h = 0;
    lMargin = 0;
    tMargin = 0;
    rMargin = 0;
    bMargin = 0;
    cMargin = 0;
    xPos = 0;
    yPos = 0;
    lasth = 0;
    LineWidth = 0;
    DashPattern = "";
    DashPhase = 0;
    underline = false;
    CurrentFont = nullptr;
    FontSizePt = 12;
    FontSize = 0;
    FontPage = 0;
    ColorFlag = false;
    WithAlpha = false;
    ws = 0;
    DrawColor = "0.000 G";
    FillColor = "0.000 g";
    TextColor = "0.000 g";
    AutoPageBreak = false;
    PageBreakTrigger = 0;
    InHeader = false;
    InFooter = false;
    CreationDate = 0;
    PDFVersion = "1.3";
    aliasNbPages = "{nb}";
    ZoomMode = "default";
    LayoutMode = "";

    std::string u = _str_tolower(unit);
    if (u == "pt")
        k = 1.0;
    else if (u == "mm")
        k = 72.0 / 25.4;
    else if (u == "cm")
        k = 72.0 / 2.54;
    else if (u == "in")
        k = 72.0;
    else
        Error("Incorrect unit: " + unit);

    StdPageSizes["a3"] = {841.89, 1190.55};
    StdPageSizes["a4"] = {595.28, 841.89};
    StdPageSizes["a5"] = {420.94, 595.28};
    StdPageSizes["letter"] = {612, 792};
    StdPageSizes["legal"] = {612, 1008};

    auto sz = _getpagesize(size);
    DefPageSize = sz;
    CurPageSize = sz;

    std::string o = _str_tolower(orientation);
    if (o == "p" || o == "portrait") {
        DefOrientation = "P";
        w = sz.first;
        h = sz.second;
    } else if (o == "l" || o == "landscape") {
        DefOrientation = "L";
        w = sz.second;
        h = sz.first;
    } else {
        Error("Incorrect orientation: " + orientation);
    }
    CurOrientation = DefOrientation;

    wPt = w * k;
    hPt = h * k;

    double margin = 28.35 / k;
    SetMargins(margin, margin);
    cMargin = margin / 10;
    LineWidth = 0.567 / k;
    SetAutoPageBreak(true, 2 * margin);
    SetDisplayMode("default");
    metadata["Producer"] = "Paozhu webpdf from fpdf " + std::string(WEBPDF_VERSION);

    fontpath = "font/";
    CoreFonts = {"courier", "helvetica", "times", "symbol", "zapfdingbats"};
}

std::pair<double, double> webpdf::_getpagesize(const std::string& size) {
    std::string s = _str_tolower(size);
    if (StdPageSizes.find(s) != StdPageSizes.end()) {
        auto& a = StdPageSizes[s];
        return {a.first / k, a.second / k};
    }
    Error("Unknown page size: " + size);
    return {0, 0};
}

std::pair<double, double> webpdf::_getpagesize(const std::pair<double, double>& size) {
    if (size.first > 0 && size.second > 0)
        return size;
    Error("Page size is incorrect");
    return {0, 0};
}

void webpdf::SetMargins(double left, double top, double right) {
    lMargin = left;
    tMargin = top;
    if (right >= 0)
        rMargin = right;
    else
        rMargin = left;
}

void webpdf::setImagesPath(const std::string& p)
{
    imagepath = p;
}
void webpdf::SetLeftMargin(double margin) {
    lMargin = margin;
    if (page > 0 && xPos < lMargin)
        xPos = lMargin;
}

void webpdf::SetTopMargin(double margin) {
    tMargin = margin;
}

void webpdf::SetRightMargin(double margin) {
    rMargin = margin;
}

void webpdf::SetAutoPageBreak(bool auto_break, double margin) {
    AutoPageBreak = auto_break;
    bMargin = margin;
    if (auto_break)
        PageBreakTrigger = h - margin;
}

void webpdf::SetDisplayMode(const std::string& zoom, const std::string& layout) {
    ZoomMode = zoom;
    LayoutMode = layout;
}

void webpdf::SetCompression(bool comp) {
    compress = comp;
}

void webpdf::SetTitle(const std::string& title, bool isUTF8) {
    metadata["Title"] = isUTF8 ? title : _UTF8toUTF16(title);
}

void webpdf::SetAuthor(const std::string& author, bool isUTF8) {
    metadata["Author"] = isUTF8 ? author : _UTF8toUTF16(author);
}

void webpdf::SetSubject(const std::string& subject, bool isUTF8) {
    metadata["Subject"] = isUTF8 ? subject : _UTF8toUTF16(subject);
}

void webpdf::SetKeywords(const std::string& keywords, bool isUTF8) {
    metadata["Keywords"] = isUTF8 ? keywords : _UTF8toUTF16(keywords);
}

void webpdf::SetCreator(const std::string& creator, bool isUTF8) {
    metadata["Creator"] = isUTF8 ? creator : _UTF8toUTF16(creator);
}

void webpdf::AliasNbPages(const std::string& alias) {
    aliasNbPages = alias;
}

int webpdf::PageNo() {
    return page;
}

double webpdf::GetPageWidth() {
    return w;
}

double webpdf::GetPageHeight() {
    return h;
}

double webpdf::GetX() {
    return xPos;
}

void webpdf::SetX(double x) {
    if (x >= 0)
        xPos = x;
    else
        xPos = w + x;
}

double webpdf::GetY() {
    return yPos;
}

void webpdf::SetY(double y, bool resetX) {
    if (y >= 0)
        yPos = y;
    else
        yPos = h + y;
    if (resetX)
        xPos = lMargin;
}

void webpdf::SetXY(double x, double y) {
    SetX(x);
    SetY(y, false);
}

void webpdf::SetLineWidth(double width) {
    LineWidth = width;
    if (page > 0)
        _out(sprintf_str("%.2f w", width * k));
}

void webpdf::SetDash(const std::string& pattern, double phase) {
    DashPattern = pattern;
    DashPhase = phase;
    if (page > 0)
        _out(sprintf_str("[%s] %.2f d", pattern.c_str(), phase * k));
}

void webpdf::SetDrawColor(int r, int g, int b) {
    if (g == -1) g = r;
    if (b == -1) b = r;
    if ((r == 0 && g == 0 && b == 0) || (r == g && g == b))
        DrawColor = sprintf_str("%.3f G", r / 255.0);
    else
        DrawColor = sprintf_str("%.3f %.3f %.3f RG", r / 255.0, g / 255.0, b / 255.0);
    if (page > 0)
        _out(DrawColor);
}

void webpdf::SetFillColor(int r, int g, int b) {
    if (g == -1) g = r;
    if (b == -1) b = r;
    if ((r == 0 && g == 0 && b == 0) || (r == g && g == b))
        FillColor = sprintf_str("%.3f g", r / 255.0);
    else
        FillColor = sprintf_str("%.3f %.3f %.3f rg", r / 255.0, g / 255.0, b / 255.0);
    ColorFlag = (FillColor != TextColor);
    if (page > 0)
        _out(FillColor);
}

void webpdf::SetTextColor(int r, int g, int b) {
    if (g == -1) g = r;
    if (b == -1) b = r;
    if ((r == 0 && g == 0 && b == 0) || (r == g && g == b))
        TextColor = sprintf_str("%.3f g", r / 255.0);
    else
        TextColor = sprintf_str("%.3f %.3f %.3f rg", r / 255.0, g / 255.0, b / 255.0);
    ColorFlag = (FillColor != TextColor);
}

void webpdf::Line(double x1, double y1, double x2, double y2) {
    _out(sprintf_str("%.2f %.2f m %.2f %.2f l S", x1 * k, (h - y1) * k, x2 * k, (h - y2) * k));
}

void webpdf::Rect(double x, double y, double rw, double rh, const std::string& style) {
    double xk = x * k;
    double yk = (this->h - y) * k;
    double wk = rw * k;
    double hk = -rh * k;
    if (style == "F")
        _out(sprintf_str("%.2f %.2f %.2f %.2f re f", xk, yk, wk, hk));
    else if (style == "FD" || style == "DF")
        _out(sprintf_str("%.2f %.2f %.2f %.2f re B", xk, yk, wk, hk));
    else
        _out(sprintf_str("%.2f %.2f %.2f %.2f re S", xk, yk, wk, hk));
}

void webpdf::Ln(double lh) {
    if (lh < 0)
        lh = lasth;
    xPos = lMargin;
    yPos += lh;
}

void webpdf::Header() {
    if (!show_header_footer || !show_header)
        return;
    if (page_header_cb) {
        page_header_cb();
    }
}

void webpdf::SetPageNumberOffset(int offset) {
    page_no_offset = offset;
}

int webpdf::GetPageNumberOffset() {
    return page_no_offset;
}

void webpdf::SetShowHeaderFooter(bool show) {
    show_header_footer = show;
    show_header = show;
    show_footer = show;
}

bool webpdf::GetShowHeaderFooter() {
    return show_header_footer;
}

void webpdf::SetShowHeader(bool show) {
    show_header = show;
}

bool webpdf::GetShowHeader() {
    return show_header;
}

void webpdf::SetShowFooter(bool show) {
    show_footer = show;
}

bool webpdf::GetShowFooter() {
    return show_footer;
}

void webpdf::ClearPageContent() {
    if (!pages.empty()) {
        pages.back().clear();
    }
}

void webpdf::ClearHeaderFromPage() {
    if (!pages.empty() && header_start_pos < pages.back().size()) {
        pages.back().resize(header_start_pos);
    }
}

void webpdf::Footer() {
    if (!show_header_footer || !show_footer)
        return;
    if (page_footer_cb) {
        page_footer_cb();
        return;
    }
    if (page <= page_no_offset)
        return;
    int display_page = page - page_no_offset;
    std::string page_num = std::to_string(display_page);

    std::string saved_family = FontFamily;
    std::string saved_style = FontStyle;
    double saved_size = FontSize;
    std::string saved_color = TextColor;
    double saved_y = yPos;

    SetFont("Helvetica", "", 10);
    SetTextColor(128, 128, 128);
    yPos = h - bMargin + 10;
    Cell(0, 10, page_num, 0, 0, "C");

    FontFamily = saved_family;
    FontStyle = saved_style;
    FontSize = saved_size;
    FontSizePt = saved_size;
    TextColor = saved_color;
    yPos = saved_y;
}

FontInfo webpdf::_loadjsonfont(const std::string& path) {
    FontInfo font;
    std::string json_str = _read_file(path);
    http::obj_val obj;
    obj.from_json(json_str);

    font.type = obj["type"].to_string();
    font.name = obj["name"].to_string();
    if (obj.isset("uni"))
        font.uni = obj["uni"].to_bool();
    if (obj.isset("ttffile"))
        font.ttffile = obj["ttffile"].to_string();
    if (obj.isset("up"))
        font.up = (int)obj["up"].to_float();
    if (obj.isset("ut"))
        font.ut = (int)obj["ut"].to_float();

    // Composite (unicode) metrics cache produced by _save_font_json: per-Unicode
    // advance widths and the Unicode -> original-GID map.
    if (obj.isset("cw_uni")) {
        auto& cwu_obj = obj["cw_uni"];
        if (cwu_obj.is_object()) {
            auto& cwu_vec = cwu_obj.ref_obj();
            for (auto& kv : cwu_vec) {
                int code;
                try {
                    code = std::stoi(kv.first);
                } catch (const std::exception&) {
                    continue;
                }
                font.cw_uni[code] = (int)kv.second.to_float();
            }
        }
    }
    if (obj.isset("cidToGid")) {
        auto& c2g_obj = obj["cidToGid"];
        if (c2g_obj.is_object()) {
            auto& c2g_vec = c2g_obj.ref_obj();
            for (auto& kv : c2g_vec) {
                int code;
                try {
                    code = std::stoi(kv.first);
                } catch (const std::exception&) {
                    continue;
                }
                font.cidToGid[code] = (uint16_t)(int)kv.second.to_float();
            }
        }
    }

    if (obj.isset("cw")) {
        auto& cw_obj = obj["cw"];
        font.cw.resize(256, 0);
        if (cw_obj.is_array()) {
            auto& cw_arr = cw_obj.ref_array();
            int count = std::min((int)cw_arr.size(), 256);
            for (int i = 0; i < count; i++) {
                int cw_val = (int)cw_arr[i].to_float();
                font.cw[i] = cw_val;
                font.cw_map[(char)i] = cw_val;
            }
        } else {
            auto& cw_vec = cw_obj.ref_obj();
            for (auto& kv : cw_vec) {
                const std::string& key = kv.first;
                if (key.size() == 1) {
                    int idx = (unsigned char)key[0];
                    if (idx >= 0 && idx < 256) {
                        int cw_val = (int)kv.second.to_float();
                        font.cw[idx] = cw_val;
                        font.cw_map[key[0]] = cw_val;
                    }
                }
            }
        }
    }

    if (obj.isset("enc"))
        font.enc = obj["enc"].to_string();
    if (obj.isset("file"))
        font.file = obj["file"].to_string();
    if (obj.isset("originalsize"))
        font.originalsize = (int)obj["originalsize"].to_float();
    if (obj.isset("size1"))
        font.size1 = (int)obj["size1"].to_float();
    if (obj.isset("size2"))
        font.size2 = (int)obj["size2"].to_float();
    if (obj.isset("diff"))
        font.diff = obj["diff"].to_string();

    if (obj.isset("desc")) {
        auto& desc_obj = obj["desc"];
        auto& desc_vec = desc_obj.ref_obj();
        for (auto& kv : desc_vec) {
            font.desc[kv.first] = kv.second.to_string();
        }
    }

    if (obj.isset("subsetted"))
        font.subsetted = obj["subsetted"].to_bool();

    // A composite font loaded from a metrics cache still needs its subset table
    // primed with the low control code points, exactly as AddFont does when it
    // parses a TTF/OTF directly.
    if (font.uni) {
        font.subset.clear();
        for (int i = 0; i < 32; i++)
            font.subset[i] = true;
    }

    if (obj.isset("uv")) {
        auto& uv_obj = obj["uv"];
        auto& uv_vec = uv_obj.ref_obj();
        for (auto& kv : uv_vec) {
            int code;
            try {
                code = std::stoi(kv.first);
            } catch (const std::exception&) {
                continue;
            }
            auto& val = kv.second;
            if (val.is_array()) {
                int v = (int)val[0].to_float();
                int c = (int)val[1].to_float();
                font.uv[code] = UvEntry(v, c);
            } else {
                font.uv[code] = UvEntry((int)val.to_float());
            }
        }
    }

    return font;
}

FontInfo webpdf::_loadfont(const std::string& path) {
    return _loadjsonfont(path);
}

void webpdf::AddFont(const std::string& family, const std::string& style, const std::string& file, const std::string& dir) {
    std::string famil = _str_tolower(family);
    std::string styl = "";
    if (!style.empty()) {
        std::string s = _str_toupper(style);
        for (char c : s) {
            if (c == 'B' || c == 'I' || c == 'U')
                styl += c;
        }
        if (styl.find('B') != std::string::npos && styl.find('I') != std::string::npos)
            styl = "BI";
    }

    std::string key = famil + styl;
    if (fonts.find(key) != fonts.end())
        return;

    std::string fontfile = file;
    std::string fontdir = dir;
    if (fontdir.empty())
        fontdir = fontpath;
    if (!fontdir.empty() && fontdir.back() != '/' && fontdir.back() != '\\')
        fontdir += '/';

    if (fontfile.empty()) {
        bool is_core = false;
        for (auto& cf : CoreFonts) {
            if (cf == famil) {
                is_core = true;
                break;
            }
        }
        if (famil == "arial") {
            famil = "helvetica";
            is_core = true;
        }
        if (is_core) {
            fontfile = famil;
            if (!styl.empty() && famil != "symbol" && famil != "zapfdingbats")
                fontfile += _str_tolower(styl);
            fontfile += ".json";
        } else {
            fontfile = famil;
            if (!styl.empty())
                fontfile += _str_tolower(styl);
            fontfile += ".json";
        }
    }

    std::string path = fontdir + fontfile;
    FontInfo font;

    std::string ext = fontfile.substr(fontfile.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "ttf" || ext == "otf") {
        TTFParser ttf(path);
        ttf.Parse();

        font.type = ttf.is_otf ? "OTF" : "TTF";
        font.name = ttf.postScriptName;
        font.uni = true;
        font.ttffile = path;
        font.originalsize = 0;

        double unit_k = 1000.0 / ttf.unitsPerEm;
        font.up = (int)round(unit_k * ttf.underlinePosition);
        font.ut = (int)round(unit_k * ttf.underlineThickness);

        font.cw_uni.clear();
        font.cidToGid.clear();
        for (auto& kv : ttf.chars) {
            uint32_t uv = kv.first;
            uint16_t gid = kv.second;
            if (gid < ttf.glyphs.size()) {
                int gw = (int)round(unit_k * ttf.glyphs[gid].advanceWidth);
                font.cw_uni[(int)uv] = gw;
                font.cidToGid[(int)uv] = gid;
            }
        }

        int missingWidth = 0;
        if (ttf.glyphs.size() > 0)
            missingWidth = (int)round(unit_k * ttf.glyphs[0].advanceWidth);

        int flags = 0;
        if (ttf.isFixedPitch) flags += 1 << 0;
        flags += 1 << 5;
        if (ttf.italicAngle != 0) flags += 1 << 6;

        int ascent = (int)round(unit_k * ttf.hheaAscender);
        if (ascent == 0)
            ascent = (int)round(unit_k * ttf.typoAscender);
        if (ascent == 0)
            ascent = (int)round(unit_k * ttf.yMax);
        int descent = (int)round(unit_k * ttf.hheaDescender);
        if (descent == 0)
            descent = (int)round(unit_k * ttf.typoDescender);
        if (descent == 0)
            descent = (int)round(unit_k * ttf.yMin);
        int capH = (int)round(unit_k * ttf.capHeight);
        if (capH == 0)
            capH = ascent;

        font.desc["Ascent"] = std::to_string(ascent);
        font.desc["Descent"] = std::to_string(descent);
        font.desc["CapHeight"] = std::to_string(capH);
        font.desc["Flags"] = std::to_string(flags);
        font.desc["FontBBox"] = "[" + std::to_string((int)round(unit_k * ttf.xMin)) + " " +
                                std::to_string((int)round(unit_k * ttf.yMin)) + " " +
                                std::to_string((int)round(unit_k * ttf.xMax)) + " " +
                                std::to_string((int)round(unit_k * ttf.yMax)) + "]";
        font.desc["ItalicAngle"] = std::to_string(ttf.italicAngle);
        int stemv = (int)(50 + pow((double)ttf.usWeightClass / 65.0, 2));
        font.desc["StemV"] = std::to_string(stemv);
        font.desc["MissingWidth"] = std::to_string(missingWidth);

        font.enc = "Identity-H";
        font.file = "";
        font.subsetted = true;
        font.subset.clear();
        for (int i = 0; i < 32; i++)
            font.subset[i] = true;
    } else {
        font = _loadfont(path);
    }

    font.i = (int)fonts.size() + 1;

    if (!font.file.empty()) {
        std::string font_file_path = fontdir + font.file;
        if (FontFiles.find(font_file_path) == FontFiles.end()) {
            FontFileInfo ffi;
            ffi.length1 = font.originalsize;
            ffi.length2 = font.size2;
            ffi.n = 0;
            FontFiles[font_file_path] = ffi;
        }
    }

    fonts[key] = font;
}

void webpdf::_scan_font_dir() {
    if (_font_dir_scanned)
        return;
    _font_dir_scanned = true;

    std::string dir = fontpath;
    if (dir.empty())
        dir = "font/";

    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec))
        return;
    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec)
            break;
        if (!entry.is_regular_file(ec))
            continue;
        std::string fname = entry.path().filename().string();
        size_t dot = fname.find_last_of('.');
        if (dot == std::string::npos || dot == 0)
            continue;
        std::string ext = _str_tolower(fname.substr(dot + 1));
        if (ext != "ttf" && ext != "otf" && ext != "json")
            continue;
        std::string stem = fname.substr(0, dot);
        // Keep the real-case stem so we can rebuild exact paths on a
        // case-sensitive filesystem; the lowercase key lets us match the
        // already-lowercased family requested by SetFont.
        _font_file_index.emplace(_str_tolower(stem), stem);
    }
}

bool webpdf::_try_auto_register(const std::string& family, const std::string& style) {
    _scan_font_dir();
    if (_font_file_index.empty())
        return false;

    std::string styl = "";
    std::string s = _str_toupper(style);
    for (char c : s) {
        if (c == 'B' || c == 'I')
            styl += c;
    }
    if (styl == "IB")
        styl = "BI";

    std::string fam_lower = _str_tolower(family);

    // Prefer a style-specific file (e.g. "myfontb") over the plain family.
    std::vector<std::string> candidates;
    if (!styl.empty())
        candidates.push_back(fam_lower + _str_tolower(styl));
    candidates.push_back(fam_lower);

    std::string dir = fontpath;
    if (dir.empty())
        dir = "font/";
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
        dir += '/';

    std::string key = fam_lower + styl;

    // If the base family (or a sibling style) of this family is already
    // registered from a real font file, synthesize the requested style from
    // that SAME file. This respects the caller's explicit AddFont choice and
    // avoids pulling in a sibling file of a different format (e.g. loading the
    // .otf when the caller registered the .ttf), which can render very
    // differently: a CID-keyed CFF/.otf subset renders as garbage in some
    // viewers while the .ttf renders correctly.
    if (!styl.empty()) {
        for (const std::string& probe :
             {fam_lower, fam_lower + "B", fam_lower + "I", fam_lower + "BI"}) {
            auto fit = fonts.find(probe);
            if (fit == fonts.end() || fit->second.ttffile.empty())
                continue;
            std::filesystem::path p(fit->second.ttffile);
            try {
                AddFont(family, style, p.filename().string(), p.parent_path().string());
            } catch (...) {
                break;
            }
            if (fonts.find(key) != fonts.end())
                return true;
            break;
        }
    }

    for (auto& cand : candidates) {
        auto it = _font_file_index.find(cand);
        if (it == _font_file_index.end())
            continue;
        const std::string& stem = it->second; // real-case stem
        std::error_code ec;
        std::string otf_path  = dir + stem + ".otf";
        std::string ttf_path  = dir + stem + ".ttf";

        // Parse the source font directly. The full TTF/OTF parse is fast and
        // rebuilds the per-Unicode width and CID->GID maps in memory; we do
        // NOT persist those maps to JSON (they are far slower to parse back
        // than the binary they came from). Prefer the .ttf when both formats
        // are present: TrueType (glyf) subsets render reliably across viewers,
        // whereas CID-keyed CFF/.otf subsets do not.
        std::string src;
        if (std::filesystem::exists(ttf_path, ec))
            src = stem + ".ttf";
        else if (std::filesystem::exists(otf_path, ec))
            src = stem + ".otf";
        if (src.empty())
            continue;

        try {
            AddFont(family, style, src);
        } catch (...) {
            continue;
        }
        auto fit = fonts.find(key);
        if (fit != fonts.end())
            return true;
    }
    return false;
}

void webpdf::SetFont(const std::string& family, const std::string& style, double size) {
    std::string famil;
    if (family.empty())
        famil = FontFamily;
    else
        famil = _str_tolower(family);

    std::string styl = "";
    std::string s = _str_toupper(style);
    for (char c : s) {
        if (c == 'B' || c == 'I' || c == 'U')
            styl += c;
    }

    bool has_underline = (styl.find('U') != std::string::npos);
    if (has_underline) {
        underline = true;
        size_t u_pos = styl.find('U');
        styl.erase(u_pos, 1);
    } else {
        underline = false;
    }

    if (styl == "IB")
        styl = "BI";

    if (size == 0)
        size = FontSizePt;

    // Skip early return if we are on a different page — each page has its own
    // PDF graphics state, so Tf must be re-emitted even if font params match.
    if (FontFamily == famil && FontStyle == styl && FontSizePt == size && page == FontPage)
        return;

    std::string key = famil + styl;
    if (fonts.find(key) == fonts.end()) {
        bool is_core = false;
        std::string f = famil;
        if (f == "arial") {
            f = "helvetica";
        }
        for (auto& cf : CoreFonts) {
            if (cf == f) {
                is_core = true;
                break;
            }
        }
        if (is_core) {
            std::string style_for_add = styl;
            if (has_underline)
                style_for_add += "U";
            AddFont(f, style_for_add);
            key = f + styl;
        } else if (_try_auto_register(famil, styl)) {
            // A matching TTF/OTF (or its cached <family>.json) was found in
            // fontpath and registered on demand.
            key = famil + styl;
        } else if (famil == "monospace") {
            // Generic monospace maps to the built-in Courier (ASCII only).
            AddFont("courier", styl);
            famil = "courier";
            key = "courier" + styl;
        } else {
            // Requested family is not registered and is not a core font. Rather
            // than fail (which drops the text onto whatever font happens to be
            // current, typically a Latin-only core font that mangles CJK into
            // raw UTF-8), fall back to the default unicode-capable family that
            // the caller established before rendering. This also covers the
            // generic serif / sans-serif keywords.
            std::string fb = DefFontFamily;
            if (!fb.empty() && fonts.find(fb + styl) != fonts.end()) {
                famil = fb;
                key = fb + styl;
            } else if (!fb.empty() && fonts.find(fb) != fonts.end()) {
                famil = fb;
                styl = "";
                key = fb;
            } else {
                Error("Undefined font: " + family + " " + style);
            }
        }
    }

    FontFamily = famil;
    FontStyle = styl;
    CurrentFont = &fonts[key];
    // Remember the most recent unicode-capable font so undefined families can
    // fall back to it instead of a Latin-only core font.
    if (CurrentFont->uni)
        DefFontFamily = famil;

    SetFontSize(size);
}

void webpdf::SetFontSize(double size) {
    FontSizePt = size;
    FontSize = size / k;
    if (page > 0 && CurrentFont) {
        _out(sprintf_str("BT /F%d %.2f Tf ET", CurrentFont->i, FontSizePt));
        FontPage = page;
    }
}

double webpdf::GetStringWidth(const std::string& s) {
    if (!CurrentFont)
        Error("No font set");
    double width = 0;
    if (CurrentFont->uni) {
        size_t i = 0;
        while (i < s.size()) {
            unsigned char c = (unsigned char)s[i];
            int unicode = 0;
            if (c < 0x80) {
                unicode = c;
                i++;
            } else if (c < 0xE0) {
                if (i + 1 >= s.size()) break;
                unicode = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F);
                i += 2;
            } else if (c < 0xF0) {
                if (i + 2 >= s.size()) break;
                unicode = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F);
                i += 3;
            } else if (c < 0xF8) {
                // 4-byte UTF-8: U+10000..U+10FFFF
                if (i + 3 >= s.size()) break;
                unicode = ((c & 0x07) << 18) | (((unsigned char)s[i+1] & 0x3F) << 12) |
                          (((unsigned char)s[i+2] & 0x3F) << 6) | ((unsigned char)s[i+3] & 0x3F);
                i += 4;
            } else {
                i++;
                continue;
            }
            auto it = CurrentFont->cw_uni.find(unicode);
            if (it != CurrentFont->cw_uni.end())
                width += it->second;
            else if (unicode >= 0x1100) // CJK and other full-width scripts
                width += 1000; // assume full-width (1em)
            else
                width += 600; // fallback, consistent with MultiCell/GetMultiCellLines
        }
    } else {
        for (char c : s) {
            int idx = (unsigned char)c;
            if (idx < (int)CurrentFont->cw.size())
                width += CurrentFont->cw[idx];
            else
                width += 600; // fallback
        }
    }
    return width * FontSize / 1000;
}

int webpdf::GetCharWidthFU(uint32_t cp) {
    if (!CurrentFont) return 600;
    if (CurrentFont->uni) {
        auto it = CurrentFont->cw_uni.find((int)cp);
        if (it != CurrentFont->cw_uni.end())
            return it->second;
        return (cp >= 0x1100) ? 1000 : 600;
    } else {
        if (cp < CurrentFont->cw.size())
            return CurrentFont->cw[cp];
        return 600;
    }
}

std::string webpdf::_UTF8toUTF16(const std::string& s) {
    std::string res;
    res += (char)0xFE;
    res += (char)0xFF;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        int unicode = 0;
        if (c < 0x80) {
            unicode = c;
            i++;
        } else if (c < 0xE0) {
            if (i + 1 >= s.size()) break;
            unicode = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F);
            i += 2;
        } else if (c < 0xF0) {
            if (i + 2 >= s.size()) break;
            unicode = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F);
            i += 3;
        } else {
            i++;
            continue;
        }
        res += (char)(unicode >> 8);
        res += (char)(unicode & 0xFF);
    }
    return res;
}

std::string webpdf::_UTF8encode(const std::string& s) {
    return s;
}

bool webpdf::_isascii(const std::string& s) {
    for (char c : s) {
        if ((unsigned char)c > 127)
            return false;
    }
    return true;
}

std::string webpdf::_escape(const std::string& s) {
    std::string res;
    for (char c : s) {
        switch (c) {
            case '\\': res += "\\\\"; break;
            case '(': res += "\\("; break;
            case ')': res += "\\)"; break;
            case '\r': res += "\\r"; break;
            default: res += c; break;
        }
    }
    return res;
}

std::string webpdf::_textstring(const std::string& s) {
    return "(" + _escape(s) + ")";
}

std::string webpdf::_utf8_to_utf16be(const std::string& s) {
    if (!CurrentFont)
        return _textstring(s);

    if (!CurrentFont->uni)
        return _textstring(s);

    std::u16string utf16;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        int unicode = 0;
        if (c < 0x80) {
            unicode = c;
            i++;
        } else if (c < 0xE0) {
            if (i + 1 >= s.size()) break;
            unicode = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F);
            i += 2;
        } else if (c < 0xF0) {
            if (i + 2 >= s.size()) break;
            unicode = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F);
            i += 3;
        } else {
            i++;
            continue;
        }
        if (unicode >= 0 && unicode < 65536) {
            utf16 += (char16_t)unicode;
            CurrentFont->subset[unicode] = true;
        }
    }
    std::string hex_str = "<";
    static const char hex_chars[] = "0123456789ABCDEF";
    for (char16_t ch : utf16) {
        uint8_t hi = (ch >> 8) & 0xFF;
        uint8_t lo = ch & 0xFF;
        hex_str += hex_chars[hi >> 4];
        hex_str += hex_chars[hi & 0x0F];
        hex_str += hex_chars[lo >> 4];
        hex_str += hex_chars[lo & 0x0F];
    }
    hex_str += ">";
    return hex_str;
}

void webpdf::Text(double x, double y, const std::string& txt) {
    if (!CurrentFont)
        Error("No font set");

    std::string txt_out = _utf8_to_utf16be(txt);
    std::string s;
    if (ColorFlag)
        s += "q " + TextColor + " ";
    s += "BT ";
    s += sprintf_str("%.2f %.2f Td ", x * k, (h - y) * k);
    s += txt_out;
    s += " Tj ET";
    if (ColorFlag)
        s += " Q";
    _out(s);
    if (underline)
        _dounderline(x, y, txt);
}

void webpdf::Cell(double cw, double ch, const std::string& txt, int border, int ln, const std::string& align, bool fill, const std::string& link) {
    std::string bstr;
    if (border == 1)
        bstr = "1";
    Cell(cw, ch, txt, bstr, ln, align, fill, link);
}

void webpdf::Cell(double cw, double ch, const std::string& txt, const std::string& border, int ln, const std::string& align, bool fill, const std::string& link) {
    if (yPos + ch > PageBreakTrigger && !InHeader && !InFooter && AcceptPageBreak()) {
        double x = xPos;
        double old_ws = ws;
        if (ws > 0) {
            ws = 0;
            _out("0 Tw");
        }
        AddPage(CurOrientation, "", CurRotation);
        xPos = x;
        if (old_ws > 0) {
            ws = old_ws;
            _out(sprintf_str("%.3f Tw", ws * k));
        }
    }

    if (cw == 0)
        cw = this->w - rMargin - xPos;

    std::string s;
    if (fill || border == "1") {
        std::string op;
        if (fill)
            op = (border == "1") ? "B" : "f";
        else
            op = "S";
        s += sprintf_str("%.2f %.2f %.2f %.2f re %s ", xPos * k, (this->h - yPos) * k, cw * k, -ch * k, op.c_str());
    }

    if (!border.empty() && border != "1") {
        double x = xPos;
        double y = yPos;
        if (border.find('L') != std::string::npos)
            s += sprintf_str("%.2f %.2f m %.2f %.2f l S ", x * k, (this->h - y) * k, x * k, (this->h - (y + ch)) * k);
        if (border.find('T') != std::string::npos)
            s += sprintf_str("%.2f %.2f m %.2f %.2f l S ", x * k, (this->h - y) * k, (x + cw) * k, (this->h - y) * k);
        if (border.find('R') != std::string::npos)
            s += sprintf_str("%.2f %.2f m %.2f %.2f l S ", (x + cw) * k, (this->h - y) * k, (x + cw) * k, (this->h - (y + ch)) * k);
        if (border.find('B') != std::string::npos)
            s += sprintf_str("%.2f %.2f m %.2f %.2f l S ", x * k, (this->h - (y + ch)) * k, (x + cw) * k, (this->h - (y + ch)) * k);
    }

    if (!txt.empty()) {
        if (!CurrentFont)
            Error("No font has been set");

        double dx;
        if (align == "R")
            dx = cw - cMargin - GetStringWidth(txt);
        else if (align == "C")
            dx = (cw - GetStringWidth(txt)) / 2;
        else
            dx = cMargin;

        if (ColorFlag)
            s += "q " + TextColor + " ";
        s += "BT ";
        s += sprintf_str("%.2f %.2f Td ",
            (xPos + dx) * k,
            (this->h - (yPos + 0.5 * ch + 0.3 * FontSize)) * k);
        s += _utf8_to_utf16be(txt);
        s += " Tj ET";
        if (underline)
            s += " " + _dounderline(xPos + dx, yPos + 0.5 * ch + 0.3 * FontSize, txt);
        if (ColorFlag)
            s += " Q";
        if (!link.empty()) {
            bool is_int = true;
            for (char c : link) {
                if (c < '0' || c > '9') {
                    is_int = false;
                    break;
                }
            }
            if (is_int && !link.empty()) {
                try {
                    int link_id = std::stoi(link);
                    Link(xPos + dx, yPos + 0.5 * ch - 0.5 * FontSize, GetStringWidth(txt), FontSize, link_id);
                } catch (const std::exception&) {
                    Link(xPos + dx, yPos + 0.5 * ch - 0.5 * FontSize, GetStringWidth(txt), FontSize, link);
                }
            } else {
                Link(xPos + dx, yPos + 0.5 * ch - 0.5 * FontSize, GetStringWidth(txt), FontSize, link);
            }
        }
    }

    if (!s.empty())
        _out(s);

    lasth = ch;
    if (ln > 0) {
        yPos += ch;
        if (ln == 1)
            xPos = lMargin;
    } else {
        xPos += cw;
    }
}

std::string webpdf::_dounderline(double x, double y, const std::string& txt) {
    double up = CurrentFont ? CurrentFont->up : -10;
    double ut = CurrentFont ? CurrentFont->ut : 5;
    int nb_spaces = 0;
    for (char c : txt) {
        if (c == ' ')
            nb_spaces++;
    }
    double str_w = GetStringWidth(txt) + ws * nb_spaces;
    return sprintf_str("%.2f %.2f %.2f %.2f re f",
        x * k,
        (h - (y - up / 1000.0 * FontSize)) * k,
        str_w * k,
        -ut / 1000.0 * FontSizePt);
}

bool webpdf::AcceptPageBreak() {
    return AutoPageBreak;
}

void webpdf::MultiCell(double cw, double ch, const std::string& txt, int border, const std::string& align, bool fill) {
    std::string bstr;
    if (border == 1)
        bstr = "1";
    else if (border > 0)
        bstr = "LTRB";
    MultiCell(cw, ch, txt, bstr, align, fill);
}

void webpdf::MultiCell(double cw, double ch, const std::string& txt, const std::string& border, const std::string& align, bool fill) {
    if (!CurrentFont)
        Error("No font set");

    if (cw == 0)
        cw = this->w - rMargin - xPos;
    double wmax = (cw - 2 * cMargin) * 1000 / FontSize;

    std::string s = txt;
    size_t nb = s.size();
    if (nb > 0 && s[nb - 1] == '\n')
        nb--;
    
    std::string b;
    std::string b2;
    if (!border.empty()) {
        if (border == "1" || border == "LTRB") {
            b = "LRT";
            b2 = "LR";
        } else {
            b2 = "";
            if (border.find('L') != std::string::npos)
                b2 += 'L';
            if (border.find('R') != std::string::npos)
                b2 += 'R';
            b = (border.find('T') != std::string::npos) ? b2 + 'T' : b2;
        }
    }

    int sep = -1;  // byte index of last space (for word-wrap)
    size_t i = 0;  // current byte index
    size_t j = 0;  // start byte index of current line
    double l = 0;  // accumulated width of current line
    double ls = 0; // width at last space
    int ns = 0;    // number of spaces on current line
    int nl = 1;    // line number
    bool is_uni = CurrentFont->uni;

    while (i < nb) {
        char c = s[i];
        if (c == '\r') {
            i++;
            continue;
        }
        if (c == '\n') {
            if (ws > 0) {
                ws = 0;
                _out("0 Tw");
            }
            Cell(cw, ch, s.substr(j, i - j), b, 2, align, fill);
            i++;
            sep = -1;
            j = i;
            l = 0;
            ns = 0;
            nl++;
            if (!border.empty() && nl == 2)
                b = b2;
            continue;
        }
        if (c == ' ' || c == '\t') {
            sep = i;
            ls = l;
            ns++;
        }

        // Get character width (handle UTF-8)
        double cw_val = 0;
        size_t char_len = 1;
        if (is_uni) {
            unsigned char uc = (unsigned char)c;
            int unicode = 0;
            if (uc < 0x80) {
                unicode = uc;
                char_len = 1;
            } else if (uc < 0xE0) {
                unicode = (i + 1 < s.size()) ? (((uc & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F)) : 0;
                char_len = 2;
            } else if (uc < 0xF0) {
                unicode = (i + 2 < s.size()) ? (((uc & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F)) : 0;
                char_len = 3;
            } else {
                unicode = 0;
                char_len = 1;
            }
            auto it = CurrentFont->cw_uni.find(unicode);
            if (it != CurrentFont->cw_uni.end())
                cw_val = it->second;
            else if (unicode >= 0x1100)
                cw_val = 1000;
            else
                cw_val = 600;
        } else {
            int ci = (unsigned char)c;
            if (ci >= 0 && ci < (int)CurrentFont->cw.size())
                cw_val = CurrentFont->cw[ci];
            else
                cw_val = 600;
        }

        if (sep != -1 && (int)(i - sep) > 20) {
            sep = -1;
        }

        l += cw_val;
        if (l > wmax) {
            // Current character exceeds width
            bool break_at_space = (sep != -1);
            if (is_uni && sep != -1) {
                int pos = sep - 1;
                while (pos >= (int)j && (s[pos] == ' ' || s[pos] == '\t'))
                    pos--;
                if (pos >= (int)j) {
                    unsigned char uc = (unsigned char)s[pos];
                    if (uc >= 0x80)
                        break_at_space = false;
                }
            }
            if (!break_at_space) {
                // No space found, break at current position
                if (i == j)
                    i += char_len; // at least one char per line
                if (ws > 0) {
                    ws = 0;
                    _out("0 Tw");
                }
                Cell(cw, ch, s.substr(j, i - j), b, 2, align, fill);
            } else {
                // Break at last space
                if (align == "J") {
                    ws = (ns > 1) ? (wmax - ls) / 1000 * FontSize / (ns - 1) : 0;
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.3f Tw", ws * k);
                    _out(buf);
                }
                Cell(cw, ch, s.substr(j, sep - j), b, 2, align, fill);
                i = sep + 1;
            }
            sep = -1;
            j = i;
            l = 0;
            ns = 0;
            nl++;
            if (!border.empty() && nl == 2)
                b = b2;
        } else {
            i += char_len;
        }
    }
    if (ws > 0) {
        ws = 0;
        _out("0 Tw");
    }
    if (!border.empty() && border.find('B') != std::string::npos)
        b += 'B';
    Cell(cw, ch, s.substr(j, i - j), b, 2, align, fill);
    xPos = lMargin;
}

int webpdf::GetMultiCellLines(double cw, const std::string& txt) {
    if (!CurrentFont)
        return 1;

    if (cw == 0)
        cw = this->w - rMargin - xPos;
    double wmax = (cw - 2 * cMargin) * 1000 / FontSize;

    std::string s = txt;
    size_t nb = s.size();
    if (nb > 0 && s[nb - 1] == '\n')
        nb--;

    int sep = -1;
    size_t i = 0;
    size_t j = 0;
    double l = 0;
    int nl = 1;
    bool is_uni = CurrentFont->uni;

    while (i < nb) {
        char c = s[i];
        if (c == '\r') {
            i++;
            continue;
        }
        if (c == '\n') {
            i++;
            sep = -1;
            j = i;
            l = 0;
            nl++;
            continue;
        }
        if (c == ' ' || c == '\t') {
            sep = i;
            l += (is_uni ? GetStringWidth(std::string(1, c)) * 1000 / FontSize : CurrentFont->cw[(unsigned char)c]);
            i++;
            continue;
        }

        double cw_val = 0;
        size_t char_len = 1;
        if (is_uni) {
            unsigned char uc = (unsigned char)c;
            int unicode = 0;
            if (uc < 0x80) {
                unicode = uc;
                char_len = 1;
            } else if (uc < 0xE0) {
                unicode = (i + 1 < s.size()) ? (((uc & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F)) : 0;
                char_len = 2;
            } else if (uc < 0xF0) {
                unicode = (i + 2 < s.size()) ? (((uc & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F)) : 0;
                char_len = 3;
            } else {
                unicode = 0;
                char_len = 1;
            }
            auto it = CurrentFont->cw_uni.find(unicode);
            if (it != CurrentFont->cw_uni.end())
                cw_val = it->second;
            else if (unicode >= 0x1100)
                cw_val = 1000;
            else
                cw_val = 600;
        } else {
            int ci = (unsigned char)c;
            if (ci >= 0 && ci < (int)CurrentFont->cw.size())
                cw_val = CurrentFont->cw[ci];
            else
                cw_val = 600;
        }

        if (sep != -1 && (int)(i - sep) > 20) {
            sep = -1;
        }

        l += cw_val;
        if (l > wmax) {
            bool break_at_space = (sep != -1);
            if (is_uni && sep != -1) {
                int pos = sep - 1;
                while (pos >= (int)j && (s[pos] == ' ' || s[pos] == '\t'))
                    pos--;
                if (pos >= (int)j) {
                    unsigned char uc = (unsigned char)s[pos];
                    if (uc >= 0x80)
                        break_at_space = false;
                }
            }
            if (!break_at_space) {
                if (i == j)
                    i += char_len;
            } else {
                i = sep + 1;
            }
            sep = -1;
            j = i;
            l = 0;
            nl++;
        } else {
            i += char_len;
        }
    }

    return nl;
}

// Split text by line count: return first max_lines, keep remaining in txt
std::string webpdf::SplitTextByLines(double cw, std::string& txt, int max_lines) {
    if (max_lines <= 0 || txt.empty()) return "";
    if (!CurrentFont) return txt;

    if (cw == 0)
        cw = this->w - rMargin - xPos;
    double wmax = (cw - 2 * cMargin) * 1000 / FontSize;

    std::string s = txt;
    size_t nb = s.size();
    if (nb > 0 && s[nb - 1] == '\n')
        nb--;

    int sep = -1;
    size_t i = 0;
    size_t j = 0;
    double l = 0;
    int nl = 1;
    size_t last_line_end = 0;  // Position after the last line we want to keep
    bool is_uni = CurrentFont->uni;

    while (i < nb) {
        char c = s[i];
        if (c == '\r') {
            i++;
            continue;
        }
        if (c == '\n') {
            i++;
            if (nl == max_lines) {
                last_line_end = i;
                break;
            }
            sep = -1;
            j = i;
            l = 0;
            nl++;
            continue;
        }
        if (c == ' ' || c == '\t') {
            sep = i;
            l += (is_uni ? GetStringWidth(std::string(1, c)) * 1000 / FontSize : CurrentFont->cw[(unsigned char)c]);
            i++;
            continue;
        }

        double cw_val = 0;
        size_t char_len = 1;
        if (is_uni) {
            unsigned char uc = (unsigned char)c;
            int unicode = 0;
            if (uc < 0x80) {
                unicode = uc;
                char_len = 1;
            } else if (uc < 0xE0) {
                unicode = (i + 1 < s.size()) ? (((uc & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F)) : 0;
                char_len = 2;
            } else if (uc < 0xF0) {
                unicode = (i + 2 < s.size()) ? (((uc & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F)) : 0;
                char_len = 3;
            } else {
                unicode = 0;
                char_len = 1;
            }
            auto it = CurrentFont->cw_uni.find(unicode);
            if (it != CurrentFont->cw_uni.end())
                cw_val = it->second;
            else if (unicode >= 0x1100)
                cw_val = 1000;
            else
                cw_val = 600;
        } else {
            int ci = (unsigned char)c;
            if (ci >= 0 && ci < (int)CurrentFont->cw.size())
                cw_val = CurrentFont->cw[ci];
            else
                cw_val = 600;
        }

        if (sep != -1 && (int)(i - sep) > 20) {
            sep = -1;
        }

        l += cw_val;
        if (l > wmax) {
            bool break_at_space = (sep != -1);
            if (is_uni && sep != -1) {
                int pos = sep - 1;
                while (pos >= (int)j && (s[pos] == ' ' || s[pos] == '\t'))
                    pos--;
                if (pos >= (int)j) {
                    unsigned char uc = (unsigned char)s[pos];
                    if (uc >= 0x80)
                        break_at_space = false;
                }
            }
            if (!break_at_space) {
                if (i == j)
                    i += char_len;
            } else {
                i = sep + 1;
            }
            if (nl == max_lines) {
                last_line_end = i;
                break;
            }
            sep = -1;
            j = i;
            l = 0;
            nl++;
        } else {
            i += char_len;
        }
    }

    // If we didn't reach max_lines, return all text
    if (nl < max_lines || last_line_end == 0) {
        std::string result = txt;
        txt = "";
        return result;
    }

    // Split the text
    std::string first_part = txt.substr(0, last_line_end);
    txt = txt.substr(last_line_end);
    return first_part;
}

// Get the number of bytes that fit within max_lines lines
int webpdf::GetMultiCellCharCount(double cw, const std::string& txt, int max_lines) {
    if (!CurrentFont || txt.empty() || max_lines <= 0)
        return 0;

    if (cw == 0)
        cw = this->w - rMargin - xPos;
    double wmax = (cw - 2 * cMargin) * 1000 / FontSize;

    std::string s = txt;
    size_t nb = s.size();
    if (nb > 0 && s[nb - 1] == '\n')
        nb--;

    int sep = -1;
    size_t i = 0;
    size_t j = 0;
    double l = 0;
    int nl = 1;
    bool is_uni = CurrentFont->uni;

    while (i < nb) {
        char c = s[i];
        if (c == '\r') {
            i++;
            continue;
        }
        if (c == '\n') {
            i++;
            if (nl >= max_lines) {
                return (int)i;
            }
            sep = -1;
            j = i;
            l = 0;
            nl++;
            continue;
        }
        if (c == ' ' || c == '\t') {
            sep = i;
            l += (is_uni ? GetStringWidth(std::string(1, c)) * 1000 / FontSize : CurrentFont->cw[(unsigned char)c]);
            i++;
            continue;
        }

        double cw_val = 0;
        size_t char_len = 1;
        if (is_uni) {
            unsigned char uc = (unsigned char)c;
            int unicode = 0;
            if (uc < 0x80) {
                unicode = uc;
                char_len = 1;
            } else if (uc < 0xE0) {
                unicode = (i + 1 < s.size()) ? (((uc & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F)) : 0;
                char_len = 2;
            } else if (uc < 0xF0) {
                unicode = (i + 2 < s.size()) ? (((uc & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F)) : 0;
                char_len = 3;
            } else {
                unicode = 0;
                char_len = 1;
            }
            auto it = CurrentFont->cw_uni.find(unicode);
            if (it != CurrentFont->cw_uni.end())
                cw_val = it->second;
            else if (unicode >= 0x1100)
                cw_val = 1000;
            else
                cw_val = 600;
        } else {
            int ci = (unsigned char)c;
            if (ci >= 0 && ci < (int)CurrentFont->cw.size())
                cw_val = CurrentFont->cw[ci];
            else
                cw_val = 600;
        }

        if (sep != -1 && (int)(i - sep) > 20) {
            sep = -1;
        }

        l += cw_val;
        if (l > wmax) {
            bool break_at_space = (sep != -1);
            if (is_uni && sep != -1) {
                int pos = sep - 1;
                while (pos >= (int)j && (s[pos] == ' ' || s[pos] == '\t'))
                    pos--;
                if (pos >= (int)j) {
                    unsigned char uc = (unsigned char)s[pos];
                    if (uc >= 0x80)
                        break_at_space = false;
                }
            }
            if (!break_at_space) {
                if (i == j)
                    i += char_len;
            } else {
                i = sep + 1;
            }
            sep = -1;
            j = i;
            l = 0;
            nl++;
            if (nl > max_lines) {
                return (int)i;
            }
        } else {
            i += char_len;
        }
    }

    return (int)txt.size();
}

void webpdf::Write(double lh, const std::string& txt, const std::string& link) {
    if (!CurrentFont)
        Error("No font set");

    std::vector<int>& cw = CurrentFont->cw;
    double avail_w = this->w - rMargin - xPos;
    double wmax = (avail_w - 2 * cMargin) * 1000 / FontSize;
    std::string s = txt;
    size_t nb = s.size();
    int sep = -1;
    int i = 0;
    int j = 0;
    double l = 0;
    int nl = 1;

    while (i < (int)nb) {
        char c = s[i];
        if (c == '\n') {
            Cell(avail_w, lh, s.substr(j, i - j), 0, 2, "", false, link);
            i++;
            sep = -1;
            j = i;
            l = 0;
            if (nl == 1) {
                xPos = lMargin;
                avail_w = this->w - rMargin - xPos;
                wmax = (avail_w - 2 * cMargin) * 1000 / FontSize;
            }
            nl++;
            continue;
        }
        if (c == ' ')
            sep = i;
        int ci = (unsigned char)c;
        if (ci >= 0 && ci < (int)cw.size())
            l += cw[ci];
        else
            l += 600;
        if (l > wmax) {
            if (sep == -1) {
                if (xPos > lMargin) {
                    xPos = lMargin;
                    yPos += lh;
                    avail_w = this->w - rMargin - xPos;
                    wmax = (avail_w - 2 * cMargin) * 1000 / FontSize;
                    i++;
                    nl++;
                    continue;
                }
                if (i == j)
                    i++;
                Cell(avail_w, lh, s.substr(j, i - j), 0, 2, "", false, link);
            } else {
                Cell(avail_w, lh, s.substr(j, sep - j), 0, 2, "", false, link);
                i = sep + 1;
            }
            sep = -1;
            j = i;
            l = 0;
            if (nl == 1) {
                xPos = lMargin;
                avail_w = this->w - rMargin - xPos;
                wmax = (avail_w - 2 * cMargin) * 1000 / FontSize;
            }
            nl++;
        } else {
            i++;
        }
    }
    if (i != j)
        Cell(l / 1000 * FontSize, lh, s.substr(j), 0, 0, "", false, link);
}

void webpdf::WriteHTML(const std::string& html, double lh) {
    auto renderer = std::make_unique<HTMLRenderer>(this);
    renderer->WriteHTML(html, lh);
}

void webpdf::AddPage(const std::string& orientation, const std::string& size, int rotation) {
    if (state == 3)
        Error("The document is closed");

    // Simulate page break during dry-run: capture current page, reset Y, continue
    if (debugmodel && !dryrun_stack_.empty()) {
        DryRunFrame& top = dryrun_stack_.back();
        if (top.phase == DryRunFrame::DRYRUN) {
            DryRunPageInfo pi;
            pi.dryrun_buf = top.dryrun_buf;
            pi.height = yPos - top.last_page_start_y;
            if (pi.height < 0) pi.height = 0;
            top.pages.push_back(pi);
            top.page_break_count++;
            top.dryrun_buf.clear();
            top.last_page_start_y = tMargin;
            yPos = tMargin;
            xPos = lMargin;
            return;
        }
    }

    std::string family = FontFamily;
    std::string style = FontStyle + (underline ? "U" : "");
    double fontsize = FontSizePt;
    double lw = LineWidth;
    std::string dp = DashPattern;
    double dph = DashPhase;
    std::string dc = DrawColor;
    std::string fc = FillColor;
    std::string tc = TextColor;
    bool cf = ColorFlag;

    if (page > 0) {
        InFooter = true;
        Footer();
        InFooter = false;
        _endpage();
    }

    _beginpage(orientation, size, rotation);

    _out("2 J");
    LineWidth = lw;
    _out(sprintf_str("%.2f w", lw * k));
    DashPattern = dp;
    DashPhase = dph;
    if (!dp.empty())
        _out(sprintf_str("[%s] %.2f d", dp.c_str(), dph * k));

    // Reset font state so SetFont always outputs Tf for the new page's
    // independent content stream (PDF graphics state does not carry across pages)
    FontFamily = "";
    FontStyle = "";
    FontSizePt = 0;
    FontSize = 0;
    CurrentFont = nullptr;

    if (!family.empty())
        SetFont(family, style, fontsize);

    DrawColor = dc;
    if (dc != "0.000 G")
        _out(dc);
    FillColor = fc;
    if (fc != "0.000 g")
        _out(fc);
    TextColor = tc;
    ColorFlag = cf;

    InHeader = true;
    header_start_pos = pages.back().size();
    Header();
    InHeader = false;

    if (LineWidth != lw) {
        LineWidth = lw;
        _out(sprintf_str("%.2f w", lw * k));
    }

    if (!family.empty())
        SetFont(family, style, fontsize);

    if (DrawColor != dc) {
        DrawColor = dc;
        _out(dc);
    }
    if (FillColor != fc) {
        FillColor = fc;
        _out(fc);
    }
    TextColor = tc;
    ColorFlag = cf;
}

void webpdf::_beginpage(const std::string& orientation, const std::string& size, int rotation) {
    page++;
    std::string o = orientation;
    if (o.empty())
        o = DefOrientation;
    CurOrientation = (_str_tolower(o) == "l" || _str_tolower(o) == "landscape") ? "L" : "P";

    std::pair<double, double> sz;
    if (size.empty()) {
        sz = DefPageSize;
    } else {
        sz = _getpagesize(size);
    }

    if (CurOrientation == "L") {
        wPt = sz.second;
        hPt = sz.first;
    } else {
        wPt = sz.first;
        hPt = sz.second;
    }
    CurPageSize = {wPt, hPt};
    CurRotation = rotation;

    PageInfo pinfo;
    pinfo.size_w = wPt;
    pinfo.size_h = hPt;
    pinfo.has_size = !size.empty();
    pinfo.rotation = rotation;
    pinfo.has_rotation = (rotation != 0);
    PageInfo_vec.push_back(pinfo);
    PageBreakTrigger = h - bMargin;
    xPos = lMargin;
    yPos = tMargin;
    FontFamily = "";
    state = 1;

    pages.push_back("");
    PageLinks.push_back({});
}

void webpdf::_endpage() {
    state = 2;
}

void webpdf::_out(const std::string& s) {
    if (state < 1)
        Error("No page has been added yet");
    if (state == 2)
        Error("No page currently open");
    if (measure_mode_) {
        return; // discard all output during measurement
    }
    if (debugmodel) {
        if (!dryrun_stack_.empty()) {
            DryRunFrame& top = dryrun_stack_.back();
            if (top.phase == DryRunFrame::DRYRUN) {
                top.dryrun_buf += s + "\n";
            } else {
                top.direct_buf += s + "\n";
            }
        } else {
            buffer_debug += s + "\n";
        }
        return;
    }
    if (capture_mode) {
        capture_buf += s + "\n";
        return;
    }
    if (pages.empty())
        return;
    pages.back() += s + "\n";
}

void webpdf::StartCapture() {
    capture_mode = true;
    capture_buf.clear();
    // Disable auto page break during capture to avoid side effects
    AutoPageBreak = false;
}

std::string webpdf::StopCapture() {
    capture_mode = false;
    AutoPageBreak = true;
    std::string result = std::move(capture_buf);
    capture_buf.clear();
    return result;
}

void webpdf::WriteCaptured(const std::string& fragment) {
    if (!pages.empty())
        pages.back() += fragment;
}

// --- Dry-run mode: measure content height before drawing ---
void webpdf::BeginDryRun() {
    DryRunFrame frame;
    frame.start_y = yPos;
    frame.last_page_start_y = yPos;
    frame.saved_autopagebreak = AutoPageBreak;
    frame.phase = DryRunFrame::DRYRUN;
    dryrun_stack_.push_back(frame);
    debugmodel = true;
    AutoPageBreak = true;
}

double webpdf::EndDryRunAndFlush() {
    if (dryrun_stack_.empty()) {
        debugmodel = false;
        return 0;
    }
    DryRunFrame& top = dryrun_stack_.back();

    // Save the last page info
    DryRunPageInfo last_pi;
    last_pi.dryrun_buf = top.dryrun_buf;
    last_pi.height = yPos - top.last_page_start_y;
    if (last_pi.height < 0) last_pi.height = 0;
    top.pages.push_back(last_pi);

    double total_h = 0;
    for (auto& pi : top.pages) {
        total_h += pi.height;
    }

    top.phase = DryRunFrame::DIRECT;
    AutoPageBreak = top.saved_autopagebreak;
    return total_h;
}

void webpdf::FlushDryRunBuffer() {
    if (dryrun_stack_.empty()) {
        if (!pages.empty() && !buffer_debug.empty()) {
            pages.back() += buffer_debug;
        }
        buffer_debug.clear();
        return;
    }
    DryRunFrame frame = dryrun_stack_.back();
    dryrun_stack_.pop_back();

    std::string combined = frame.direct_buf + frame.dryrun_buf;

    if (dryrun_stack_.empty()) {
        if (!pages.empty() && !combined.empty()) {
            pages.back() += combined;
        }
        debugmodel = false;
        buffer_debug.clear();
    } else {
        DryRunFrame& parent = dryrun_stack_.back();
        if (parent.phase == DryRunFrame::DRYRUN) {
            parent.dryrun_buf += combined;
        } else {
            parent.direct_buf += combined;
        }
    }
}

bool webpdf::DryRunHasPageBreaks() {
    return DryRunPageBreakCount() > 0;
}

int webpdf::DryRunPageBreakCount() {
    if (dryrun_stack_.empty()) return 0;
    return dryrun_stack_.back().page_break_count;
}

int webpdf::DryRunPageCount() {
    if (dryrun_stack_.empty()) return 0;
    return (int)dryrun_stack_.back().pages.size();
}

double webpdf::DryRunPageHeight(int page_idx) {
    if (dryrun_stack_.empty()) return 0;
    auto& frame_pages = dryrun_stack_.back().pages;
    if (page_idx < 0 || page_idx >= (int)frame_pages.size()) return 0;
    return frame_pages[page_idx].height;
}

std::string webpdf::DryRunPageContent(int page_idx) {
    if (dryrun_stack_.empty()) return "";
    auto& frame_pages = dryrun_stack_.back().pages;
    if (page_idx < 0 || page_idx >= (int)frame_pages.size()) return "";
    return frame_pages[page_idx].dryrun_buf;
}

void webpdf::BeginMeasure() {
    measure_mode_ = true;
    measure_start_y_ = yPos;
    AutoPageBreak = false;
}

double webpdf::EndMeasure() {
    double measured_h = yPos - measure_start_y_;
    if (measured_h < 0) measured_h = 0;
    measure_mode_ = false;
    AutoPageBreak = true;
    return measured_h;
}


void webpdf::_put(const std::string& s) {
    buffer += s + "\n";
}

size_t webpdf::_getoffset() {
    return buffer.size();
}

void webpdf::_newobj(int obj_id) {
    int obj_num;
    if (obj_id > 0)
        obj_num = obj_id;
    else {
        (this->n)++;
        obj_num = this->n;
    }
    if ((int)offsets.size() <= obj_num)
        offsets.resize(obj_num + 1, 0);
    offsets[obj_num] = _getoffset();
    _put(sprintf_str("%d 0 obj", obj_num));
}

std::string webpdf::_compress_zlib(const std::string& data) {
    std::string result;
    uLongf destLen = ::compressBound((uLong)data.size());
    result.resize(destLen);
    if (::compress((Bytef*)&result[0], &destLen, (const Bytef*)data.data(), (uLong)data.size()) != Z_OK)
        Error("Compression failed");
    result.resize(destLen);
    return result;
}

std::string webpdf::_uncompress_zlib(const std::string& data, size_t max_size) {
    if (data.empty())
        return std::string();
    std::string result;
    size_t cap = data.size() * 10;
    if (cap < 1024)
        cap = 1024;
    if (max_size > 0 && cap > max_size)
        cap = max_size;
    uLongf destLen = (uLongf)cap;
    result.resize(destLen);
    while (true) {
        uLongf avail = destLen;
        int ret = ::uncompress((Bytef*)&result[0], &avail, (const Bytef*)data.data(), (uLong)data.size());
        if (ret == Z_OK) {
            result.resize(avail);
            return result;
        }
        if (ret != Z_BUF_ERROR)
            Error("Uncompression failed");
        if (max_size > 0 && (size_t)destLen >= max_size)
            Error("Uncompressed data too large");
        size_t next = (size_t)destLen * 2;
        if (next <= (size_t)destLen) // overflow
            Error("Uncompressed data too large");
        if (max_size > 0 && next > max_size)
            next = max_size;
        destLen = (uLongf)next;
        result.resize(destLen);
    }
}

void webpdf::_putstream(const std::string& data) {
    _put("stream");
    buffer += data + "\n";
    _put("endstream");
}

void webpdf::_putstreamobject(const std::string& data) {
    std::string p;
    std::string entries;
    if (compress) {
        entries = "/Filter /FlateDecode ";
        p = _compress_zlib(data);
    } else {
        p = data;
    }
    entries += "/Length " + std::to_string(p.size());
    _newobj();
    _put("<<" + entries + ">>");
    _putstream(p);
    _put("endobj");
}

void webpdf::Close() {
    if (state == 3)
        return;
    if (page == 0)
        Error("No page has been added");
    InFooter = true;
    Footer();
    InFooter = false;
    _endpage();
    _enddoc();
    state = 3;
}

void webpdf::_enddoc() {
    CreationDate = time(nullptr);
    _putheader();
    _putpages();
    _putresources();
    if (!Outlines.empty()) _putoutlines();
    _newobj();
    Info_n = n;
    _put("<<");
    _putinfo();
    _put(">>");
    _put("endobj");
    _newobj();
    Catalog_n = n;
    _put("<<");
    _putcatalog();
    _put(">>");
    _put("endobj");
    _puttrailer();
}

void webpdf::_putheader() {
    buffer = "%" + std::string("PDF-") + PDFVersion + "\n%\xE3\xE2\xCF\xD3\n";
    offsets.clear();
    offsets.resize(2, 0);
    n = 2;
}

void webpdf::_putpages() {
    int nb = page;
    int obj_n = this->n;
    for (int i = 0; i < nb; i++) {
        PageInfo_vec[i].n = ++obj_n;
        obj_n++;
        for (size_t j = 0; j < PageLinks[i].size(); j++)
            PageLinks[i][j].annot_obj = ++obj_n;
    }
    for (int i = 0; i < nb; i++)
        _putpage(i + 1);
    _newobj(1);
    _put("<</Type /Pages");
    std::string kids = "/Kids [";
    for (int i = 0; i < nb; i++) {
        kids += sprintf_str("%d 0 R ", PageInfo_vec[i].n);
    }
    kids += "]";
    _put(kids);
    _put(sprintf_str("/Count %d", nb));
    double wsize, hsize;
    if (DefOrientation == "P") {
        wsize = DefPageSize.first * k;
        hsize = DefPageSize.second * k;
    } else {
        wsize = DefPageSize.second * k;
        hsize = DefPageSize.first * k;
    }
    _put(sprintf_str("/MediaBox [0 0 %.2f %.2f]", wsize, hsize));
    _put(">>");
    _put("endobj");
}

void webpdf::_putpage(int page_num) {
    int idx = page_num - 1;
    _newobj();
    _put("<</Type /Page");
    _put("/Parent 1 0 R");
    if (PageInfo_vec[idx].has_size)
        _put(sprintf_str("/MediaBox [0 0 %.2f %.2f]", PageInfo_vec[idx].size_w, PageInfo_vec[idx].size_h));
    if (PageInfo_vec[idx].has_rotation)
        _put(sprintf_str("/Rotate %d", PageInfo_vec[idx].rotation));
    _put("/Resources 2 0 R");
    if (!PageLinks[idx].empty()) {
        std::string s = "/Annots [";
        for (size_t i = 0; i < PageLinks[idx].size(); i++) {
            if (i > 0) s += " ";
            s += sprintf_str("%d 0 R", PageLinks[idx][i].annot_obj);
        }
        s += "]";
        _put(s);
    }
    if (WithAlpha)
        _put("/Group <</Type /Group /S /Transparency /CS /DeviceRGB>>");
    _put(sprintf_str("/Contents %d 0 R>>", n + 1));
    _put("endobj");

    std::string page_content = pages[idx];
    if (!aliasNbPages.empty()) {
        size_t pos;
        while ((pos = page_content.find(aliasNbPages)) != std::string::npos) {
            page_content.replace(pos, aliasNbPages.size(), std::to_string(page));
        }
    }
    _putstreamobject(page_content);
    _putlinks(idx);
}

// Helper: convert UTF-8 to PDF text string with UTF-16BE encoding (with BOM)
static std::string _utf8_to_outline_str(const std::string& s) {
    // Check if all ASCII
    bool all_ascii = true;
    for (unsigned char c : s) {
        if (c >= 0x80) { all_ascii = false; break; }
    }
    if (all_ascii) {
        return "(" + s + ")";
    }
    // Convert to UTF-16BE with BOM
    std::string utf16be;
    utf16be += (char)0xFE; // BOM high
    utf16be += (char)0xFF; // BOM low
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        int unicode = 0;
        if (c < 0x80) {
            unicode = c; i++;
        } else if (c < 0xE0) {
            unicode = ((c & 0x1F) << 6) | ((unsigned char)s[i+1] & 0x3F);
            i += 2;
        } else if (c < 0xF0) {
            unicode = ((c & 0x0F) << 12) | (((unsigned char)s[i+1] & 0x3F) << 6) | ((unsigned char)s[i+2] & 0x3F);
            i += 3;
        } else {
            i++; continue;
        }
        if (unicode >= 0 && unicode < 65536) {
            utf16be += (char)((unicode >> 8) & 0xFF);
            utf16be += (char)(unicode & 0xFF);
        }
    }
    // Escape special chars
    std::string res;
    for (char c : utf16be) {
        if (c == '(' || c == ')' || c == '\\') res += '\\';
        res += c;
    }
    return "<" + [&]() {
        std::string hex;
        for (unsigned char c : utf16be) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02X", c);
            hex += buf;
        }
        return hex;
    }() + ">";
}

void webpdf::_putoutlines() {
    if (Outlines.empty()) return;

    int nb = (int)Outlines.size();

    // Build hierarchy: assign parent, find siblings and children
    std::vector<int> level_stack;
    for (int i = 0; i < nb; i++) {
        int lvl = Outlines[i].level;
        while (!level_stack.empty() && Outlines[level_stack.back()].level >= lvl)
            level_stack.pop_back();
        if (!level_stack.empty()) {
            Outlines[i].parent = level_stack.back();
            Outlines[level_stack.back()].last_child = i;
        }
        level_stack.push_back(i);
    }

    // Pre-assign object numbers (just increment n, don't write to buffer)
    // Outlines root dictionary gets the first object number
    n++;
    Outline_n = n;
    // Then each outline item gets subsequent numbers
    for (int i = 0; i < nb; i++) {
        n++;
        Outlines[i].n = n;
    }
    if ((int)offsets.size() <= n)
        offsets.resize(n + 1, 0);

    // Find first and last top-level items
    int first_n = 0, last_n = 0;
    for (int i = 0; i < nb; i++) {
        if (Outlines[i].parent == -1) {
            if (first_n == 0) first_n = Outlines[i].n;
            last_n = Outlines[i].n;
        }
    }

    // Write the Outlines root dictionary
    offsets[Outline_n] = _getoffset();
    _put(sprintf_str("%d 0 obj", Outline_n));
    _put("<<");
    _put("/Type /Outlines");
    if (first_n > 0) _put(sprintf_str("/First %d 0 R", first_n));
    if (last_n > 0) _put(sprintf_str("/Last %d 0 R", last_n));
    _put(sprintf_str("/Count %d", nb));
    _put(">>");
    _put("endobj");

    // Write each outline item
    for (int i = 0; i < nb; i++) {
        // Find prev/next siblings
        int prev_n = 0, next_n = 0;
        for (int j = i - 1; j >= 0; j--) {
            if (Outlines[j].parent == Outlines[i].parent && Outlines[j].level == Outlines[i].level) {
                prev_n = Outlines[j].n; break;
            }
        }
        for (int j = i + 1; j < nb; j++) {
            if (Outlines[j].parent == Outlines[i].parent && Outlines[j].level == Outlines[i].level) {
                next_n = Outlines[j].n; break;
            }
        }

        // Find first/last child and count descendants
        int first_child_n = 0, last_child_n = 0;
        int count = 0;
        for (int j = i + 1; j < nb; j++) {
            if (Outlines[j].parent == i) {
                if (first_child_n == 0) first_child_n = Outlines[j].n;
                last_child_n = Outlines[j].n;
            }
            if (Outlines[j].level > Outlines[i].level) {
                count++;
            } else if (j > i + 1 && Outlines[j].level <= Outlines[i].level) {
                break;
            }
        }

        offsets[Outlines[i].n] = _getoffset();
        _put(sprintf_str("%d 0 obj", Outlines[i].n));
        _put("<<");
        _put(sprintf_str("/Title %s", _utf8_to_outline_str(Outlines[i].title).c_str()));
        int parent_obj = (Outlines[i].parent >= 0) ? Outlines[Outlines[i].parent].n : Outline_n;
        _put(sprintf_str("/Parent %d 0 R", parent_obj));

        int page_idx = Outlines[i].page_no - 1;
        if (page_idx >= 0 && page_idx < (int)PageInfo_vec.size()) {
            double h_val;
            if (PageInfo_vec[page_idx].has_size)
                h_val = PageInfo_vec[page_idx].size_h;
            else
                h_val = (DefOrientation == "P") ? DefPageSize.second * k : DefPageSize.first * k;
            _put(sprintf_str("/Dest [%d 0 R /XYZ 0 %.2f null]", PageInfo_vec[page_idx].n, h_val));
        }
        if (prev_n > 0) _put(sprintf_str("/Prev %d 0 R", prev_n));
        if (next_n > 0) _put(sprintf_str("/Next %d 0 R", next_n));
        if (first_child_n > 0) {
            _put(sprintf_str("/First %d 0 R", first_child_n));
            _put(sprintf_str("/Last %d 0 R", last_child_n));
            _put(sprintf_str("/Count %d", count));
        }
        _put(">>");
        _put("endobj");
    }
}

void webpdf::_putlinks(int page_idx) {
    for (size_t i = 0; i < PageLinks[page_idx].size(); i++) {
        PageLink& pl = PageLinks[page_idx][i];
        _newobj();
        double x1 = pl.x * k;
        double y1 = (h - pl.y) * k;
        double x2 = (pl.x + pl.w) * k;
        double y2 = (h - pl.y - pl.h) * k;
        _put(sprintf_str("<</Type /Annot /Subtype /Link /Rect [%.2f %.2f %.2f %.2f] /Border [0 0 0] ", x1, y1, x2, y2));
        if (pl.is_uri) {
            _put(sprintf_str("/A <</S /URI /URI %s>>>>", _textstring(pl.link_str).c_str()));
        } else {
            auto& l = links[pl.link_int - 1];
            int page_id = l.first;
            double y_val = l.second;
            double h_val;
            if (PageInfo_vec[page_id - 1].has_size)
                h_val = PageInfo_vec[page_id - 1].size_h;
            else
                h_val = (DefOrientation == "P") ? DefPageSize.second * k : DefPageSize.first * k;
            _put(sprintf_str("/Dest [%d 0 R /XYZ 0 %.2f null]>>", PageInfo_vec[page_id - 1].n, h_val - y_val * k));
        }
        _put("endobj");
    }
}

void webpdf::_putresources() {
    _putfonts();
    _putimages();
    _putshadings();
    _newobj(2);
    _put("<<");
    _putresourcedict();
    _put(">>");
    _put("endobj");
}

void webpdf::_putresourcedict() {
    _put("/ProcSet [/PDF /Text /ImageB /ImageC /ImageI]");
    _put("/Font <<");
    std::vector<FontInfo*> font_list;
    for (auto& kv : fonts) {
        font_list.push_back(&kv.second);
    }
    std::sort(font_list.begin(), font_list.end(), [](FontInfo* a, FontInfo* b) {
        return a->i < b->i;
    });
    for (auto f : font_list) {
        _put(sprintf_str("/F%d %d 0 R", f->i, f->n));
    }
    _put(">>");
    _put("/XObject <<");
    for (auto& kv : images) {
        ImageInfo& img = kv.second;
        _put(sprintf_str("/I%d %d 0 R", img.i, img.n));
    }
    _put(">>");
    if (!shadings.empty()) {
        _put("/Shading <<");
        for (auto& s : shadings) {
            _put(sprintf_str("/Sh%d %d 0 R", s.i, s.n));
        }
        _put(">>");
    }
}

std::string webpdf::_tounicodecmap(const std::map<int, UvEntry>& uv) {
    std::string ranges;
    int nbr = 0;
    std::string chars;
    int nbc = 0;

    for (auto& kv : uv) {
        int c = kv.first;
        const UvEntry& v = kv.second;
        if (v.is_range) {
            ranges += sprintf_str("<%02X> <%02X> <%04X>\n", c, c + v.count - 1, v.value);
            nbr++;
        } else {
            chars += sprintf_str("<%02X> <%04X>\n", c, v.value);
            nbc++;
        }
    }

    std::string s = "/CIDInit /ProcSet findresource begin\n";
    s += "12 dict begin\n";
    s += "begincmap\n";
    s += "/CIDSystemInfo\n";
    s += "<</Registry (Adobe)\n";
    s += "/Ordering (UCS)\n";
    s += "/Supplement 0\n";
    s += ">> def\n";
    s += "/CMapName /Adobe-Identity-UCS def\n";
    s += "/CMapType 2 def\n";
    s += "1 begincodespacerange\n";
    s += "<00> <FF>\n";
    s += "endcodespacerange\n";
    if (nbr > 0) {
        s += sprintf_str("%d beginbfrange\n", nbr);
        s += ranges;
        s += "endbfrange\n";
    }
    if (nbc > 0) {
        s += sprintf_str("%d beginbfchar\n", nbc);
        s += chars;
        s += "endbfchar\n";
    }
    s += "endcmap\n";
    s += "CMapName currentdict /CMap defineresource pop\n";
    s += "end\n";
    s += "end";
    return s;
}

void webpdf::_putfonts() {
    for (auto& kv : FontFiles) {
        const std::string& file = kv.first;
        FontFileInfo& info = kv.second;
        _newobj();
        info.n = n;
        std::string font_data = _read_file(file);
        if (font_data.empty())
            Error("Font file not found: " + file);
        bool compressed = (file.size() >= 2 && file.substr(file.size() - 2) == ".z");
        if (!compressed && info.length2 > 0) {
            font_data = font_data.substr(6, info.length1) + font_data.substr(6 + info.length1 + 6, info.length2);
        }
        _put(sprintf_str("<</Length %d", (int)font_data.size()));
        if (compressed)
            _put("/Filter /FlateDecode");
        _put(sprintf_str("/Length1 %d", info.length1));
        if (info.length2 > 0)
            _put(sprintf_str("/Length2 %d /Length3 0", info.length2));
        _put(">>");
        _putstream(font_data);
        _put("endobj");
    }

    std::vector<FontInfo*> font_list;
    for (auto& kv : fonts) {
        font_list.push_back(&kv.second);
    }
    std::sort(font_list.begin(), font_list.end(), [](FontInfo* a, FontInfo* b) {
        return a->i < b->i;
    });

    for (auto font : font_list) {
        std::string cmapkey;
        bool has_uv = !font->uv.empty();
        bool has_diff = !font->diff.empty();

        if (has_diff) {
            if (encodings.find(font->enc) == encodings.end()) {
                _newobj();
                _put("<</Type /Encoding /BaseEncoding /WinAnsiEncoding /Differences [" + font->diff + "]>>");
                _put("endobj");
                encodings[font->enc] = n;
            }
        }

        if (has_uv) {
            if (!font->enc.empty())
                cmapkey = font->enc;
            else
                cmapkey = font->name;
            if (cmaps.find(cmapkey) == cmaps.end()) {
                std::string cmap = _tounicodecmap(font->uv);
                _putstreamobject(cmap);
                cmaps[cmapkey] = n;
            }
        }

        font->n = n + 1;
        std::string type = font->type;
        std::string name = font->name;
        if (font->subsetted)
            name = "AAAAAA+" + name;

        if (type == "Core") {
            _newobj();
            _put("<</Type /Font");
            _put("/BaseFont /" + name);
            _put("/Subtype /Type1");
            if (name != "Symbol" && name != "ZapfDingbats")
                _put("/Encoding /WinAnsiEncoding");
            if (has_uv)
                _put(sprintf_str("/ToUnicode %d 0 R", cmaps[cmapkey]));
            _put(">>");
            _put("endobj");
        } else if (type == "Type1" || type == "TrueType") {
            _newobj();
            _put("<</Type /Font");
            _put("/BaseFont /" + name);
            _put("/Subtype /" + type);
            _put("/FirstChar 32 /LastChar 255");
            _put(sprintf_str("/Widths %d 0 R", n + 1));
            _put(sprintf_str("/FontDescriptor %d 0 R", n + 2));
            if (has_diff)
                _put(sprintf_str("/Encoding %d 0 R", encodings[font->enc]));
            else
                _put("/Encoding /WinAnsiEncoding");
            if (has_uv)
                _put(sprintf_str("/ToUnicode %d 0 R", cmaps[cmapkey]));
            _put(">>");
            _put("endobj");

            _newobj();
            std::string s = "[";
            for (int i = 32; i <= 255; i++) {
                int cw_val = 0;
                if (i < (int)font->cw.size())
                    cw_val = font->cw[i];
                s += std::to_string(cw_val) + " ";
            }
            _put(s + "]");
            _put("endobj");

            _newobj();
            std::string desc_str = "<</Type /FontDescriptor /FontName /" + name;
            for (auto& dkv : font->desc) {
                desc_str += " /" + dkv.first + " " + dkv.second;
            }
            if (!font->file.empty()) {
                std::string font_file_key;
                for (auto& fkv : FontFiles) {
                    if (fkv.first.find(font->file) != std::string::npos) {
                        font_file_key = fkv.first;
                        break;
                    }
                }
                if (!font_file_key.empty()) {
                    std::string ff = (type == "Type1") ? "FontFile" : "FontFile2";
                    desc_str += " /" + ff + " " + std::to_string(FontFiles[font_file_key].n) + " 0 R";
                }
            }
            _put(desc_str + ">>");
            _put("endobj");
        } else if (type == "OTF") {
            TTFParser otf(font->ttffile);
            otf.Parse();

            std::vector<uint32_t> subset_chars;
            for (auto& kv : font->subset) {
                if (kv.second)
                    subset_chars.push_back((uint32_t)kv.first);
            }
            otf.Subset(subset_chars);
            std::string otffontstream = otf.Build();

            std::string fontstream = _compress_zlib(otffontstream);

            std::string fontname = "AAAAAA+" + font->name;

            int type0_n = n + 1;
            int cidfont_n = n + 2;
            int toUnicode_n = n + 3;
            int cidSystemInfo_n = n + 4;
            int fontDesc_n = n + 5;
            int fontFile_n = n + 6;
            int encCMap_n = n + 7;

            _newobj();
            _put("<</Type /Font");
            _put("/Subtype /Type0");
            _put("/BaseFont /" + fontname);
            // The subset is a CID-keyed CFF with an identity charset (CID==GID).
            // A custom Encoding CMap maps the 2-byte Unicode content codes to
            // those CIDs. Because the charset is identity, viewers that take the
            // Identity-ordering CID->GID shortcut (pdfium, CoreGraphics) and
            // viewers that honour the charset (Acrobat) all select the same
            // glyph.
            _put(sprintf_str("/Encoding %d 0 R", encCMap_n));
            _put(sprintf_str("/DescendantFonts [%d 0 R]", cidfont_n));
            _put(sprintf_str("/ToUnicode %d 0 R", toUnicode_n));
            _put(">>");
            _put("endobj");

            _newobj();
            _put("<</Type /Font");
            _put("/Subtype /CIDFontType0");
            _put("/BaseFont /" + fontname);
            _put(sprintf_str("/CIDSystemInfo %d 0 R", cidSystemInfo_n));
            _put(sprintf_str("/FontDescriptor %d 0 R", fontDesc_n));

            auto it_missing = font->desc.find("MissingWidth");
            if (it_missing != font->desc.end())
                _put("/DW " + it_missing->second);

            // /W is keyed by CID. With the identity charset CID == new GID, so
            // widths are listed against the GID; cffUniToGid maps Unicode -> GID
            // and cw_uni holds the width per Unicode.
            std::vector<std::pair<int, int>> gw; // (CID == new GID, width)
            for (auto& kv : otf.cffUniToGid) {
                int uv = (int)kv.first;
                int cid = (int)kv.second;
                int cw = 0;
                auto wit = font->cw_uni.find(uv);
                if (wit != font->cw_uni.end())
                    cw = wit->second;
                gw.push_back({cid, cw});
            }
            std::sort(gw.begin(), gw.end());

            std::string w_str = "";
            size_t i = 0;
            size_t count = gw.size();
            while (i < count) {
                int start = gw[i].first;
                size_t j = i;
                while (j + 1 < count && gw[j + 1].first == gw[j].first + 1)
                    j++;
                int end = gw[j].first;

                bool all_same = true;
                int first_w = gw[i].second;
                for (size_t ki = i + 1; ki <= j; ki++) {
                    if (gw[ki].second != first_w) {
                        all_same = false;
                        break;
                    }
                }

                if (all_same) {
                    w_str += std::to_string(start) + " " + std::to_string(end) + " " + std::to_string(first_w) + "\n";
                } else {
                    w_str += std::to_string(start) + " [ ";
                    for (size_t ki = i; ki <= j; ki++)
                        w_str += std::to_string(gw[ki].second) + " ";
                    w_str += "]\n";
                }
                i = j + 1;
            }
            _put("/W [" + w_str + "]");
            _put(">>");
            _put("endobj");

            _newobj();
            std::string toUni = "/CIDInit /ProcSet findresource begin\n";
            toUni += "12 dict begin\n";
            toUni += "begincmap\n";
            toUni += "/CIDSystemInfo\n";
            toUni += "<</Registry (Adobe)\n";
            toUni += "/Ordering (UCS)\n";
            toUni += "/Supplement 0\n";
            toUni += ">> def\n";
            toUni += "/CMapName /Adobe-Identity-UCS def\n";
            toUni += "/CMapType 2 def\n";
            toUni += "1 begincodespacerange\n";
            toUni += "<0000> <FFFF>\n";
            toUni += "endcodespacerange\n";
            toUni += "1 beginbfrange\n";
            toUni += "<0000> <FFFF> <0000>\n";
            toUni += "endbfrange\n";
            toUni += "endcmap\n";
            toUni += "CMapName currentdict /CMap defineresource pop\n";
            toUni += "end\n";
            toUni += "end";
            _put(sprintf_str("<</Length %d>>", (int)toUni.size()));
            _putstream(toUni);
            _put("endobj");

            _newobj();
            _put("<</Registry (Adobe)");
            _put("/Ordering (Identity)");
            _put("/Supplement 0");
            _put(">>");
            _put("endobj");

            _newobj();
            std::string desc_str2 = "<</Type /FontDescriptor";
            desc_str2 += " /FontName /" + fontname;
            for (auto& dkv : font->desc) {
                if (dkv.first == "Flags") {
                    int flags = std::stoi(dkv.second);
                    flags = flags | 4;
                    flags = flags & ~32;
                    desc_str2 += " /Flags " + std::to_string(flags);
                } else {
                    desc_str2 += " /" + dkv.first + " " + dkv.second;
                }
            }
            desc_str2 += " /FontFile3 " + std::to_string(fontFile_n) + " 0 R";
            _put(desc_str2 + ">>");
            _put("endobj");

            _newobj();
            _put(sprintf_str("<</Length %d", (int)fontstream.size()));
            _put("/Filter /FlateDecode");
            _put("/Subtype /OpenType");
            _put(">>");
            _putstream(fontstream);
            _put("endobj");

            // Encoding CMap: content code (2-byte Unicode) -> CID (== new GID).
            // This replaces Identity-H: because the CFF charset is now identity
            // (CID==GID), the Unicode->glyph redirection has to live here so it
            // is honoured by every viewer.
            std::vector<std::pair<int, int>> uni2cid; // (Unicode, CID)
            for (auto& kv : otf.cffUniToGid)
                uni2cid.push_back({(int)kv.first, (int)kv.second});
            std::sort(uni2cid.begin(), uni2cid.end());

            std::string cr_body;
            {
                std::string block;
                int blkn = 0;
                auto flush_block = [&]() {
                    if (blkn == 0)
                        return;
                    cr_body += std::to_string(blkn) + " begincidrange\n" + block + "endcidrange\n";
                    block.clear();
                    blkn = 0;
                };
                size_t bi = 0, bm = uni2cid.size();
                while (bi < bm) {
                    int u0 = uni2cid[bi].first;
                    int c0 = uni2cid[bi].second;
                    size_t bj = bi;
                    while (bj + 1 < bm &&
                           uni2cid[bj + 1].first == uni2cid[bj].first + 1 &&
                           uni2cid[bj + 1].second == uni2cid[bj].second + 1)
                        bj++;
                    int u1 = uni2cid[bj].first;
                    block += sprintf_str("<%04X> <%04X> %d\n", u0, u1, c0);
                    blkn++;
                    if (blkn == 100)
                        flush_block();
                    bi = bj + 1;
                }
                flush_block();
            }
            std::string enc_cmap = "/CIDInit /ProcSet findresource begin\n";
            enc_cmap += "12 dict begin\n";
            enc_cmap += "begincmap\n";
            enc_cmap += "/CIDSystemInfo <</Registry (Adobe) /Ordering (Identity) /Supplement 0>> def\n";
            enc_cmap += "/CMapName /PZ-Identity def\n";
            enc_cmap += "/CMapType 1 def\n";
            enc_cmap += "1 begincodespacerange\n";
            enc_cmap += "<0000> <FFFF>\n";
            enc_cmap += "endcodespacerange\n";
            enc_cmap += cr_body;
            enc_cmap += "endcmap\n";
            enc_cmap += "CMapName currentdict /CMap defineresource pop\n";
            enc_cmap += "end\n";
            enc_cmap += "end";
            _newobj();
            _put("<</Type /CMap");
            _put("/CMapName /PZ-Identity");
            _put("/CIDSystemInfo <</Registry (Adobe) /Ordering (Identity) /Supplement 0>>");
            _put(sprintf_str("/Length %d>>", (int)enc_cmap.size()));
            _putstream(enc_cmap);
            _put("endobj");

            font->n = type0_n;
        } else if (type == "TTF" || type == "TrueTypeUnicode" || type == "CIDFontType2") {
            TTFParser ttf(font->ttffile);
            ttf.Parse();

            std::vector<uint32_t> subset_chars;
            for (auto& kv : font->subset) {
                if (kv.second)
                    subset_chars.push_back((uint32_t)kv.first);
            }
            ttf.Subset(subset_chars);
            std::string ttfontstream = ttf.Build();
            int ttfontsize = (int)ttfontstream.size();

            std::string fontstream = _compress_zlib(ttfontstream);

            std::string fontname = "AAAAAA+" + font->name;

            std::map<int, uint16_t> codeToGlyph;
            for (auto& kv : font->cidToGid) {
                int uv = kv.first;
                if (font->subset[uv]) {
                    uint16_t oldGid = kv.second;
                    uint16_t newGid = 0;
                    auto it = ttf.glyphMap.find(oldGid);
                    if (it != ttf.glyphMap.end())
                        newGid = it->second;
                    codeToGlyph[uv] = newGid;
                }
            }

            int type0_n = n + 1;
            int cidfont_n = n + 2;
            int toUnicode_n = n + 3;
            int cidSystemInfo_n = n + 4;
            int fontDesc_n = n + 5;
            int cidToGidMap_n = n + 6;
            int fontFile_n = n + 7;

            _newobj();
            _put("<</Type /Font");
            _put("/Subtype /Type0");
            _put("/BaseFont /" + fontname);
            _put("/Encoding /Identity-H");
            _put(sprintf_str("/DescendantFonts [%d 0 R]", cidfont_n));
            _put(sprintf_str("/ToUnicode %d 0 R", toUnicode_n));
            _put(">>");
            _put("endobj");

            _newobj();
            _put("<</Type /Font");
            _put("/Subtype /CIDFontType2");
            _put("/BaseFont /" + fontname);
            _put(sprintf_str("/CIDSystemInfo %d 0 R", cidSystemInfo_n));
            _put(sprintf_str("/FontDescriptor %d 0 R", fontDesc_n));

            auto it_missing = font->desc.find("MissingWidth");
            if (it_missing != font->desc.end())
                _put("/DW " + it_missing->second);

            std::vector<int> uv_list;
            for (auto& kv : font->cw_uni) {
                if (kv.first >= 0 && kv.first < 65536 && font->subset[kv.first])
                    uv_list.push_back(kv.first);
            }
            std::sort(uv_list.begin(), uv_list.end());

            std::string w_str = "";
            size_t i = 0;
            size_t count = uv_list.size();
            while (i < count) {
                int start = uv_list[i];
                size_t j = i;
                while (j + 1 < count && uv_list[j + 1] == uv_list[j] + 1)
                    j++;
                int end = uv_list[j];

                bool all_same = true;
                int first_w = font->cw_uni[start];
                for (int ki = start + 1; ki <= end; ki++) {
                    if (font->cw_uni[ki] != first_w) {
                        all_same = false;
                        break;
                    }
                }

                if (all_same) {
                    w_str += std::to_string(start) + " " + std::to_string(end) + " " + std::to_string(first_w) + "\n";
                } else {
                    w_str += std::to_string(start) + " [ ";
                    for (int ki = start; ki <= end; ki++)
                        w_str += std::to_string(font->cw_uni[ki]) + " ";
                    w_str += "]\n";
                }
                i = j + 1;
            }
            _put("/W [" + w_str + "]");
            _put(sprintf_str("/CIDToGIDMap %d 0 R", cidToGidMap_n));
            _put(">>");
            _put("endobj");

            _newobj();
            std::string toUni = "/CIDInit /ProcSet findresource begin\n";
            toUni += "12 dict begin\n";
            toUni += "begincmap\n";
            toUni += "/CIDSystemInfo\n";
            toUni += "<</Registry (Adobe)\n";
            toUni += "/Ordering (UCS)\n";
            toUni += "/Supplement 0\n";
            toUni += ">> def\n";
            toUni += "/CMapName /Adobe-Identity-UCS def\n";
            toUni += "/CMapType 2 def\n";
            toUni += "1 begincodespacerange\n";
            toUni += "<0000> <FFFF>\n";
            toUni += "endcodespacerange\n";
            toUni += "1 beginbfrange\n";
            toUni += "<0000> <FFFF> <0000>\n";
            toUni += "endbfrange\n";
            toUni += "endcmap\n";
            toUni += "CMapName currentdict /CMap defineresource pop\n";
            toUni += "end\n";
            toUni += "end";
            _put(sprintf_str("<</Length %d>>", (int)toUni.size()));
            _putstream(toUni);
            _put("endobj");

            _newobj();
            _put("<</Registry (Adobe)");
            _put("/Ordering (UCS)");
            _put("/Supplement 0");
            _put(">>");
            _put("endobj");

            _newobj();
            std::string desc_str2 = "<</Type /FontDescriptor";
            desc_str2 += " /FontName /" + fontname;
            for (auto& dkv : font->desc) {
                if (dkv.first == "Flags") {
                    int flags = std::stoi(dkv.second);
                    flags = flags | 4;
                    flags = flags & ~32;
                    desc_str2 += " /Flags " + std::to_string(flags);
                } else {
                    desc_str2 += " /" + dkv.first + " " + dkv.second;
                }
            }
            desc_str2 += " /FontFile2 " + std::to_string(fontFile_n) + " 0 R";
            _put(desc_str2 + ">>");
            _put("endobj");

            std::string cidtogidmap;
            cidtogidmap.resize(256 * 256 * 2, '\x00');
            for (auto& kv : codeToGlyph) {
                int cc = kv.first;
                uint16_t glyph = kv.second;
                if (cc >= 0 && cc < 65536) {
                    cidtogidmap[cc * 2] = (char)((glyph >> 8) & 0xFF);
                    cidtogidmap[cc * 2 + 1] = (char)(glyph & 0xFF);
                }
            }
            std::string cidtogid_compressed = _compress_zlib(cidtogidmap);
            _newobj();
            _put(sprintf_str("<</Length %d", (int)cidtogid_compressed.size()));
            _put("/Filter /FlateDecode");
            _put(">>");
            _putstream(cidtogid_compressed);
            _put("endobj");

            _newobj();
            _put(sprintf_str("<</Length %d", (int)fontstream.size()));
            _put("/Filter /FlateDecode");
            _put(sprintf_str("/Length1 %d", ttfontsize));
            _put(">>");
            _putstream(fontstream);
            _put("endobj");

            font->n = type0_n;
        }
    }
}

void webpdf::_putimages() {
    for (auto& kv : images) {
        ImageInfo& info = kv.second;
        _putimage(info);
    }
}

void webpdf::_putshadings() {
    for (auto& s : shadings) {
        _putshading(s);
    }
}

void webpdf::_putshading(AxialShadingInfo& info) {
    if (info.stops.size() < 2) return;

    int func_n = 0;
    if (info.stops.size() == 2) {
        _newobj();
        func_n = this->n;
        _put("<<");
        _put("/FunctionType 2");
        _put("/Domain [0 1]");
        _put("/C0 [" +
            std::to_string(info.stops[0].r / 255.0) + " " +
            std::to_string(info.stops[0].g / 255.0) + " " +
            std::to_string(info.stops[0].b / 255.0) + "]");
        _put("/C1 [" +
            std::to_string(info.stops[1].r / 255.0) + " " +
            std::to_string(info.stops[1].g / 255.0) + " " +
            std::to_string(info.stops[1].b / 255.0) + "]");
        _put("/N 1");
        _put(">>");
        _put("endobj");
    } else {
        _newobj();
        func_n = this->n;
        int size = 256;
        std::string sample_data;
        for (int i = 0; i < size; i++) {
            double t = (double)i / (size - 1);
            double r = 0, g = 0, b = 0;
            for (size_t j = 0; j < info.stops.size() - 1; j++) {
                double t0 = info.stops[j].offset;
                double t1 = info.stops[j+1].offset;
                if (t >= t0 && t <= t1) {
                    double f = (t - t0) / (t1 - t0);
                    r = info.stops[j].r + f * (info.stops[j+1].r - info.stops[j].r);
                    g = info.stops[j].g + f * (info.stops[j+1].g - info.stops[j].g);
                    b = info.stops[j].b + f * (info.stops[j+1].b - info.stops[j].b);
                    break;
                }
            }
            sample_data += (char)(int)r;
            sample_data += (char)(int)g;
            sample_data += (char)(int)b;
        }
        _put("<<");
        _put("/FunctionType 0");
        _put("/Domain [0 1]");
        _put("/Range [0 1 0 1 0 1]");
        _put("/Size [256]");
        _put("/BitsPerSample 8");
        _put(sprintf_str("/Length %d>>", (int)sample_data.size()));
        _putstream(sample_data);
        _put("endobj");
    }

    _newobj();
    info.n = this->n;
    _put("<<");
    if (info.is_radial) {
        _put("/ShadingType 3");
        _put("/ColorSpace /DeviceRGB");
        _put(sprintf_str("/Coords [%.3f %.3f %.3f %.3f %.3f %.3f]", info.x0, info.y0, info.r0, info.x1, info.y1, info.r1));
    } else {
        _put("/ShadingType 2");
        _put("/ColorSpace /DeviceRGB");
        _put(sprintf_str("/Coords [%.3f %.3f %.3f %.3f]", info.x0, info.y0, info.x1, info.y1));
    }
    _put("/Function " + std::to_string(func_n) + " 0 R");
    _put("/Extend [true true]");
    _put(">>");
    _put("endobj");
}

void webpdf::_putimage(ImageInfo& info) {
    _newobj();
    info.n = this->n;
    _put("<</Type /XObject");
    _put("/Subtype /Image");
    _put(sprintf_str("/Width %d", info.w));
    _put(sprintf_str("/Height %d", info.h));
    if (info.cs == "Indexed") {
        _put(sprintf_str("/ColorSpace [/Indexed /DeviceRGB %d %d 0 R]", (int)(info.pal.size() / 3 - 1), this->n + 1));
    } else {
        _put("/ColorSpace /" + info.cs);
        if (info.cs == "DeviceCMYK")
            _put("/Decode [1 0 1 0 1 0 1 0]");
    }
    _put(sprintf_str("/BitsPerComponent %d", info.bpc));
    if (!info.f.empty())
        _put("/Filter /" + info.f);
    if (!info.dp.empty())
        _put("/DecodeParms <<" + info.dp + ">>");
    if (!info.trns.empty()) {
        std::string trns_str;
        for (size_t i = 0; i < info.trns.size(); i++) {
            if (i > 0) trns_str += " ";
            trns_str += std::to_string(info.trns[i]) + " " + std::to_string(info.trns[i]);
        }
        _put("/Mask [" + trns_str + "]");
    }
    if (!info.smask.empty())
        _put(sprintf_str("/SMask %d 0 R", this->n + 1));
    _put(sprintf_str("/Length %d>>", (int)info.data.size()));
    _putstream(info.data);
    _put("endobj");

    if (!info.smask.empty()) {
        ImageInfo smask_info;
        smask_info.w = info.w;
        smask_info.h = info.h;
        smask_info.cs = "DeviceGray";
        smask_info.bpc = 8;
        smask_info.f = info.f;
        smask_info.dp = "/Predictor 15 /Colors 1 /BitsPerComponent 8 /Columns " + std::to_string(info.w);
        smask_info.data = info.smask;
        _putimage(smask_info);
    }

    if (info.cs == "Indexed") {
        _putstreamobject(info.pal);
    }
}

void webpdf::_putxobjectdict() {
    _put("/XObject <<");
    for (auto& kv : images) {
        ImageInfo& info = kv.second;
        _put(sprintf_str("/I%d %d 0 R", info.i, info.n));
    }
    _put(">>");
}

void webpdf::_putinfo() {
    char date_buf[32];
    struct tm* tm_info = gmtime(&CreationDate);
    strftime(date_buf, sizeof(date_buf), "%Y%m%d%H%M%S", tm_info);
    std::string date = "D:" + std::string(date_buf) + "+00'00'";
    metadata["CreationDate"] = date;
    if (metadata.find("Producer") == metadata.end())
        metadata["Producer"] = "webpdf " + std::string(WEBPDF_VERSION);
    
    if (metadata.find("Producer") != metadata.end())
        _put("/Producer " + _textstring(metadata["Producer"]));
    for (auto& kv : metadata) {
        if (kv.first != "Producer")
            _put("/" + kv.first + " " + _textstring(kv.second));
    }
}

void webpdf::_putcatalog() {
    int first_page_n = PageInfo_vec[0].n;
    _put("/Type /Catalog");
    _put("/Pages 1 0 R");
    if (!Outlines.empty())
        _put(sprintf_str("/Outlines %d 0 R", Outline_n));
    if (!ZoomMode.empty()) {
        if (ZoomMode == "fullpage")
            _put(sprintf_str("/OpenAction [%d 0 R /Fit]", first_page_n));
        else if (ZoomMode == "fullwidth")
            _put(sprintf_str("/OpenAction [%d 0 R /FitH null]", first_page_n));
        else if (ZoomMode == "real")
            _put(sprintf_str("/OpenAction [%d 0 R /XYZ null null 1]", first_page_n));
        else if (ZoomMode != "default") {
            try {
                double zoom = std::stod(ZoomMode) / 100.0;
                _put(sprintf_str("/OpenAction [%d 0 R /XYZ null null %.2f]", first_page_n, zoom));
            } catch (...) {}
        }
    }
    if (!LayoutMode.empty()) {
        if (LayoutMode == "single")
            _put("/PageLayout /SinglePage");
        else if (LayoutMode == "continuous")
            _put("/PageLayout /OneColumn");
        else if (LayoutMode == "two")
            _put("/PageLayout /TwoColumnLeft");
    }
}

void webpdf::_puttrailer() {
    _put("xref");
    _put(sprintf_str("0 %d", n + 1));
    _put("0000000000 65535 f ");
    for (int i = 1; i <= n; i++) {
        _put(sprintf_str("%010d 00000 n ", (int)offsets[i]));
    }
    _put("trailer");
    _put("<<");
    _put(sprintf_str("/Size %d", n + 1));
    _put(sprintf_str("/Root %d 0 R", Catalog_n));
    _put(sprintf_str("/Info %d 0 R", Info_n));
    _put(">>");
    _put("startxref");
    _put(sprintf_str("%d", (int)_getoffset()));
    _put("%%EOF");
}

std::string webpdf::Output(const std::string& dest, const std::string& name, bool /*isUTF8*/) {
    if (state < 3)
        Close();

    std::string d = _str_tolower(dest);
    std::string out_name = name;
    if (d.empty()) {
        if (out_name.empty()) {
            d = "i";
        } else {
            d = "f";
        }
    }

    if (d == "f" || d == "fd" || d == "fpdi") {
        std::string filename = out_name;
        if (filename.empty())
            filename = "doc.pdf";
        std::ofstream out(filename, std::ios::binary);
        if (!out.is_open())
            Error("Cannot open output file: " + filename);
        out.write(buffer.data(), buffer.size());
        out.close();
        return buffer;
    }

    if (d == "s") {
        return buffer;
    }

    return buffer;
}
std::string &&webpdf::move_result()
{
    return std::move(buffer);
}

int webpdf::AddLink() {
    links.push_back({0, 0.0});
    return (int)links.size();
}

void webpdf::SetLink(int link, double y, int link_page) {
    if (link < 1 || link > (int)links.size())
        return;
    if (link_page == -1)
        link_page = this->page;
    if (y == -1)
        y = yPos;
    links[link - 1] = {link_page, y};
}

void webpdf::AddOutline(const std::string& title, int page_no, int level) {
    OutlineEntry entry;
    entry.title = title;
    entry.page_no = page_no;
    entry.level = level;
    entry.parent = -1;
    entry.last_child = -1;
    entry.n = 0;
    Outlines.push_back(entry);
}

void webpdf::Link(double x, double y, double link_w, double link_h, const std::string& link) {
    if (page < 1)
        return;
    PageLink pl;
    pl.x = x;
    pl.y = y;
    pl.w = link_w;
    pl.h = link_h;
    pl.link_str = link;
    pl.is_uri = true;
    pl.link_int = 0;
    pl.annot_obj = 0;
    PageLinks[page - 1].push_back(pl);
}

void webpdf::Link(double x, double y, double link_w, double link_h, int link) {
    if (page < 1)
        return;
    PageLink pl;
    pl.x = x;
    pl.y = y;
    pl.w = link_w;
    pl.h = link_h;
    pl.link_int = link;
    pl.is_uri = false;
    pl.annot_obj = 0;
    PageLinks[page - 1].push_back(pl);
}

ImageInfo webpdf::_parsejpg(const std::string& file) {
    ImageInfo info;
    info.f = "DCTDecode";
    info.bpc = 8;

    std::string data = _read_file(file);
    const unsigned char* p = (const unsigned char*)data.data();
    size_t len = data.size();

    if (len < 2 || p[0] != 0xFF || p[1] != 0xD8)
        Error("Not a JPEG file: " + file);

    const int MAX_JPEG_DIM = 20000;
    size_t i = 2;
    bool found_sofn = false;
    while (i + 1 < len) {
        if (p[i] != 0xFF)
            Error("Invalid JPEG file");
        while (i < len && p[i] == 0xFF) i++;
        if (i >= len) break;
        unsigned char marker = p[i];
        i++;
        if (marker == 0xD9 || marker == 0xDA)
            break;
        if (i + 1 >= len) break;
        unsigned int seg_len = ((unsigned)p[i] << 8) | p[i+1];
        if (seg_len < 2) break;
        if (i + seg_len > len)
            Error("JPEG segment exceeds file size: " + file);
        if (marker == 0xC0 || marker == 0xC1 || marker == 0xC2) {
            if (seg_len >= 8) {
                info.h = ((unsigned)p[i+3] << 8) | p[i+4];
                info.w = ((unsigned)p[i+5] << 8) | p[i+6];
                if (info.w <= 0 || info.h <= 0 || info.w > MAX_JPEG_DIM || info.h > MAX_JPEG_DIM)
                    Error("JPEG dimensions out of range: " + file);
                int components = p[i+7];
                if (components == 1)
                    info.cs = "DeviceGray";
                else if (components == 3)
                    info.cs = "DeviceRGB";
                else if (components == 4)
                    info.cs = "DeviceCMYK";
                found_sofn = true;
            }
            break;
        }
        i += seg_len;
    }

    if (!found_sofn)
        Error("Invalid JPEG file: " + file);

    info.data = data;
    return info;
}

ImageInfo webpdf::_parsepng(const std::string& file) {
    ImageInfo info;

    std::string data = _read_file(file);
    const unsigned char* p = (const unsigned char*)data.data();
    size_t len = data.size();

    if (len < 8 || memcmp(p, "\x89PNG\r\n\x1a\n", 8) != 0)
        Error("Not a PNG file: " + file);

    size_t i = 8;
    bool found_ihdr = false;
    std::string idat_data;
    std::string plte_data;
    std::string trns_data;
    int color_type = 0;
    int bit_depth = 0;
    int width = 0;
    int height = 0;

    const int MAX_PNG_DIM = 20000;
    const size_t MAX_PNG_CHUNK = 100 * 1024 * 1024;

    while (i + 8 <= len) {
        uint32_t chunk_len_raw = (p[i] << 24) | (p[i+1] << 16) | (p[i+2] << 8) | p[i+3];
        size_t chunk_len = (size_t)chunk_len_raw;
        if (chunk_len > MAX_PNG_CHUNK)
            Error("PNG chunk too large: " + file);
        if (i + 8 + chunk_len + 4 > len)
            Error("PNG chunk exceeds file size: " + file);
        std::string chunk_type((char*)&p[i+4], 4);
        size_t data_start = i + 8;

        if (chunk_type == "IHDR") {
            if (chunk_len < 13)
                Error("Invalid PNG IHDR: " + file);
            width = (int)((unsigned)((p[data_start] << 24) | (p[data_start+1] << 16) | (p[data_start+2] << 8) | p[data_start+3]));
            height = (int)((unsigned)((p[data_start+4] << 24) | (p[data_start+5] << 16) | (p[data_start+6] << 8) | p[data_start+7]));
            if (width <= 0 || height <= 0 || width > MAX_PNG_DIM || height > MAX_PNG_DIM)
                Error("PNG dimensions out of range: " + file);
            bit_depth = p[data_start+8];
            color_type = p[data_start+9];
            info.w = width;
            info.h = height;
            info.bpc = bit_depth;
            found_ihdr = true;
        } else if (chunk_type == "IDAT") {
            idat_data.append((char*)&p[data_start], chunk_len);
        } else if (chunk_type == "PLTE") {
            plte_data.append((char*)&p[data_start], chunk_len);
        } else if (chunk_type == "tRNS") {
            trns_data.append((char*)&p[data_start], chunk_len);
        } else if (chunk_type == "IEND") {
            break;
        }

        i += 8 + chunk_len + 4;
    }

    if (!found_ihdr)
        Error("Invalid PNG file: " + file);

    std::string colspace;
    if (color_type == 0 || color_type == 4)
        colspace = "DeviceGray";
    else if (color_type == 2 || color_type == 6)
        colspace = "DeviceRGB";
    else if (color_type == 3)
        colspace = "Indexed";
    else
        Error("Unknown color type: " + file);

    info.cs = colspace;

    int colors = (colspace == "DeviceRGB") ? 3 : 1;
    info.dp = "/Predictor 15 /Colors " + std::to_string(colors) + " /BitsPerComponent " + std::to_string(bit_depth) + " /Columns " + std::to_string(width);

    if (colspace == "Indexed") {
        if (plte_data.empty())
            Error("Missing palette in " + file);
        info.pal = plte_data;
    }

    if (!trns_data.empty()) {
        if (color_type == 0) {
            if (trns_data.size() >= 2) {
                int gray = (unsigned char)trns_data[1];
                info.trns.push_back(gray);
            }
        } else if (color_type == 2) {
            if (trns_data.size() >= 6) {
                int r = (unsigned char)trns_data[1];
                int g = (unsigned char)trns_data[3];
                int b = (unsigned char)trns_data[5];
                info.trns.push_back(r);
                info.trns.push_back(g);
                info.trns.push_back(b);
            }
        } else if (color_type == 3) {
            size_t pos = trns_data.find((char)0);
            if (pos != std::string::npos) {
                info.trns.push_back((int)pos);
            }
        }
    }

    info.f = "FlateDecode";

    if (color_type >= 4) {
        int bytes_per_pixel = colors + 1;
        size_t max_decompressed = (size_t)(1 + width * bytes_per_pixel) * height + height * 10 + 1024;
        if (max_decompressed > 500 * 1024 * 1024)
            max_decompressed = 500 * 1024 * 1024;
        std::string decompressed = _uncompress_zlib(idat_data, max_decompressed);
        std::string color_data;
        std::string alpha_data;
        int row_len = 1 + bytes_per_pixel * width;
        size_t need = (size_t)row_len * height;
        if (decompressed.size() < need)
            Error("PNG decompressed data too short: " + file);
        for (int y = 0; y < height; y++) {
            int pos = y * row_len;
            color_data += decompressed[pos];
            alpha_data += decompressed[pos];
            for (int x = 0; x < width; x++) {
                int pixel_pos = pos + 1 + x * bytes_per_pixel;
                for (int c = 0; c < colors; c++) {
                    color_data += decompressed[pixel_pos + c];
                }
                alpha_data += decompressed[pixel_pos + colors];
            }
        }
        info.data = _compress_zlib(color_data);
        info.smask = _compress_zlib(alpha_data);
    } else {
        info.data = idat_data;
    }

    return info;
}

void webpdf::Image(const std::string& file, double x, double y, double iw, double ih, const std::string& type, const std::string& link) {
    if (!validate_path(file))
        Error("Invalid image path: " + file);
    
    std::string key = file;
    if (images.find(key) == images.end()) {
        std::string t = type;
        if (t.empty()) {
            size_t dot = file.find_last_of('.');
            if (dot == std::string::npos)
                Error("Missing image extension: " + file);
            t = file.substr(dot + 1);
        }
        t = _str_tolower(t);
        ImageInfo info;
        std::string real_path = _resolve_image_path(file);
        if (t == "jpg" || t == "jpeg")
            info = _parsejpg(real_path);
        else if (t == "png")
            info = _parsepng(real_path);
        else if (t == "svg")
            Error("SVG images not yet supported via Image(): " + file);
        else
            Error("Unsupported image type: " + t);
        info.i = (int)images.size() + 1;
        images[key] = info;
    }

    ImageInfo& info = images[key];
    if (iw == 0 && ih == 0) {
        iw = info.w / k;
        ih = info.h / k;
    } else if (iw == 0) {
        iw = ih * info.w / info.h;
    } else if (ih == 0) {
        ih = iw * info.h / info.w;
    }

    if (x == -1)
        x = xPos;
    if (y == -1)
        y = yPos;

    _out(sprintf_str("q %.2f 0 0 %.2f %.2f %.2f cm /I%d Do Q",
        iw * k, ih * k, x * k, (this->h - (y + ih)) * k, info.i));

    if (!link.empty())
        Link(x, y, iw, ih, link);
}

bool webpdf::GetImageSize(const std::string& file, double& out_w, double& out_h) {
    if (!validate_path(file))
        return false;
    
    std::string key = file;
    if (images.find(key) == images.end()) {
        std::string t;
        size_t dot = file.find_last_of('.');
        if (dot == std::string::npos)
            return false;
        t = file.substr(dot + 1);
        t = _str_tolower(t);
        ImageInfo info;
        std::string real_path = _resolve_image_path(file);
        if (t == "jpg" || t == "jpeg")
            info = _parsejpg(real_path);
        else if (t == "png")
            info = _parsepng(real_path);
        else
            return false;
        info.i = (int)images.size() + 1;
        images[key] = info;
    }
    ImageInfo& info = images[key];
    out_w = info.w / k;
    out_h = info.h / k;
    return true;
}

void webpdf::ImageRaw(const std::string& file, double x, double y, double iw, double ih) {
    if (!validate_path(file))
        return;
    std::string key = file;
    if (images.find(key) == images.end()) {
        double gw, gh;
        if (!GetImageSize(file, gw, gh))
            return;
    }
    ImageInfo& info = images[key];
    _out(sprintf_str("%.2f 0 0 %.2f %.2f %.2f cm /I%d Do",
        iw, -ih, x, y + ih, info.i));
}

int webpdf::AddAxialShading(double x0, double y0, double x1, double y1, const std::vector<GradientStop>& stops) {
    AxialShadingInfo info;
    info.i = shading_count++;
    info.x0 = x0;
    info.y0 = y0;
    info.x1 = x1;
    info.y1 = y1;
    info.is_radial = false;
    info.stops = stops;
    shadings.push_back(info);
    return info.i;
}

int webpdf::AddRadialShading(double x0, double y0, double r0, double x1, double y1, double r1, const std::vector<GradientStop>& stops) {
    AxialShadingInfo info;
    info.i = shading_count++;
    info.x0 = x0;
    info.y0 = y0;
    info.x1 = x1;
    info.y1 = y1;
    info.is_radial = true;
    info.r0 = r0;
    info.r1 = r1;
    info.stops = stops;
    shadings.push_back(info);
    return info.i;
}

void webpdf::ShadeFill(int shading_id) {
    if (shading_id < 0 || shading_id >= (int)shadings.size())
        return;
    _out(sprintf_str("/Sh%d sh", shading_id));
}

}

