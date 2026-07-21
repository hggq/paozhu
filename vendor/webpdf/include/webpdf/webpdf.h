#ifndef WEBPDF_H
#define WEBPDF_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdint>
#include <memory>
#include <functional>
#include "request.h"

namespace pz {

class HTMLRenderer;

class WebPDFException : public std::runtime_error {
public:
    explicit WebPDFException(const std::string& msg) : std::runtime_error("webpdf error: " + msg) {}
};

struct UvEntry {
    bool is_range;
    int value;
    int count;
    UvEntry() : is_range(false), value(0), count(0) {}
    UvEntry(int v) : is_range(false), value(v), count(0) {}
    UvEntry(int v, int c) : is_range(true), value(v), count(c) {}
};

struct FontInfo {
    int i = 0;
    std::string type;
    std::string name;
    int up = -100;
    int ut = 50;
    std::vector<int> cw;
    std::map<char, int> cw_map;
    std::map<int, int> cw_uni;
    std::map<int, uint16_t> cidToGid;
    std::string enc;
    std::map<int, UvEntry> uv;
    std::string file;
    int originalsize = 0;
    int size1 = 0;
    int size2 = 0;
    std::string diff;
    std::map<std::string, std::string> desc;
    bool subsetted = false;
    bool uni = false;
    std::string ttffile;
    std::map<int, bool> subset;
    int n = 0;
};

struct FontFileInfo {
    int length1 = 0;
    int length2 = 0;
    int n = 0;
};

struct ImageInfo {
    int i = 0;
    int w = 0;
    int h = 0;
    std::string cs;
    int bpc = 0;
    std::string f;
    std::string dp;
    std::string data;
    std::string pal;
    std::vector<int> trns;
    std::string smask;
    int n = 0;
};

struct GradientStop {
    double offset;
    double r, g, b;
    double opacity;
};

struct AxialShadingInfo {
    int i = 0;
    int n = 0;
    double x0, y0, x1, y1;
    bool is_radial = false;
    double r0 = 0, r1 = 0; // radii for radial (Type 3) shading
    std::vector<GradientStop> stops;
};

struct PageLink {
    double x;
    double y;
    double w;
    double h;
    std::string link_str;
    int link_int;
    bool is_uri;
    int annot_obj = 0;
};

struct PageInfo {
    double size_w = 0;
    double size_h = 0;
    bool has_size = false;
    int rotation = 0;
    bool has_rotation = false;
    int n = 0;
};

struct OutlineEntry {
    std::string title;
    int page_no;      // 1-based page number
    int level;        // 0 = top level, 1 = child, etc.
    int parent;       // index of parent entry (-1 for root)
    int last_child;   // index of last child (-1 if none)
    int n;            // PDF object number
};

class webpdf {
public:
    static constexpr const char* VERSION = "1.9";

    webpdf(const std::string& orientation = "P", const std::string& unit = "mm", const std::string& size = "A4");
    virtual ~webpdf() = default;

    void SetMargins(double left, double top, double right = -1);
    void SetLeftMargin(double margin);
    void SetTopMargin(double margin);
    void SetRightMargin(double margin);
    void SetAutoPageBreak(bool auto_break, double margin = 0);
    void SetDisplayMode(const std::string& zoom, const std::string& layout = "default");
    void SetCompression(bool compress);
    void SetTitle(const std::string& title, bool isUTF8 = false);
    void SetAuthor(const std::string& author, bool isUTF8 = false);
    void SetSubject(const std::string& subject, bool isUTF8 = false);
    void SetKeywords(const std::string& keywords, bool isUTF8 = false);
    void SetCreator(const std::string& creator, bool isUTF8 = false);
    void AliasNbPages(const std::string& alias = "{nb}");
    void Error(const std::string& msg);

    void Close();
    void AddPage(const std::string& orientation = "", const std::string& size = "", int rotation = 0);
    int PageNo();

    void SetPageNumberOffset(int offset);
    int GetPageNumberOffset();

    void SetShowHeaderFooter(bool show);
    bool GetShowHeaderFooter();
    void SetShowHeader(bool show);
    bool GetShowHeader();
    void SetShowFooter(bool show);
    bool GetShowFooter();

    void ClearPageContent();
    void ClearHeaderFromPage();

    // Fragment capture/replay for measuring content height
    void StartCapture();
    std::string StopCapture();
    void WriteCaptured(const std::string& fragment);

    // Dry-run mode: render to buffer to measure height, then flush to real stream
    void BeginDryRun();            // debugmodel=true, start intercepting _out
    double EndDryRunAndFlush();    // debugmodel=false, return measured height
    void FlushDryRunBuffer();      // write buffer_debug to real page stream
    bool DryRunHasPageBreaks();    // whether last dry-run spanned multiple pages
    int DryRunPageBreakCount();    // number of page breaks in last dry-run
    int DryRunPageCount();         // total number of pages in last dry-run
    double DryRunPageHeight(int page_idx);  // height of content on a given page
    std::string DryRunPageContent(int page_idx);  // content stream of a given page
    void BeginMeasure();           // measure_mode=true, discard all _out output
    double EndMeasure();           // measure_mode=false, return measured height

    virtual void Header();
    virtual void Footer();

    void SetDrawColor(int r, int g = -1, int b = -1);
    void SetFillColor(int r, int g = -1, int b = -1);
    void SetTextColor(int r, int g = -1, int b = -1);

    double GetStringWidth(const std::string& s);
    int GetCharWidthFU(uint32_t cp); // get single char width in font units (1/1000 em)
    void SetLineWidth(double width);
    void SetDash(const std::string& pattern, double phase = 0);
    void Line(double x1, double y1, double x2, double y2);
    void Rect(double x, double y, double w, double h, const std::string& style = "");

    void AddFont(const std::string& family, const std::string& style = "", const std::string& file = "", const std::string& dir = "");
    void SetFont(const std::string& family, const std::string& style = "", double size = 0);
    void SetFontSize(double size);

    int AddLink();
    void SetLink(int link, double y = 0, int page = -1);
    void Link(double x, double y, double w, double h, const std::string& link);
    void Link(double x, double y, double w, double h, int link);

    void Text(double x, double y, const std::string& txt);
    virtual bool AcceptPageBreak();
    void Cell(double w, double h = 0, const std::string& txt = "", int border = 0, int ln = 0, const std::string& align = "", bool fill = false, const std::string& link = "");
    void Cell(double w, double h, const std::string& txt, const std::string& border, int ln = 0, const std::string& align = "", bool fill = false, const std::string& link = "");
    void MultiCell(double w, double h, const std::string& txt, int border = 0, const std::string& align = "J", bool fill = false);
    void MultiCell(double w, double h, const std::string& txt, const std::string& border, const std::string& align = "J", bool fill = false);
    int GetMultiCellLines(double w, const std::string& txt);
    int GetMultiCellCharCount(double w, const std::string& txt, int max_lines);
    std::string SplitTextByLines(double w, std::string& txt, int max_lines);
    void Write(double h, const std::string& txt, const std::string& link = "");
    void WriteHTML(const std::string& html, double h = 5);
    void Ln(double h = -1);

    void Image(const std::string& file, double x = -1, double y = -1, double w = 0, double h = 0, const std::string& type = "", const std::string& link = "");
    bool GetImageSize(const std::string& file, double& w, double& h);
    void ImageRaw(const std::string& file, double x, double y, double w, double h);

    int AddAxialShading(double x0, double y0, double x1, double y1, const std::vector<GradientStop>& stops);
    int AddRadialShading(double x0, double y0, double r0, double x1, double y1, double r1, const std::vector<GradientStop>& stops);
    void ShadeFill(int shading_id);

    double GetPageWidth();
    double GetPageHeight();
    double GetX();
    void SetX(double x);
    double GetY();
    void SetY(double y, bool resetX = true);
    void SetXY(double x, double y);

    std::string Output(const std::string& dest = "", const std::string& name = "", bool isUTF8 = false);

    void AddOutline(const std::string& title, int page_no, int level = 0);

    std::function<void()> page_header_cb;
    std::function<void()> page_footer_cb;

    void setImagesPath(const std::string& p);
    std::string &&move_result();
    std::string buffer;
protected:
    friend class HTMLRenderer;
    friend class SVGRenderer;
    int page;
    int page_no_offset = 0;
    bool show_header_footer = true;
    bool show_header = true;
    bool show_footer = true;
    int n;
    std::vector<size_t> offsets;
    
    std::vector<std::string> pages;
    int state;
    bool compress;
    double k;
    std::string DefOrientation;
    std::string CurOrientation;
    std::map<std::string, std::pair<double, double>> StdPageSizes;
    std::pair<double, double> DefPageSize;
    std::pair<double, double> CurPageSize;
    int CurRotation;
    std::vector<PageInfo> PageInfo_vec;
    std::vector<OutlineEntry> Outlines;
    int Outline_n = 0;
    int Catalog_n = 0; // object number for Catalog
    int Info_n = 0;    // object number for Info
    double wPt, hPt;
    double w, h;
    double lMargin;
    double tMargin;
    double rMargin;
    double bMargin;
    double cMargin;
    double xPos, yPos;
    double lasth;
    double LineWidth;
    std::string DashPattern;
    double DashPhase;
    std::string fontpath;
    std::string imagepath;
    std::vector<std::string> CoreFonts;
    std::map<std::string, FontInfo> fonts;
    std::map<std::string, FontFileInfo> FontFiles;
    std::map<std::string, int> encodings;
    std::map<std::string, int> cmaps;
    std::string FontFamily;
    // Last unicode-capable (composite) font family that was selected. Used as a
    // fallback when a requested CSS font-family is not registered, so CJK text
    // is never forced onto a Latin-only core font (which would emit raw UTF-8).
    std::string DefFontFamily;
    // Lazily built index of fontpath: lowercase filename stem -> real-case stem
    // (extension dropped). Populated once by _scan_font_dir() and used to match
    // a requested family against the actual TTF/OTF/JSON files on disk.
    bool _font_dir_scanned = false;
    std::map<std::string, std::string> _font_file_index;
    std::string FontStyle;
    bool underline;
    FontInfo* CurrentFont;
    double FontSizePt;
    double FontSize;
    int FontPage; // tracks which page the font Tf was last output on
    std::string DrawColor;
    std::string FillColor;
    std::string TextColor;
    bool ColorFlag;
    bool WithAlpha;
    double ws;
    std::map<std::string, ImageInfo> images;
    std::vector<AxialShadingInfo> shadings;
    int shading_count = 0;
    std::vector<std::vector<PageLink>> PageLinks;
    std::vector<std::pair<int, double>> links;
    bool AutoPageBreak;
    double PageBreakTrigger;
    bool InHeader;
    bool InFooter;
    size_t header_start_pos = 0;
    bool capture_mode = false;
    std::string capture_buf;
    struct DryRunPageInfo {
        std::string dryrun_buf;
        double height = 0;
    };
    struct DryRunFrame {
        std::string dryrun_buf;
        std::string direct_buf;
        double start_y = 0;
        bool saved_autopagebreak = true;
        enum Phase { DRYRUN, DIRECT } phase = DRYRUN;
        int page_break_count = 0;
        std::vector<DryRunPageInfo> pages;
        double last_page_start_y = 0;
    };
    std::vector<DryRunFrame> dryrun_stack_;
    bool debugmodel = false;
    std::string buffer_debug;
    double dryrun_start_y = 0;
    bool measure_mode_ = false;
    double measure_start_y_ = 0;
    std::string aliasNbPages;
    std::string ZoomMode;
    std::string LayoutMode;
    std::map<std::string, std::string> metadata;
    time_t CreationDate;
    std::string PDFVersion;

    std::pair<double, double> _getpagesize(const std::string& size);
    std::pair<double, double> _getpagesize(const std::pair<double, double>& size);
    void _beginpage(const std::string& orientation, const std::string& size, int rotation);
    void _endpage();
    FontInfo _loadfont(const std::string& path);
    FontInfo _loadjsonfont(const std::string& path);
    // Auto font registration. When SetFont is asked for a CSS font-family that
    // was never registered through AddFont, look for a matching file in
    // fontpath and register it on the fly by parsing the .ttf/.otf directly.
    // Glyph metrics are NOT persisted to JSON: dumping the per-Unicode width
    // and CID->GID maps (tens of thousands of entries for CJK) produces huge
    // files that are slower to parse back than re-reading the font binary.
    bool _try_auto_register(const std::string& family, const std::string& style);
    void _scan_font_dir();
    bool _isascii(const std::string& s);
    std::string _UTF8encode(const std::string& s);
    std::string _UTF8toUTF16(const std::string& s);
    std::string _escape(const std::string& s);
    std::string _textstring(const std::string& s);
    std::string _utf8_to_utf16be(const std::string& s);
    std::string _dounderline(double x, double y, const std::string& txt);

    ImageInfo _parsejpg(const std::string& file);
    ImageInfo _parsepng(const std::string& file);

    void _out(const std::string& s);
    void _put(const std::string& s);
    size_t _getoffset();
    void _newobj(int n = -1);
    void _putstream(const std::string& data);
    void _putstreamobject(const std::string& data);
    void _putlinks(int page_idx);
    void _putpage(int page_num);
    void _putpages();
    void _putoutlines();
    void _putfonts();
    std::string _tounicodecmap(const std::map<int, UvEntry>& uv);
    void _putimages();
    void _putimage(ImageInfo& info);
    void _putshadings();
    void _putshading(AxialShadingInfo& info);
    void _putxobjectdict();
    void _putresourcedict();
    void _putresources();
    void _putinfo();
    void _putcatalog();
    void _putheader();
    void _puttrailer();
    void _enddoc();

    std::string _compress_zlib(const std::string& data);
    std::string _uncompress_zlib(const std::string& data, size_t max_size = 0);
    std::string _read_file(const std::string& path);
    std::string _resolve_image_path(const std::string& file) const;
    std::string _format_double(double val, int precision = 2);
    std::string _str_tolower(const std::string& s);
    std::string _str_toupper(const std::string& s);
};

}

#endif
