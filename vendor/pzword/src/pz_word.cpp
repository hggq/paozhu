#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include "minizip/zip.h"
#include "minizip/unzip.h"
#include "pz_word.h"

namespace pz
{
static std::string html_escape(const std::string &s) {
    std::string result;
    result.reserve(s.size());
    for(char c : s) {
        switch(c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c;
        }
    }
    return result;
}

// 颜色/高亮等枚举型属性值清洗：只保留字母数字，
// 防止不可信输入携带 " < > & 破坏生成的 XML/HTML 属性
static std::string sanitize_token(const std::string &s)
{
    std::string result;
    result.reserve(s.size());
    for(char c : s)
    {
        if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            result += c;
    }
    return result;
}

// 把 [begin, end) 区间的 XML 文本追加到 out，还原常见实体
static void append_xml_text(std::string &out, const std::string &content, size_t begin, size_t end)
{
    for(size_t si = begin; si < end; si++)
    {
        if(content[si] == '&')
        {
            if(content.compare(si, 5, "&amp;") == 0) { out += '&'; si += 4; continue; }
            if(content.compare(si, 4, "&lt;") == 0) { out += '<'; si += 3; continue; }
            if(content.compare(si, 4, "&gt;") == 0) { out += '>'; si += 3; continue; }
            if(content.compare(si, 6, "&quot;") == 0) { out += '"'; si += 5; continue; }
            if(content.compare(si, 6, "&apos;") == 0) { out += '\''; si += 5; continue; }
        }
        out += content[si];
    }
}

// 从 pos 处的 '<' 开始扫描一个标签
// 返回标签名、是否闭合标签(</xxx>)、是否自闭合(<xxx/>)，after 为 '>' 之后的位置
// 属性值中的 '>' 会被引号保护，不会误判标签结束
static bool scan_tag(const std::string &content, size_t pos, size_t end,
                     std::string &name, bool &is_close, bool &is_self_close, size_t &after)
{
    if(pos >= end || content[pos] != '<') return false;
    size_t si = pos + 1;
    is_close = false;
    is_self_close = false;
    if(si < end && content[si] == '/')
    {
        is_close = true;
        si++;
    }
    size_t name_start = si;
    while(si < end)
    {
        char c = content[si];
        if(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '>' || c == '/')
        {
            break;
        }
        si++;
    }
    if(si == name_start) return false;
    name.assign(content, name_start, si - name_start);

    bool in_quote = false;
    char quote_char = 0;
    for(; si < end; si++)
    {
        char c = content[si];
        if(in_quote)
        {
            if(c == quote_char) in_quote = false;
            continue;
        }
        if(c == '"' || c == '\'')
        {
            in_quote = true;
            quote_char = c;
            continue;
        }
        if(c == '>')
        {
            if(content[si - 1] == '/') is_self_close = true;
            after = si + 1;
            return true;
        }
    }
    return false;
}

word::word()
{
}

void word::clear()
{
    contents.clear();
    error_msg.clear();
    default_style = WORD_RUN();
    table_styles.clear();
}

bool word::is_safe_zip_path(const std::string &path) const
{
    if(path.empty()) return false;
    if(path[0] == '/') return false;
    if(path.find("..") != std::string::npos) return false;
    if(path.size() >= 2 && path[1] == ':') return false;
    return true;
}

// 在 [si, end) 内找下一个子元素，跳过注释/声明
// 遇到上层的闭合标签(</xxx)或找不到元素时返回 false
// 找到后 tag 内记录开始标签、内容区间和整个元素结束位置，同名嵌套会被正确配对
bool word::next_element(const std::string &content, size_t si, size_t end, XML_TAG &tag) const
{
    while(si < end)
    {
        size_t lt = content.find('<', si);
        if(lt == std::string::npos || lt >= end) return false;

        if(content.compare(lt, 4, "<!--") == 0)
        {
            size_t e = content.find("-->", lt + 4);
            if(e == std::string::npos || e >= end) return false;
            si = e + 3;
            continue;
        }
        if(lt + 1 < end && (content[lt + 1] == '?' || content[lt + 1] == '!'))
        {
            size_t e = content.find('>', lt + 1);
            if(e == std::string::npos || e >= end) return false;
            si = e + 1;
            continue;
        }
        if(lt + 1 < end && content[lt + 1] == '/')
        {
            // 上层元素的闭合标签，本层没有更多子元素
            return false;
        }

        std::string name;
        bool is_close = false, is_self = false;
        size_t after = 0;
        if(!scan_tag(content, lt, end, name, is_close, is_self, after))
        {
            return false;
        }

        tag.name = std::move(name);
        tag.open_start = lt;
        tag.open_end = after;
        tag.self_closing = is_self;
        if(is_self)
        {
            tag.body_start = after;
            tag.body_end = after;
            tag.next = after;
            return true;
        }

        // 找同名闭合标签，同名嵌套时用深度配对
        int depth = 1;
        size_t pos = after;
        while(pos < end)
        {
            size_t p = content.find('<', pos);
            if(p == std::string::npos || p >= end) break;
            if(content.compare(p, 4, "<!--") == 0)
            {
                size_t e = content.find("-->", p + 4);
                if(e == std::string::npos || e >= end) break;
                pos = e + 3;
                continue;
            }
            std::string n2;
            bool c2 = false, s2 = false;
            size_t a2 = 0;
            if(!scan_tag(content, p, end, n2, c2, s2, a2))
            {
                pos = p + 1;
                continue;
            }
            if(n2 == tag.name && !s2)
            {
                if(c2)
                {
                    depth--;
                    if(depth == 0)
                    {
                        tag.body_start = tag.open_end;
                        tag.body_end = p;
                        tag.next = a2;
                        return true;
                    }
                }
                else
                {
                    depth++;
                }
            }
            pos = a2;
        }
        return false;
    }
    return false;
}

// 从开始标签中取属性值，如 get_tag_attr(content, tag, "w:val")
std::string word::get_tag_attr(const std::string &content, const XML_TAG &tag, std::string_view attr_name) const
{
    std::string pattern(attr_name);
    pattern += "=\"";
    size_t p = content.find(pattern, tag.open_start);
    while(p != std::string::npos && p + pattern.size() < tag.open_end)
    {
        char prev = (p > 0) ? content[p - 1] : ' ';
        if(prev == ' ' || prev == '\t' || prev == '\r' || prev == '\n')
        {
            size_t vs = p + pattern.size();
            size_t ve = content.find('"', vs);
            if(ve == std::string::npos || ve >= tag.open_end) return "";
            return content.substr(vs, ve - vs);
        }
        p = content.find(pattern, p + 1);
    }
    return "";
}

void word::process_document(const std::string &file_content)
{
    // 找根元素 <w:document>
    XML_TAG root;
    size_t si = 0;
    bool found = false;
    while(next_element(file_content, si, file_content.size(), root))
    {
        if(root.name == "w:document")
        {
            found = true;
            break;
        }
        si = root.next;
    }
    if(!found)
    {
        error_msg = "Error: <w:document> not found";
        return;
    }

    // 在根元素里找 <w:body>
    XML_TAG body;
    si = root.body_start;
    found = false;
    while(next_element(file_content, si, root.body_end, body))
    {
        if(body.name == "w:body")
        {
            found = true;
            break;
        }
        si = body.next;
    }
    if(!found)
    {
        error_msg = "Error: <w:body> not found";
        return;
    }

    parse_body(file_content, body.body_start, body.body_end);
}

// 解析 word/styles.xml 里的表格样式（w:type="table"），
// document.xml 只引用 styleId，边框/表头/条纹颜色都定义在这里
void word::parse_styles(const std::string &file_content)
{
    XML_TAG root;
    size_t si = 0;
    bool found = false;
    while(next_element(file_content, si, file_content.size(), root))
    {
        if(root.name == "w:styles")
        {
            found = true;
            break;
        }
        si = root.next;
    }
    if(!found) return;

    XML_TAG style;
    si = root.body_start;
    while(next_element(file_content, si, root.body_end, style))
    {
        si = style.next;
        if(style.name != "w:style") continue;
        if(get_tag_attr(file_content, style, "w:type") != "table") continue;
        std::string id = get_tag_attr(file_content, style, "w:styleId");
        if(id.empty()) continue;

        WORD_TABLE_STYLE ts;
        XML_TAG tag;
        size_t pi = style.body_start;
        while(next_element(file_content, pi, style.body_end, tag))
        {
            pi = tag.next;
            if(tag.name == "w:tblPr")
            {
                // 表级边框：取第一个有颜色的边框
                XML_TAG prop;
                size_t bi = tag.body_start;
                while(next_element(file_content, bi, tag.body_end, prop))
                {
                    bi = prop.next;
                    if(prop.name != "w:tblBorders") continue;
                    XML_TAG border;
                    size_t bj = prop.body_start;
                    while(next_element(file_content, bj, prop.body_end, border))
                    {
                        bj = border.next;
                        std::string color = get_tag_attr(file_content, border, "w:color");
                        if(ts.border_color.empty() && !color.empty() && color != "auto")
                            ts.border_color = color;
                        std::string sz = get_tag_attr(file_content, border, "w:sz");
                        unsigned int s = 0;
                        for(char c : sz) if(c >= '0' && c <= '9') s = s * 10 + (c - '0');
                        // sz 单位 1/8 pt，转 px，至少 1
                        if(s > 0 && ts.border_size == 0) ts.border_size = (s / 8 > 0) ? s / 8 : 1;
                    }
                }
            }
            else if(tag.name == "w:tblStylePr")
            {
                std::string type = get_tag_attr(file_content, tag, "w:type");
                if(type != "firstRow" && type != "band1Horz") continue;

                XML_TAG sub;
                size_t sj = tag.body_start;
                while(next_element(file_content, sj, tag.body_end, sub))
                {
                    sj = sub.next;
                    if(sub.name == "w:tcPr")
                    {
                        // 单元格背景色
                        XML_TAG shd;
                        size_t k = sub.body_start;
                        while(next_element(file_content, k, sub.body_end, shd))
                        {
                            k = shd.next;
                            if(shd.name != "w:shd") continue;
                            std::string fill = get_tag_attr(file_content, shd, "w:fill");
                            if(fill.empty() || fill == "auto") continue;
                            if(type == "firstRow") ts.first_row_fill = fill;
                            else ts.band_fill = fill;
                        }
                    }
                    else if(sub.name == "w:rPr" && type == "firstRow")
                    {
                        // 表头行文字颜色
                        XML_TAG clr;
                        size_t k = sub.body_start;
                        while(next_element(file_content, k, sub.body_end, clr))
                        {
                            k = clr.next;
                            if(clr.name != "w:color") continue;
                            std::string v = get_tag_attr(file_content, clr, "w:val");
                            if(!v.empty() && v != "auto") ts.first_row_text_color = v;
                        }
                    }
                }
            }
        }
        table_styles[id] = std::move(ts);
    }
}

// 递归解析 body 级内容：段落、表格、内容控件
void word::parse_body(const std::string &content, size_t begin, size_t end, int depth)
{
    if(depth > MAX_PARSE_DEPTH) return;  // 防止恶意深度嵌套导致栈溢出
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:p")
        {
            WORD_PARAGRAPH para;
            parse_paragraph(content, tag.body_start, tag.body_end, para);
            if(!para.runs.empty())
            {
                WORD_CONTENT c;
                c.type = WORD_CONTENT_TYPE::PARAGRAPH;
                c.paragraph = std::move(para);
                contents.push_back(std::move(c));
            }
        }
        else if(tag.name == "w:tbl")
        {
            WORD_TABLE table;
            parse_table(content, tag.body_start, tag.body_end, table);
            if(!table.rows.empty())
            {
                WORD_CONTENT c;
                c.type = WORD_CONTENT_TYPE::TABLE;
                c.table = std::move(table);
                contents.push_back(std::move(c));
            }
        }
        else if(tag.name == "w:sdt" || tag.name == "w:sdtContent")
        {
            // 内容控件，递归进入内部继续找段落/表格
            parse_body(content, tag.body_start, tag.body_end, depth + 1);
        }
    }
}

// 递归解析段落：属性、文本 run、超链接等容器
void word::parse_paragraph(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, int depth)
{
    if(depth > MAX_PARSE_DEPTH) return;  // 防止恶意深度嵌套导致栈溢出
    XML_TAG tag;
    size_t si = begin;
    WORD_RUN default_run;  // 段落级别的默认 run 格式
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:pPr")
        {
            parse_paragraph_properties(content, tag.body_start, tag.body_end, para, default_run);
        }
        else if(tag.name == "w:r")
        {
            parse_run(content, tag.body_start, tag.body_end, para, default_run);
        }
        else if(tag.name == "w:hyperlink" || tag.name == "w:ins" ||
                tag.name == "w:smartTag" || tag.name == "w:sdt" || tag.name == "w:sdtContent")
        {
            // 这些容器里还是 run，递归处理
            parse_paragraph(content, tag.body_start, tag.body_end, para, depth + 1);
        }
    }
}

void word::parse_paragraph_properties(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, WORD_RUN &default_run)
{
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:pStyle")
        {
            // 样式 id 形如 "Heading1"，中文版 Word 的标题样式 id 是纯数字 "1"~"9"
            std::string val = get_tag_attr(content, tag, "w:val");
            size_t di = 0;
            if(val.size() > 7 && (val.compare(0, 7, "Heading") == 0 || val.compare(0, 7, "heading") == 0))
            {
                di = 7;
            }
            else if(val.empty() || val[0] < '1' || val[0] > '9')
            {
                continue;
            }
            unsigned int level = 0;
            bool valid = true;
            for(; di < val.size(); di++)
            {
                if(val[di] >= '0' && val[di] <= '9')
                {
                    level = level * 10 + (val[di] - '0');
                }
                else if(val[di] != ' ')
                {
                    valid = false;
                    break;
                }
            }
            if(valid && level > 0 && level <= MAX_HEADING_LEVEL)
            {
                para.heading_level = static_cast<unsigned char>(level);
            }
        }
        else if(tag.name == "w:rPr")
        {
            // 段落标记的默认 run 属性
            parse_run_properties(content, tag.body_start, tag.body_end, default_run);
        }
    }
}

void word::parse_run_properties(const std::string &content, size_t begin, size_t end, WORD_RUN &run)
{
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:b" || tag.name == "w:i" || tag.name == "w:u")
        {
            // <w:b/> 表示开启，w:val="0"/"false"/"none" 表示关闭
            std::string val = get_tag_attr(content, tag, "w:val");
            bool on = val.empty() || (val != "0" && val != "false" && val != "none");
            if(tag.name == "w:b") run.bold = on;
            else if(tag.name == "w:i") run.italic = on;
            else run.underline = on;
        }
        else if(tag.name == "w:sz")
        {
            // w:sz 的值是半磅
            std::string val = get_tag_attr(content, tag, "w:val");
            unsigned int size = 0;
            for(char c : val)
            {
                if(c >= '0' && c <= '9') size = size * 10 + (c - '0');
            }
            size /= 2;
            if(size > 0 && size < 256) run.font_size = static_cast<unsigned char>(size);
        }
        else if(tag.name == "w:rFonts")
        {
            if(run.font_name.empty())
            {
                std::string val = get_tag_attr(content, tag, "w:eastAsia");
                if(val.empty()) val = get_tag_attr(content, tag, "w:ascii");
                run.font_name = val;
            }
        }
        else if(tag.name == "w:color")
        {
            run.color = get_tag_attr(content, tag, "w:val");
        }
        else if(tag.name == "w:highlight")
        {
            run.highlight = get_tag_attr(content, tag, "w:val");
        }
        else if(tag.name == "w:shd")
        {
            // 文字底纹（Pages 用 w:shd 表示背景色，Word 用 w:highlight）
            std::string fill = get_tag_attr(content, tag, "w:fill");
            if(!fill.empty() && fill != "auto") run.highlight = fill;
        }
    }
}

// 解析 run：文本、制表符、换行
void word::parse_run(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, WORD_RUN &default_run)
{
    WORD_RUN run = default_run;  // 继承段落默认格式
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:rPr")
        {
            parse_run_properties(content, tag.body_start, tag.body_end, run);
        }
        else if(tag.name == "w:t")
        {
            append_xml_text(run.text, content, tag.body_start, tag.body_end);
        }
        else if(tag.name == "w:tab")
        {
            run.text.push_back('\t');
        }
        else if(tag.name == "w:br" || tag.name == "w:cr")
        {
            run.text.push_back('\n');
        }
    }
    if(!run.text.empty())
    {
        para.runs.push_back(std::move(run));
    }
}

// 递归解析表格，处理 gridSpan(横向合并) 和 vMerge(纵向合并)
void word::parse_table(const std::string &content, size_t begin, size_t end, WORD_TABLE &table)
{
    // 每个网格列当前纵向合并的归属单元格 (行下标, 单元格下标)
    std::vector<std::pair<size_t, size_t>> vmerge_owner;

    // 表格样式引用和条件格式开关（w:tblLook）
    std::string style_id;
    bool look_first_row = true;
    bool look_h_band = true;

    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:tblPr")
        {
            // 解析表格属性：宽度、边框
            XML_TAG prop;
            size_t pi = tag.body_start;
            while(next_element(content, pi, tag.body_end, prop))
            {
                pi = prop.next;
                if(prop.name == "w:tblW")
                {
                    std::string val = get_tag_attr(content, prop, "w:w");
                    std::string type = get_tag_attr(content, prop, "w:type");
                    unsigned int w = 0;
                    for(char c : val) if(c >= '0' && c <= '9') w = w * 10 + (c - '0');
                    // dxa 转 px（1px ≈ 15dxa），pct 是百分比（按 5000 = 100%），auto 忽略
                    if(type == "dxa" && w > 0) table.width = w / 15;
                    else if(type == "pct") table.width = w * 0; // 百分比暂不处理，设 0 表示 auto
                }
                else if(prop.name == "w:tblStyle")
                {
                    style_id = get_tag_attr(content, prop, "w:val");
                }
                else if(prop.name == "w:tblLook")
                {
                    // 新格式用独立属性，旧格式只有 w:val 十六进制位掩码
                    std::string fr = get_tag_attr(content, prop, "w:firstRow");
                    std::string nhb = get_tag_attr(content, prop, "w:noHBand");
                    if(!fr.empty()) look_first_row = (fr != "0" && fr != "false");
                    if(!nhb.empty()) look_h_band = (nhb == "0" || nhb == "false");
                    if(fr.empty() && nhb.empty())
                    {
                        std::string v = get_tag_attr(content, prop, "w:val");
                        unsigned int mask = 0;
                        for(char c : v)
                        {
                            if(c >= '0' && c <= '9') mask = mask * 16 + (c - '0');
                            else if(c >= 'a' && c <= 'f') mask = mask * 16 + (c - 'a' + 10);
                            else if(c >= 'A' && c <= 'F') mask = mask * 16 + (c - 'A' + 10);
                        }
                        if(!v.empty())
                        {
                            look_first_row = (mask & 0x0020) != 0;
                            look_h_band = (mask & 0x0200) == 0;
                        }
                    }
                }
                else if(prop.name == "w:tblBorders")
                {
                    // 取第一个有颜色的边框作为表格边框颜色
                    XML_TAG border;
                    size_t bi = prop.body_start;
                    while(next_element(content, bi, prop.body_end, border))
                    {
                        bi = border.next;
                        std::string border_color = get_tag_attr(content, border, "w:color");
                        std::string border_sz = get_tag_attr(content, border, "w:sz");
                        if(!border_color.empty() && border_color != "auto")
                        {
                            table.border_color = border_color;
                        }
                        if(!border_sz.empty())
                        {
                            unsigned int sz = 0;
                            for(char c : border_sz) if(c >= '0' && c <= '9') sz = sz * 10 + (c - '0');
                            // sz 单位是 1/8 pt，转换为 px
                            if(sz > 0 && table.border_size == 0) table.border_size = sz / 8;
                            if(table.border_size == 0) table.border_size = 1;
                        }
                    }
                    if(!table.border_color.empty() && table.border_size == 0) table.border_size = 1;
                }
            }
            continue;
        }
        if(tag.name != "w:tr") continue;

        WORD_TABLE_ROW row;
        std::vector<bool> vmerge_flags;
        parse_table_row(content, tag.body_start, tag.body_end, row, vmerge_flags);
        if(row.cells.empty()) continue;

        // vMerge 续接的单元格并入上方单元格(row_span+1)，其余保留
        WORD_TABLE_ROW merged_row;
        size_t grid_col = 0;
        for(size_t ci = 0; ci < row.cells.size(); ci++)
        {
            size_t span = row.cells[ci].col_span > 0 ? row.cells[ci].col_span : 1;
            bool merged_up = false;
            if(ci < vmerge_flags.size() && vmerge_flags[ci] && grid_col < vmerge_owner.size())
            {
                auto owner = vmerge_owner[grid_col];
                if(owner.first < table.rows.size() && owner.second < table.rows[owner.first].cells.size())
                {
                    table.rows[owner.first].cells[owner.second].row_span++;
                    merged_up = true;
                }
            }
            if(!merged_up)
            {
                if(vmerge_owner.size() < grid_col + span)
                {
                    vmerge_owner.resize(grid_col + span, {SIZE_MAX, SIZE_MAX});
                }
                for(size_t g = grid_col; g < grid_col + span; g++)
                {
                    vmerge_owner[g] = {table.rows.size(), merged_row.cells.size()};
                }
                merged_row.cells.push_back(std::move(row.cells[ci]));
            }
            grid_col += span;
        }
        if(grid_col > table.cols) table.cols = static_cast<unsigned int>(grid_col);
        if(!merged_row.cells.empty()) table.rows.push_back(std::move(merged_row));
    }

    // 套用表格样式：document.xml 里只有 <w:tblStyle w:val="..."/> 引用，
    // 边框/表头/条纹颜色定义在 styles.xml，直接格式优先于样式
    if(!style_id.empty())
    {
        auto it = table_styles.find(style_id);
        if(it != table_styles.end())
        {
            const WORD_TABLE_STYLE &ts = it->second;
            if(table.border_color.empty() && !ts.border_color.empty())
            {
                table.border_color = ts.border_color;
                if(table.border_size == 0)
                    table.border_size = (ts.border_size > 0) ? ts.border_size : 1;
            }
            for(size_t ri = 0; ri < table.rows.size(); ri++)
            {
                bool header = look_first_row && ri == 0;
                for(auto &cell : table.rows[ri].cells)
                {
                    if(header)
                    {
                        if(cell.bg_color.empty() && !ts.first_row_fill.empty())
                            cell.bg_color = ts.first_row_fill;
                        if(cell.text_color.empty() && !ts.first_row_text_color.empty())
                            cell.text_color = ts.first_row_text_color;
                    }
                    else if(look_h_band && !ts.band_fill.empty())
                    {
                        // 表头之后的第 1、3、5… 个数据行是 band1（隔行条纹）
                        size_t data_ri = ri - (look_first_row ? 1 : 0);
                        if(data_ri % 2 == 0 && cell.bg_color.empty())
                            cell.bg_color = ts.band_fill;
                    }
                }
            }
        }
    }
}

void word::parse_table_row(const std::string &content, size_t begin, size_t end, WORD_TABLE_ROW &row, std::vector<bool> &vmerge_flags)
{
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name != "w:tc") continue;

        WORD_TABLE_CELL cell;
        bool vmerge_continue = false;
        parse_table_cell(content, tag.body_start, tag.body_end, cell, vmerge_continue);
        row.cells.push_back(std::move(cell));
        vmerge_flags.push_back(vmerge_continue);
    }
}

void word::parse_table_cell(const std::string &content, size_t begin, size_t end, WORD_TABLE_CELL &cell, bool &vmerge_continue)
{
    vmerge_continue = false;
    XML_TAG tag;
    size_t si = begin;
    while(next_element(content, si, end, tag))
    {
        si = tag.next;
        if(tag.name == "w:tcPr")
        {
            XML_TAG prop;
            size_t pi = tag.body_start;
            while(next_element(content, pi, tag.body_end, prop))
            {
                pi = prop.next;
                if(prop.name == "w:gridSpan")
                {
                    std::string val = get_tag_attr(content, prop, "w:val");
                    unsigned int span = 0;
                    for(char c : val)
                    {
                        if(c >= '0' && c <= '9') span = span * 10 + (c - '0');
                    }
                    if(span > 1 && span < 256) cell.col_span = static_cast<unsigned char>(span);
                }
                else if(prop.name == "w:vMerge")
                {
                    // 无 w:val 或 w:val="continue" 表示续接上一行
                    std::string val = get_tag_attr(content, prop, "w:val");
                    if(val != "restart") vmerge_continue = true;
                }
                else if(prop.name == "w:shd")
                {
                    // 单元格背景色 w:fill
                    std::string fill = get_tag_attr(content, prop, "w:fill");
                    if(!fill.empty() && fill != "auto") cell.bg_color = fill;
                }
                else if(prop.name == "w:tcW")
                {
                    // 单元格宽度
                    std::string val = get_tag_attr(content, prop, "w:w");
                    std::string type = get_tag_attr(content, prop, "w:type");
                    unsigned int w = 0;
                    for(char c : val) if(c >= '0' && c <= '9') w = w * 10 + (c - '0');
                    if(type == "dxa" && w > 0) cell.width = w / 15;
                }
                else if(prop.name == "w:tcMar")
                {
                    // 单元格内边距，取四个方向的平均值
                    XML_TAG margin;
                    size_t mi = prop.body_start;
                    unsigned int total_pad = 0, pad_count = 0;
                    while(next_element(content, mi, prop.body_end, margin))
                    {
                        mi = margin.next;
                        std::string val = get_tag_attr(content, margin, "w:w");
                        unsigned int w = 0;
                        for(char c : val) if(c >= '0' && c <= '9') w = w * 10 + (c - '0');
                        if(w > 0) { total_pad += w / 15; pad_count++; }
                    }
                    if(pad_count > 0) cell.padding = total_pad / pad_count;
                }
            }
        }
        else if(tag.name == "w:p")
        {
            WORD_PARAGRAPH para;
            WORD_RUN default_run;
            parse_paragraph(content, tag.body_start, tag.body_end, para);
            if(!para.runs.empty())
            {
                if(!cell.text.empty()) cell.text += '\n';
                for(const auto &run : para.runs)
                {
                    cell.text += run.text;
                }
                // 提取第一个有颜色的 run 的文字颜色作为单元格颜色
                if(cell.text_color.empty())
                {
                    for(const auto &run : para.runs)
                    {
                        if(!run.color.empty()) { cell.text_color = run.color; break; }
                    }
                }
            }
        }
    }
}

bool word::read(const std::string &zipfilename)
{
    clear();
    error_msg.clear();
    
    unzFile uf = unzOpen(zipfilename.c_str());
    if (uf == NULL) {
        error_msg = "Error: Unable to open zip file ";
        error_msg.append(zipfilename);
        return false;
    }
    
    unz_global_info globalInfo;
    if (unzGetGlobalInfo(uf, &globalInfo) != UNZ_OK) {
        error_msg = "Error: Unable to get global info for ";
        error_msg.append(zipfilename);
        unzClose(uf);
        return false;
    }
    
    if (unzGoToFirstFile(uf) != UNZ_OK) {
        error_msg = "Error: Unable to go to first file in ";
        error_msg.append(zipfilename);
        unzClose(uf);
        return false;
    }
    
    std::string tempname;
    std::string file_content;
    std::string document_xml;
    std::string styles_xml;
    size_t zip_entry_count = 0;
    
    do {
        if(zip_entry_count >= MAX_ZIP_ENTRIES)
        {
            break;
        }
        zip_entry_count++;
        
        tempname.clear();
        char buffer[4096];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(uf, &fileInfo, buffer, sizeof(buffer), NULL, 0, NULL, 0) != UNZ_OK) {
            error_msg = "Error: Unable to get current file info in ";
            error_msg.append(zipfilename);
            break;
        }
        // 注意：文件名长度 >= 缓冲区时 minizip 不会写 '\0' 终止符，
        // 必须按 size_filename 截取，不能当 C 字符串用（否则越界读）
        size_t name_len = fileInfo.size_filename;
        if(name_len > sizeof(buffer)) name_len = sizeof(buffer);
        tempname.assign(buffer, name_len);
        
        if(!is_safe_zip_path(tempname))
        {
            unzCloseCurrentFile(uf);
            continue;
        }
        
        if (unzOpenCurrentFile(uf) != UNZ_OK) {
            error_msg = "Error: Unable to open current file in ";
            error_msg.append(zipfilename);
            break;
        }
        
        int bytesRead;
        file_content.clear();
        do {
            bytesRead = unzReadCurrentFile(uf, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                if(file_content.size() + bytesRead > MAX_FILE_SIZE)
                {
                    error_msg = "Error: File size exceeds maximum limit";
                    unzCloseCurrentFile(uf);
                    unzClose(uf);
                    return false;
                }
                file_content.append(buffer, bytesRead);
            }
        } while (bytesRead > 0);
        unzCloseCurrentFile(uf);
        
        if(tempname == "word/document.xml")
        {
            document_xml = std::move(file_content);
            file_content.clear();
        }
        else if(tempname == "word/styles.xml")
        {
            styles_xml = std::move(file_content);
            file_content.clear();
        }
        
    } while (unzGoToNextFile(uf) == UNZ_OK);
    
    unzClose(uf);

    // 先解析表格样式再解析正文（zip 里条目顺序不定，不能依赖出现顺序）
    if(!styles_xml.empty()) parse_styles(styles_xml);
    if(!document_xml.empty()) process_document(document_xml);
    return true;
}

bool word::read_from_unzipped(const std::string &base_path)
{
    clear();
    error_msg.clear();
    
    std::string dir = base_path;
    if(!dir.empty() && dir.back() != '/')
    {
        dir += "/";
    }

    // styles.xml 可选，有就先解析表格样式
    {
        std::ifstream sf(dir + "word/styles.xml", std::ios::binary);
        if(sf)
        {
            sf.seekg(0, std::ios::end);
            size_t ssize = sf.tellg();
            sf.seekg(0, std::ios::beg);
            if(ssize <= MAX_FILE_SIZE)
            {
                std::string styles_content(ssize, '\0');
                sf.read(&styles_content[0], ssize);
                parse_styles(styles_content);
            }
        }
    }

    std::string doc_path = dir + "word/document.xml";
    
    std::ifstream file(doc_path, std::ios::binary);
    if(!file)
    {
        error_msg = "Error: Unable to open file ";
        error_msg.append(doc_path);
        return false;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if(size > MAX_FILE_SIZE)
    {
        error_msg = "Error: File size exceeds maximum limit";
        return false;
    }
    
    std::string file_content(size, '\0');
    file.read(&file_content[0], size);
    file.close();
    
    process_document(file_content);
    return true;
}

std::string word::to_html() const
{
    std::string html;
    html.reserve(contents.size() * 512);
    
    html += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Word Document</title>\n</head>\n<body>\n";
    
    for(const auto &content : contents)
    {
        if(content.type == WORD_CONTENT_TYPE::PARAGRAPH)
        {
            const auto &para = content.paragraph;
            
            std::string tag_name;
            if(para.heading_level > 0 && para.heading_level <= 5)
            {
                tag_name = "h";
                tag_name += '0' + para.heading_level;
            }
            else
            {
                tag_name = "p";
            }
            
            html += "<";
            html += tag_name;
            html += ">";
            
            for(const auto &run : para.runs)
            {
                std::string style;
                if(run.bold)
                {
                    style += "font-weight:bold;";
                }
                if(run.italic)
                {
                    style += "font-style:italic;";
                }
                if(run.underline)
                {
                    style += "text-decoration:underline;";
                }
                if(run.font_size > 0)
                {
                    style += "font-size:" + std::to_string(run.font_size) + "px;";
                }
                if(!run.color.empty())
                {
                    style += "color:#" + sanitize_token(run.color) + ";";
                }
                if(!run.highlight.empty())
                {
                    style += "background-color:" + sanitize_token(run.highlight) + ";";
                }
                if(!run.font_name.empty())
                {
                    style += "font-family:" + html_escape(run.font_name) + ";";
                }
                
                if(!style.empty())
                {
                    html += "<span style=\"";
                    html += style;
                    html += "\">";
                    html += html_escape(run.text);
                    html += "</span>";
                }
                else
                {
                    html += html_escape(run.text);
                }
            }
            
            html += "</";
            html += tag_name;
            html += ">\n";
        }
        else if(content.type == WORD_CONTENT_TYPE::TABLE)
        {
            const auto &table = content.table;
            
            html += "<table style=\"border-collapse: collapse;";
            if(table.width > 0) html += "width:" + std::to_string(table.width) + "px;";
            if(table.border_size > 0)
            {
                html += "border:" + std::to_string(table.border_size) + "px solid ";
                if(!table.border_color.empty())
                    html += "#" + sanitize_token(table.border_color) + ";";
                else
                    html += "#000000;";
            }
            else
            {
                html += "border:1px solid #000000;";
            }
            html += "\">\n";
            
            for(const auto &row : table.rows)
            {
                html += "<tr>\n";
                for(const auto &cell : row.cells)
                {
                    html += "<td";
                    if(cell.col_span > 1)
                        html += " colspan=\"" + std::to_string(cell.col_span) + "\"";
                    if(cell.row_span > 1)
                        html += " rowspan=\"" + std::to_string(cell.row_span) + "\"";

                    // 单元格样式
                    std::string td_style;
                    if(!cell.bg_color.empty()) td_style += "background-color:#" + sanitize_token(cell.bg_color) + ";";
                    if(!cell.text_color.empty()) td_style += "color:#" + sanitize_token(cell.text_color) + ";";
                    if(cell.width > 0) td_style += "width:" + std::to_string(cell.width) + "px;";
                    if(cell.height > 0) td_style += "height:" + std::to_string(cell.height) + "px;";
                    if(cell.padding > 0) td_style += "padding:" + std::to_string(cell.padding) + "px;";
                    if(!td_style.empty())
                    {
                        html += " style=\"";
                        html += td_style;
                        html += "\"";
                    }

                    html += ">";
                    html += html_escape(cell.text);
                    html += "</td>";
                }
                html += "</tr>\n";
            }
            
            html += "</table>\n";
        }
    }
    
    html += "</body>\n</html>";
    
    return html;
}

// ========== HTML 解析（read_html）==========

// XML 实体转义（用于 DOCX 输出）
static std::string xml_escape(const std::string &s)
{
    std::string result;
    result.reserve(s.size());
    for(char c : s)
    {
        switch(c)
        {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            default: result += c;
        }
    }
    return result;
}

// HTML 解码：还原常见的 HTML 实体
static std::string html_decode(const std::string &s)
{
    std::string result;
    result.reserve(s.size());
    for(size_t i = 0; i < s.size(); i++)
    {
        if(s[i] == '&')
        {
            if(s.compare(i, 5, "&amp;") == 0) { result += '&'; i += 4; continue; }
            if(s.compare(i, 4, "&lt;") == 0) { result += '<'; i += 3; continue; }
            if(s.compare(i, 4, "&gt;") == 0) { result += '>'; i += 3; continue; }
            if(s.compare(i, 6, "&quot;") == 0) { result += '"'; i += 5; continue; }
            if(s.compare(i, 6, "&apos;") == 0) { result += '\''; i += 5; continue; }
            if(s.compare(i, 6, "&nbsp;") == 0) { result += (char)0xC2; result += (char)0xA0; i += 5; continue; }
            // 数字实体 &#xxxx;
            if(i + 1 < s.size() && s[i + 1] == '#')
            {
                size_t j = i + 2;
                unsigned int codepoint = 0;
                bool too_big = false;
                while(j < s.size() && s[j] >= '0' && s[j] <= '9')
                {
                    codepoint = codepoint * 10 + (s[j] - '0');
                    if(codepoint > 0x10FFFF) { too_big = true; break; }  // 超出 Unicode 范围，也防止整型溢出
                    j++;
                }
                if(!too_big && j < s.size() && s[j] == ';' && codepoint > 0)
                {
                    if(codepoint < 0x80)
                    {
                        result += static_cast<char>(codepoint);
                    }
                    else if(codepoint < 0x800)
                    {
                        result += static_cast<char>(0xC0 | (codepoint >> 6));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else
                    {
                        result += static_cast<char>(0xE0 | (codepoint >> 12));
                        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    i = j;
                    continue;
                }
            }
        }
        result += s[i];
    }
    return result;
}

// 扫描 HTML 标签，返回标签名、是否闭合、是否自闭合、'&gt;' 之后的位置和属性字符串
static bool scan_html_tag(const std::string &content, size_t pos, size_t end,
                          std::string &name, bool &is_close, bool &is_self_close,
                          size_t &after, std::string &attrs)
{
    if(pos >= end || content[pos] != '<') return false;
    size_t si = pos + 1;
    is_close = false;
    is_self_close = false;
    attrs.clear();
    if(si < end && content[si] == '/')
    {
        is_close = true;
        si++;
    }
    size_t name_start = si;
    while(si < end)
    {
        char c = content[si];
        if(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '>' || c == '/')
        {
            break;
        }
        si++;
    }
    if(si == name_start) return false;
    name.assign(content, name_start, si - name_start);
    // 转小写
    for(auto &c : name) if(c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

    bool in_quote = false;
    char quote_char = 0;
    size_t attr_start = si;
    for(; si < end; si++)
    {
        char c = content[si];
        if(in_quote)
        {
            if(c == quote_char) in_quote = false;
            continue;
        }
        if(c == '"' || c == '\'')
        {
            in_quote = true;
            quote_char = c;
            continue;
        }
        if(c == '>')
        {
            if(si > 0 && content[si - 1] == '/') is_self_close = true;
            attrs.assign(content, attr_start, si - attr_start);
            after = si + 1;
            return true;
        }
    }
    return false;
}

// 从 HTML 属性字符串中取属性值
std::string word::get_html_attr(const std::string &attrs, std::string_view attr_name) const
{
    std::string pattern(attr_name);
    std::string lower;
    lower.reserve(attrs.size());
    for(auto c : attrs) lower += (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;

    pattern += "=\"";
    size_t p = lower.find(pattern);
    while(p != std::string::npos)
    {
        if(p == 0 || lower[p - 1] == ' ' || lower[p - 1] == '\t' || lower[p - 1] == '\r' || lower[p - 1] == '\n')
        {
            size_t vs = p + pattern.size();
            size_t ve = attrs.find('"', vs);
            if(ve == std::string::npos) return "";
            return attrs.substr(vs, ve - vs);
        }
        p = lower.find(pattern, p + 1);
    }
    return "";
}

// 解析 CSS 尺寸值 "12px" → 数字，非 px 返回 0
unsigned int word::parse_css_px(const std::string &val)
{
    if(val.empty()) return 0;
    size_t px = val.find("px");
    if(px == std::string::npos) return 0;
    unsigned int result = 0;
    for(size_t i = 0; i < px; i++)
        if(val[i] >= '0' && val[i] <= '9') result = result * 10 + (val[i] - '0');
    return result;
}

// 解析 CSS style 到 WORD_TABLE_CELL
void word::parse_html_cell_style(const std::string &style_str, WORD_TABLE_CELL &cell) const
{
    size_t pos = 0;
    while(pos < style_str.size())
    {
        while(pos < style_str.size() && (style_str[pos] == ' ' || style_str[pos] == '\t' ||
              style_str[pos] == '\r' || style_str[pos] == '\n' || style_str[pos] == ';'))
            pos++;
        if(pos >= style_str.size()) break;

        size_t colon = style_str.find(':', pos);
        if(colon == std::string::npos) break;
        std::string prop = style_str.substr(pos, colon - pos);
        while(!prop.empty() && (prop.back() == ' ' || prop.back() == '\t' ||
              prop.back() == '\r' || prop.back() == '\n'))
            prop.pop_back();

        pos = colon + 1;
        while(pos < style_str.size() && (style_str[pos] == ' ' || style_str[pos] == '\t' ||
              style_str[pos] == '\r' || style_str[pos] == '\n')) pos++;
        size_t semi = style_str.find(';', pos);
        std::string val;
        if(semi == std::string::npos)
        {
            val = style_str.substr(pos);
            pos = style_str.size();
        }
        else
        {
            val = style_str.substr(pos, semi - pos);
            pos = semi + 1;
        }
        while(!val.empty() && (val.back() == ' ' || val.back() == '\t' ||
              val.back() == '\r' || val.back() == '\n'))
            val.pop_back();

        if(prop == "background-color")
        {
            if(!val.empty() && val[0] == '#') cell.bg_color = val.substr(1);
            else cell.bg_color = val;
        }
        else if(prop == "color")
        {
            if(!val.empty() && val[0] == '#') cell.text_color = val.substr(1);
            else cell.text_color = val;
        }
        else if(prop == "width")
        {
            cell.width = parse_css_px(val);
        }
        else if(prop == "height")
        {
            cell.height = parse_css_px(val);
        }
        else if(prop == "padding")
        {
            cell.padding = parse_css_px(val);
        }
    }
}

// 将 HTML 颜色名转为 RRGGBB hex，不认识的返回原值
static std::string color_name_to_hex(const std::string &name)
{
    static const std::map<std::string, std::string> colors = {
        {"red", "FF0000"}, {"green", "008000"}, {"blue", "0000FF"},
        {"yellow", "FFFF00"}, {"white", "FFFFFF"}, {"black", "000000"},
        {"gray", "808080"}, {"silver", "C0C0C0"}, {"maroon", "800000"},
        {"purple", "800080"}, {"fuchsia", "FF00FF"}, {"lime", "00FF00"},
        {"olive", "808000"}, {"navy", "000080"}, {"teal", "008080"},
        {"aqua", "00FFFF"},
    };
    auto it = colors.find(name);
    if(it != colors.end()) return it->second;
    return name;
}

// 解析 CSS style 字符串到 WORD_RUN
void word::parse_html_style(const std::string &style_str, WORD_RUN &run) const
{
    size_t pos = 0;
    while(pos < style_str.size())
    {
        // 跳过空白和分号
        while(pos < style_str.size() && (style_str[pos] == ' ' || style_str[pos] == '\t' ||
              style_str[pos] == '\r' || style_str[pos] == '\n' || style_str[pos] == ';'))
            pos++;
        if(pos >= style_str.size()) break;

        size_t colon = style_str.find(':', pos);
        if(colon == std::string::npos) break;
        std::string prop = style_str.substr(pos, colon - pos);
        // 去除键尾空白
        while(!prop.empty() && (prop.back() == ' ' || prop.back() == '\t' ||
              prop.back() == '\r' || prop.back() == '\n'))
            prop.pop_back();

        pos = colon + 1;
        while(pos < style_str.size() && (style_str[pos] == ' ' || style_str[pos] == '\t' ||
              style_str[pos] == '\r' || style_str[pos] == '\n')) pos++;
        size_t semi = style_str.find(';', pos);
        std::string val;
        if(semi == std::string::npos)
        {
            val = style_str.substr(pos);
            pos = style_str.size();
        }
        else
        {
            val = style_str.substr(pos, semi - pos);
            pos = semi + 1;
        }
        while(!val.empty() && (val.back() == ' ' || val.back() == '\t' ||
              val.back() == '\r' || val.back() == '\n'))
            val.pop_back();

        if(prop == "font-weight" && val == "bold")
            run.bold = true;
        else if(prop == "font-style" && val == "italic")
            run.italic = true;
        else if(prop == "text-decoration" && val == "underline")
            run.underline = true;
        else if(prop == "font-size")
        {
            size_t px = val.find("px");
            if(px != std::string::npos)
            {
                unsigned int sz = 0;
                for(size_t i = 0; i < px; i++)
                    if(val[i] >= '0' && val[i] <= '9') sz = sz * 10 + (val[i] - '0');
                if(sz > 0 && sz < 256) run.font_size = static_cast<unsigned char>(sz);
            }
        }
        else if(prop == "color" && !val.empty())
        {
            if(val[0] == '#')
                run.color = val.substr(1);
            else
                run.color = color_name_to_hex(val);
        }
        else if(prop == "background-color")
        {
            run.highlight = val;
        }
        else if(prop == "font-family")
        {
            // 去掉引号
            if(!val.empty() && (val.front() == '"' || val.front() == '\''))
                val = val.substr(1, val.size() - 2);
            run.font_name = val;
        }
    }
}

// 找同名闭合标签（支持嵌套），返回整个元素结束后的位置
static size_t find_closing_tag(const std::string &content, size_t begin, size_t end, const std::string &tag_name)
{
    int depth = 1;
    size_t pos = begin;
    while(pos < end)
    {
        size_t lt = content.find('<', pos);
        if(lt == std::string::npos) break;
        // 跳过注释
        if(content.compare(lt, 4, "<!--") == 0)
        {
            size_t e = content.find("-->", lt + 4);
            if(e == std::string::npos) break;
            pos = e + 3;
            continue;
        }
        std::string name;
        bool is_close = false, is_self = false;
        size_t after = 0;
        std::string attrs;
        if(!scan_html_tag(content, lt, end, name, is_close, is_self, after, attrs))
        {
            pos = lt + 1;
            continue;
        }
        if(name == tag_name && !is_self)
        {
            if(is_close) depth--;
            else depth++;
            if(depth == 0) return after;
        }
        pos = after;
    }
    return end;
}

// CSS margin 属性 → 页边距，支持 margin-top/right/bottom/left 和 margin 简写（1~4 个值）
void word::apply_css_margin(const std::string &prop, const std::string &val)
{
    if(prop == "margin-top") { margin_top = parse_css_px(val); return; }
    if(prop == "margin-right") { margin_right = parse_css_px(val); return; }
    if(prop == "margin-bottom") { margin_bottom = parse_css_px(val); return; }
    if(prop == "margin-left") { margin_left = parse_css_px(val); return; }
    if(prop != "margin") return;

    // 按空白拆分简写值
    std::vector<std::string> vals;
    size_t pos = 0;
    while(pos < val.size())
    {
        while(pos < val.size() && (val[pos] == ' ' || val[pos] == '\t')) pos++;
        size_t start = pos;
        while(pos < val.size() && val[pos] != ' ' && val[pos] != '\t') pos++;
        if(pos > start) vals.push_back(val.substr(start, pos - start));
    }
    if(vals.empty()) return;

    if(vals.size() == 1)
    {
        unsigned int v = parse_css_px(vals[0]);
        margin_top = margin_right = margin_bottom = margin_left = v;
    }
    else if(vals.size() == 2)
    {
        margin_top = margin_bottom = parse_css_px(vals[0]);
        margin_left = margin_right = parse_css_px(vals[1]);
    }
    else if(vals.size() == 3)
    {
        margin_top = parse_css_px(vals[0]);
        margin_left = margin_right = parse_css_px(vals[1]);
        margin_bottom = parse_css_px(vals[2]);
    }
    else
    {
        margin_top = parse_css_px(vals[0]);
        margin_right = parse_css_px(vals[1]);
        margin_bottom = parse_css_px(vals[2]);
        margin_left = parse_css_px(vals[3]);
    }
}

// 解析 <head><style> 的 CSS 文本：body{} 规则作为全局默认样式，
// body{}/@page{} 里的 margin 作为页边距
void word::parse_html_head_style(const std::string &css)
{
    // 去掉注释 /* ... */
    std::string clean;
    clean.reserve(css.size());
    for(size_t si = 0; si < css.size(); si++)
    {
        if(css.compare(si, 2, "/*") == 0)
        {
            size_t e = css.find("*/", si + 2);
            if(e == std::string::npos) break;
            si = e + 1;
            continue;
        }
        clean += css[si];
    }

    // 逐条规则：选择器 { 声明块 }
    size_t pos = 0;
    while(pos < clean.size())
    {
        size_t brace = clean.find('{', pos);
        if(brace == std::string::npos) break;
        size_t close = clean.find('}', brace);
        if(close == std::string::npos) break;

        std::string selector = clean.substr(pos, brace - pos);
        std::string decls = clean.substr(brace + 1, close - brace - 1);
        pos = close + 1;

        // 选择器按逗号拆分，匹配 body/html/@page
        bool is_body = false, is_page = false;
        size_t sp = 0;
        while(sp < selector.size())
        {
            size_t comma = selector.find(',', sp);
            std::string one = selector.substr(sp, comma == std::string::npos ? std::string::npos : comma - sp);
            sp = (comma == std::string::npos) ? selector.size() : comma + 1;
            // 去首尾空白
            size_t b = one.find_first_not_of(" \t\r\n");
            size_t e = one.find_last_not_of(" \t\r\n");
            if(b == std::string::npos) continue;
            one = one.substr(b, e - b + 1);
            if(one == "body" || one == "html") is_body = true;
            else if(one == "@page") is_page = true;
        }
        if(!is_body && !is_page) continue;

        // 字体类属性只对 body 生效
        if(is_body)
        {
            parse_html_style(decls, default_style);
            // body 的 background-color 是页面背景，不作为文字高亮
            default_style.highlight.clear();
        }

        // margin 属性 → 页边距
        size_t dp = 0;
        while(dp < decls.size())
        {
            while(dp < decls.size() && (decls[dp] == ' ' || decls[dp] == '\t' ||
                  decls[dp] == '\r' || decls[dp] == '\n' || decls[dp] == ';'))
                dp++;
            if(dp >= decls.size()) break;
            size_t colon = decls.find(':', dp);
            if(colon == std::string::npos) break;
            std::string prop = decls.substr(dp, colon - dp);
            while(!prop.empty() && (prop.back() == ' ' || prop.back() == '\t'))
                prop.pop_back();
            dp = colon + 1;
            while(dp < decls.size() && decls[dp] == ' ') dp++;
            size_t semi = decls.find(';', dp);
            std::string val = decls.substr(dp, semi == std::string::npos ? std::string::npos : semi - dp);
            dp = (semi == std::string::npos) ? decls.size() : semi + 1;
            while(!val.empty() && (val.back() == ' ' || val.back() == '\t' ||
                  val.back() == '\r' || val.back() == '\n'))
                val.pop_back();
            apply_css_margin(prop, val);
        }
    }
}

// 解析 body 内块级元素
void word::parse_html_body(const std::string &content, size_t begin, size_t end)
{
    size_t pos = begin;
    while(pos < end)
    {
        size_t lt = content.find('<', pos);
        if(lt == std::string::npos) break;

        // 跳过注释
        if(content.compare(lt, 4, "<!--") == 0)
        {
            size_t e = content.find("-->", lt + 4);
            if(e == std::string::npos) break;
            pos = e + 3;
            continue;
        }

        std::string name;
        bool is_close = false, is_self = false;
        size_t after = 0;
        std::string attrs;
        if(!scan_html_tag(content, lt, end, name, is_close, is_self, after, attrs))
        {
            pos = lt + 1;
            continue;
        }

        if(is_close)
        {
            pos = after;
            continue;
        }

        if(name == "h1" || name == "h2" || name == "h3" || name == "h4" || name == "h5" ||
           name == "p" || name == "div")
        {
            size_t closing = find_closing_tag(content, after, end, name);
            parse_html_block(content, after, closing, name);
            pos = closing;
        }
        else if(name == "table")
        {
            size_t closing = find_closing_tag(content, after, end, name);
            WORD_TABLE table;
            parse_html_table(content, after, closing, table, attrs);
            if(!table.rows.empty())
            {
                WORD_CONTENT c;
                c.type = WORD_CONTENT_TYPE::TABLE;
                c.table = std::move(table);
                contents.push_back(std::move(c));
            }
            pos = closing;
        }
        else
        {
            pos = after;
        }
    }
}

// 解析块级元素（h1-h5, p, div）
void word::parse_html_block(const std::string &content, size_t begin, size_t end, const std::string &tag_name)
{
    WORD_PARAGRAPH para;

    // 设置标题级别
    if(tag_name.size() == 2 && tag_name[0] == 'h' && tag_name[1] >= '1' && tag_name[1] <= '5')
    {
        para.heading_level = tag_name[1] - '0';
    }

    // run 从全局默认样式继承；标题的字号由 Heading 样式决定，不继承全局字号
    WORD_RUN def = default_style;
    def.text.clear();
    if(para.heading_level > 0)
    {
        def.font_size = 0;
    }

    parse_html_inline(content, begin, end, para, def);

    if(!para.runs.empty())
    {
        WORD_CONTENT c;
        c.type = WORD_CONTENT_TYPE::PARAGRAPH;
        c.paragraph = std::move(para);
        contents.push_back(std::move(c));
    }
}

// 解析行内元素（span 等），提取文本和格式
void word::parse_html_inline(const std::string &content, size_t begin, size_t end, WORD_PARAGRAPH &para, const WORD_RUN &default_run, int depth)
{
    if(depth > MAX_PARSE_DEPTH) return;  // 防止恶意深度嵌套的 span/font 导致栈溢出
    WORD_RUN current_run = default_run;
    size_t pos = begin;

    while(pos < end)
    {
        size_t lt = content.find('<', pos);
        if(lt == std::string::npos || lt >= end)
        {
            // 剩余纯文本
            std::string text = html_decode(content.substr(pos, end - pos));
            current_run.text += text;
            break;
        }

        // lt 之前的纯文本
        if(lt > pos)
        {
            std::string text = html_decode(content.substr(pos, lt - pos));
            current_run.text += text;
        }

        // 跳过注释
        if(content.compare(lt, 4, "<!--") == 0)
        {
            size_t e = content.find("-->", lt + 4);
            if(e == std::string::npos) break;
            pos = e + 3;
            continue;
        }

        std::string name;
        bool is_close = false, is_self = false;
        size_t after = 0;
        std::string attrs;
        if(!scan_html_tag(content, lt, end, name, is_close, is_self, after, attrs))
        {
            pos = lt + 1;
            continue;
        }

        if((name == "span" || name == "font") && !is_close)
        {
            // span/font 开始标签：保存当前 run 的格式，新 run 继承父级格式
            WORD_RUN inner_run = current_run;  // 先复制父级格式（在 move 之前）
            inner_run.text.clear();                // 清空文本，只保留格式
            if(!current_run.text.empty())
            {
                para.runs.push_back(std::move(current_run));
                current_run = default_run;
            }
            // 解析 style 属性，叠加到 inner_run 上
            std::string style = get_html_attr(attrs, "style");
            if(!style.empty())
                parse_html_style(style, inner_run);
            // font 标签的 color 属性
            if(name == "font")
            {
                std::string font_color = get_html_attr(attrs, "color");
                if(!font_color.empty())
                {
                    if(font_color[0] == '#')
                        inner_run.color = font_color.substr(1);
                    else
                        inner_run.color = color_name_to_hex(font_color);
                }
            }

            size_t closing = find_closing_tag(content, after, end, name);
            // 递归解析内部，继承 inner_run 的格式
            parse_html_inline(content, after, closing, para, inner_run, depth + 1);
            pos = closing;
            continue;
        }
        else if((name == "/span" || name == "/font" || (name == "span" && is_close) || (name == "font" && is_close)))
        {
            // span 闭合：保存当前 run
            if(!current_run.text.empty())
            {
                para.runs.push_back(std::move(current_run));
                current_run = default_run;
            }
            pos = after;
            continue;
        }
        else if(name == "br" || is_self)
        {
            current_run.text += '\n';
            pos = after;
            continue;
        }
        else if(is_close)
        {
            // 块级元素的闭合标签，停止
            if(name == "/p" || name == "/div" || (name.size() == 3 && name[0] == '/' && name[1] == 'h'))
            {
                pos = after;
                break;
            }
            pos = after;
            continue;
        }

        pos = after;
    }

    if(!current_run.text.empty())
    {
        para.runs.push_back(std::move(current_run));
    }
}

// 解析 HTML table
void word::parse_html_table(const std::string &content, size_t begin, size_t end, WORD_TABLE &table, const std::string &table_attrs)
{
    // 解析 table 自身的 style
    std::string table_style = get_html_attr(table_attrs, "style");
    if(!table_style.empty())
    {
        size_t pos = 0;
        while(pos < table_style.size())
        {
            while(pos < table_style.size() && (table_style[pos] == ' ' || table_style[pos] == '\t' || table_style[pos] == ';'))
                pos++;
            if(pos >= table_style.size()) break;
            size_t colon = table_style.find(':', pos);
            if(colon == std::string::npos) break;
            std::string prop = table_style.substr(pos, colon - pos);
            while(!prop.empty() && (prop.back() == ' ' || prop.back() == '\t')) prop.pop_back();
            pos = colon + 1;
            while(pos < table_style.size() && table_style[pos] == ' ') pos++;
            size_t semi = table_style.find(';', pos);
            std::string val;
            if(semi == std::string::npos) { val = table_style.substr(pos); pos = table_style.size(); }
            else { val = table_style.substr(pos, semi - pos); pos = semi + 1; }
            while(!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();

            if(prop == "width")
            {
                table.width = parse_css_px(val);
            }
            else if(prop == "border")
            {
                // border: 2px solid #FF0000
                // 解析边框大小和颜色
                size_t space1 = val.find(' ');
                if(space1 != std::string::npos)
                {
                    std::string sz_str = val.substr(0, space1);
                    unsigned int sz = parse_css_px(sz_str);
                    if(sz > 0) table.border_size = sz;
                    // 找颜色值（#开头）
                    size_t hash = val.find('#');
                    if(hash != std::string::npos)
                    {
                        std::string clr = val.substr(hash + 1);
                        // 去除尾部空白和分号
                        while(!clr.empty() && (clr.back() == ' ' || clr.back() == ';'))
                            clr.pop_back();
                        if(!clr.empty()) table.border_color = clr;
                    }
                }
            }
            else if(prop == "border-color")
            {
                if(!val.empty() && val[0] == '#') table.border_color = val.substr(1);
                else table.border_color = val;
            }
        }
    }

    size_t pos = begin;
    while(pos < end)
    {
        size_t lt = content.find('<', pos);
        if(lt == std::string::npos) break;

        if(content.compare(lt, 4, "<!--") == 0)
        {
            size_t e = content.find("-->", lt + 4);
            if(e == std::string::npos) break;
            pos = e + 3;
            continue;
        }

        std::string name;
        bool is_close = false, is_self = false;
        size_t after = 0;
        std::string attrs;
        if(!scan_html_tag(content, lt, end, name, is_close, is_self, after, attrs))
        {
            pos = lt + 1;
            continue;
        }

        if(name == "tr" && !is_close)
        {
            size_t tr_end = find_closing_tag(content, after, end, "tr");
            WORD_TABLE_ROW row;
            size_t tr_pos = after;
            while(tr_pos < tr_end)
            {
                size_t td_lt = content.find('<', tr_pos);
                if(td_lt == std::string::npos || td_lt >= tr_end) break;

                if(content.compare(td_lt, 4, "<!--") == 0)
                {
                    size_t e = content.find("-->", td_lt + 4);
                    if(e == std::string::npos) break;
                    tr_pos = e + 3;
                    continue;
                }

                std::string td_name;
                bool td_close = false, td_self = false;
                size_t td_after = 0;
                std::string td_attrs;
                if(!scan_html_tag(content, td_lt, tr_end, td_name, td_close, td_self, td_after, td_attrs))
                {
                    tr_pos = td_lt + 1;
                    continue;
                }

                if((td_name == "td" || td_name == "th") && !td_close)
                {
                    size_t td_end = find_closing_tag(content, td_after, tr_end, td_name);
                    WORD_TABLE_CELL cell;

                    // 解析 td/th 的 style 属性
                    std::string td_style = get_html_attr(td_attrs, "style");
                    if(!td_style.empty())
                        parse_html_cell_style(td_style, cell);

                    // 提取 td 内的纯文本（忽略 span 等格式）
                    std::string cell_text;
                    size_t td_pos = td_after;
                    while(td_pos < td_end)
                    {
                        size_t inner_lt = content.find('<', td_pos);
                        if(inner_lt == std::string::npos || inner_lt >= td_end)
                        {
                            cell_text += html_decode(content.substr(td_pos, td_end - td_pos));
                            break;
                        }
                        if(inner_lt > td_pos)
                            cell_text += html_decode(content.substr(td_pos, inner_lt - td_pos));

                        std::string inner_name;
                        bool inner_close = false, inner_self = false;
                        size_t inner_after = 0;
                        std::string inner_attrs;
                        if(scan_html_tag(content, inner_lt, td_end, inner_name, inner_close, inner_self, inner_after, inner_attrs))
                        {
                            if(inner_name == "br")
                                cell_text += '\n';
                            td_pos = inner_after;
                            continue;
                        }
                        td_pos = inner_lt + 1;
                    }
                    if(!cell_text.empty())
                    {
                        // 去除首尾空白
                        size_t first = 0, last = cell_text.size();
                        while(first < last && (cell_text[first] == ' ' || cell_text[first] == '\t' || cell_text[first] == '\n' || cell_text[first] == '\r'))
                            first++;
                        while(last > first && (cell_text[last - 1] == ' ' || cell_text[last - 1] == '\t' || cell_text[last - 1] == '\n' || cell_text[last - 1] == '\r'))
                            last--;
                        cell.text = cell_text.substr(first, last - first);
                    }
                    row.cells.push_back(std::move(cell));
                    tr_pos = td_end;
                }
                else
                {
                    tr_pos = td_after;
                }
            }
            if(!row.cells.empty())
            {
                if(row.cells.size() > table.cols)
                    table.cols = static_cast<unsigned int>(row.cells.size());
                table.rows.push_back(std::move(row));
            }
            pos = tr_end;
        }
        else
        {
            pos = after;
        }
    }
}

// ========== DOCX 写入（write）==========

static bool zip_add_file(zipFile zf, const char *filename, const std::string &content)
{
    // zipWriteInFileInZip 的长度参数是 unsigned int，防止 size_t 截断写出损坏的 zip
    if(content.size() > 0xFFFFFFFFu) return false;
    zip_fileinfo zi = {};
    if(zipOpenNewFileInZip(zf, filename, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
        return false;
    if(zipWriteInFileInZip(zf, content.data(), static_cast<unsigned int>(content.size())) != ZIP_OK)
    {
        zipCloseFileInZip(zf);
        return false;
    }
    zipCloseFileInZip(zf);
    return true;
}

bool word::read_html(const std::string &html_file)
{
    clear();

    std::ifstream file(html_file, std::ios::binary);
    if(!file)
    {
        error_msg = "Error: Unable to open file " + html_file;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if(size > MAX_FILE_SIZE)
    {
        error_msg = "Error: File size exceeds maximum limit";
        return false;
    }

    std::string html_content(size, '\0');
    file.read(&html_content[0], size);
    file.close();

    // 小写副本用于定位标签（不区分大小写）
    std::string lower;
    lower.reserve(html_content.size());
    for(auto c : html_content) lower += (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;

    // 解析 <head> 里的 <style> 作为全局样式
    {
        size_t head_end = lower.find("</head>");
        if(head_end == std::string::npos) head_end = lower.find("<body");
        if(head_end == std::string::npos) head_end = lower.size();

        size_t sp = 0;
        while(sp < head_end)
        {
            size_t style_start = lower.find("<style", sp);
            if(style_start == std::string::npos || style_start >= head_end) break;
            size_t gt = html_content.find('>', style_start);
            if(gt == std::string::npos) break;
            size_t style_end = lower.find("</style>", gt);
            if(style_end == std::string::npos || style_end > head_end) break;
            parse_html_head_style(html_content.substr(gt + 1, style_end - gt - 1));
            sp = style_end + 8;
        }
    }

    // 找 <body> 标签
    size_t body_start = 0;
    size_t body_end = html_content.size();
    {
        size_t p = lower.find("<body");
        if(p != std::string::npos)
        {
            size_t gt = html_content.find('>', p);
            if(gt != std::string::npos)
                body_start = gt + 1;
        }

        p = lower.find("</body>");
        if(p != std::string::npos)
            body_end = p;
    }

    if(body_start >= body_end)
    {
        error_msg = "Error: <body> not found in HTML";
        return false;
    }

    parse_html_body(html_content, body_start, body_end);
    return true;
}

bool word::write(const std::string &docx_file)
{
    error_msg.clear();

    zipFile zf = zipOpen(docx_file.c_str(), APPEND_STATUS_CREATE);
    if(zf == NULL)
    {
        error_msg = "Error: Unable to create zip file " + docx_file;
        return false;
    }

    if(!zip_add_file(zf, "[Content_Types].xml", gen_docx_content_types()))
    {
        error_msg = "Error: Failed to add [Content_Types].xml";
        zipClose(zf, NULL);
        return false;
    }
    if(!zip_add_file(zf, "_rels/.rels", gen_docx_rels()))
    {
        error_msg = "Error: Failed to add _rels/.rels";
        zipClose(zf, NULL);
        return false;
    }
    if(!zip_add_file(zf, "word/_rels/document.xml.rels", gen_docx_document_rels()))
    {
        error_msg = "Error: Failed to add word/_rels/document.xml.rels";
        zipClose(zf, NULL);
        return false;
    }
    if(!zip_add_file(zf, "word/document.xml", gen_docx_document()))
    {
        error_msg = "Error: Failed to add word/document.xml";
        zipClose(zf, NULL);
        return false;
    }
    if(!zip_add_file(zf, "word/styles.xml", gen_docx_styles()))
    {
        error_msg = "Error: Failed to add word/styles.xml";
        zipClose(zf, NULL);
        return false;
    }

    zipClose(zf, NULL);
    return true;
}

// 生成 [Content_Types].xml
std::string word::gen_docx_content_types() const
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
           "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
           "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
           "<Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
           "</Types>";
}

// 生成 _rels/.rels
std::string word::gen_docx_rels() const
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>"
           "</Relationships>";
}

// 生成 word/_rels/document.xml.rels
std::string word::gen_docx_document_rels() const
{
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           "</Relationships>";
}

// 生成 word/document.xml
std::string word::gen_docx_document() const
{
    std::string xml;
    xml.reserve(contents.size() * 512);

    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
           " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
           "<w:body>";

    for(const auto &content : contents)
    {
        if(content.type == WORD_CONTENT_TYPE::PARAGRAPH)
        {
            const auto &para = content.paragraph;
            xml += "<w:p>";

            // 段落属性
            if(para.heading_level > 0 && para.heading_level <= 5)
            {
                xml += "<w:pPr><w:pStyle w:val=\"Heading";
                xml += '0' + para.heading_level;
                xml += "\"/></w:pPr>";
            }

            for(const auto &run : para.runs)
            {
                // 收集 run 属性，只有有实际属性时才生成 w:rPr
                // 注意：OOXML 规范要求 w:rPr 子元素有严格顺序：
                // rFonts → b/bCs → i/iCs → color → sz/szCs → highlight → u
                // 顺序不对 Word 会丢弃属性（如文字颜色失效）
                std::string rpr;

                if(!run.font_name.empty())
                {
                    // 字体名来自不可信 HTML（可能含 " < > &），必须转义防止属性注入
                    std::string fn = xml_escape(run.font_name);
                    rpr += "<w:rFonts w:ascii=\"" + fn + "\""
                           " w:eastAsia=\"" + fn + "\""
                           " w:hAnsi=\"" + fn + "\"/>";
                }
                else if(run.bold || run.italic || run.underline || run.font_size > 0 ||
                        !run.color.empty() || !run.highlight.empty())
                {
                    // 有颜色/格式但无字体名时，加默认字体确保 Pages 兼容
                    rpr += "<w:rFonts w:ascii=\"Calibri\" w:eastAsia=\"宋体\" w:hAnsi=\"Calibri\"/>";
                }
                if(run.bold)
                    rpr += "<w:b/><w:bCs/>";
                if(run.italic)
                    rpr += "<w:i/><w:iCs/>";
                if(!run.color.empty())
                {
                    rpr += "<w:color w:val=\"" + sanitize_token(run.color) + "\"/>";
                }
                if(run.font_size > 0)
                {
                    rpr += "<w:sz w:val=\"" + std::to_string(run.font_size * 2) + "\"/>";
                    rpr += "<w:szCs w:val=\"" + std::to_string(run.font_size * 2) + "\"/>";
                }
                if(!run.highlight.empty())
                {
                    rpr += "<w:highlight w:val=\"" + sanitize_token(run.highlight) + "\"/>";
                }
                if(run.underline)
                    rpr += "<w:u w:val=\"single\"/>";

                xml += "<w:r>";
                if(!rpr.empty())
                {
                    xml += "<w:rPr>" + rpr + "</w:rPr>";
                }

                // w:t 元素，如果文本以空格开头或结尾，需要 xml:space="preserve"
                if(!run.text.empty() && (run.text.front() == ' ' || run.text.back() == ' '))
                {
                    xml += "<w:t xml:space=\"preserve\">" + xml_escape(run.text) + "</w:t>";
                }
                else
                {
                    xml += "<w:t>" + xml_escape(run.text) + "</w:t>";
                }

                xml += "</w:r>";
            }

            xml += "</w:p>";
        }
        else if(content.type == WORD_CONTENT_TYPE::TABLE)
        {
            const auto &table = content.table;
            xml += "<w:tbl><w:tblPr>";

            // 表格宽度
            if(table.width > 0)
                xml += "<w:tblW w:w=\"" + std::to_string(table.width * 15) + "\" w:type=\"dxa\"/>";
            else
                xml += "<w:tblW w:w=\"0\" w:type=\"auto\"/>";

            // 边框
            std::string border_sz = "4";  // 默认 0.5pt
            std::string border_clr = "auto";
            if(table.border_size > 0)
                border_sz = std::to_string(table.border_size * 8);  // px → 1/8 pt
            if(!table.border_color.empty())
                border_clr = sanitize_token(table.border_color);

            xml += "<w:tblBorders><w:top w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "<w:left w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "<w:bottom w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "<w:right w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "<w:insideH w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "<w:insideV w:val=\"single\" w:sz=\"" + border_sz + "\" w:space=\"0\" w:color=\"" + border_clr + "\"/>"
                   "</w:tblBorders></w:tblPr>";

            // 网格列定义
            xml += "<w:tblGrid>";
            for(unsigned int c = 0; c < table.cols; c++)
                xml += "<w:gridCol w:w=\"" + std::to_string(9000 / (table.cols > 0 ? table.cols : 1)) + "\"/>";
            xml += "</w:tblGrid>";

            for(const auto &row : table.rows)
            {
                xml += "<w:tr>";
                for(const auto &cell : row.cells)
                {
                    xml += "<w:tc><w:tcPr>";

                    // 单元格宽度
                    if(cell.width > 0)
                        xml += "<w:tcW w:w=\"" + std::to_string(cell.width * 15) + "\" w:type=\"dxa\"/>";

                    // 背景颜色
                    if(!cell.bg_color.empty())
                        xml += "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + sanitize_token(cell.bg_color) + "\"/>";

                    // 内边距
                    if(cell.padding > 0)
                    {
                        std::string pad = std::to_string(cell.padding * 15);
                        xml += "<w:tcMar><w:top w:w=\"" + pad + "\" w:type=\"dxa\"/>"
                               "<w:left w:w=\"" + pad + "\" w:type=\"dxa\"/>"
                               "<w:bottom w:w=\"" + pad + "\" w:type=\"dxa\"/>"
                               "<w:right w:w=\"" + pad + "\" w:type=\"dxa\"/></w:tcMar>";
                    }

                    xml += "</w:tcPr>";

                    // 单元格内容（带文字颜色）
                    xml += "<w:p>";
                    if(!cell.text_color.empty())
                    {
                        xml += "<w:r><w:rPr><w:color w:val=\"" + sanitize_token(cell.text_color) + "\"/>";
                        if(!cell.text.empty() && (cell.text.front() == ' ' || cell.text.back() == ' '))
                            xml += "</w:rPr><w:t xml:space=\"preserve\">" + xml_escape(cell.text) + "</w:t></w:r>";
                        else
                            xml += "</w:rPr><w:t>" + xml_escape(cell.text) + "</w:t></w:r>";
                    }
                    else
                    {
                        xml += "<w:r><w:t>";
                        xml += xml_escape(cell.text);
                        xml += "</w:t></w:r>";
                    }
                    xml += "</w:p></w:tc>";
                }
                xml += "</w:tr>";
            }

            xml += "</w:tbl>";
        }
    }

    // 页面设置：A4，页边距由成员变量控制（px → twips，1px = 15 twips）
    xml += "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
           "<w:pgMar w:top=\"" + std::to_string(margin_top * 15) + "\""
           " w:right=\"" + std::to_string(margin_right * 15) + "\""
           " w:bottom=\"" + std::to_string(margin_bottom * 15) + "\""
           " w:left=\"" + std::to_string(margin_left * 15) + "\""
           " w:header=\"851\" w:footer=\"992\" w:gutter=\"0\"/>"
           "<w:cols w:space=\"425\"/><w:docGrid w:type=\"lines\" w:linePitch=\"312\"/></w:sectPr>";

    xml += "</w:body></w:document>";

    return xml;
}

// 生成 word/styles.xml，定义段落默认样式和标题样式 1-5
std::string word::gen_docx_styles() const
{
    // 文档默认 run 属性：优先用全局样式（HTML <style> body{}），无则用主题默认
    // 注意 w:rPr 子元素顺序：rFonts → color → sz/szCs → lang
    std::string def_rpr;
    if(!default_style.font_name.empty())
    {
        // 字体名来自不可信 HTML <style>，转义防止属性注入
        std::string fn = xml_escape(default_style.font_name);
        def_rpr += "<w:rFonts w:ascii=\"" + fn + "\""
                   " w:eastAsia=\"" + fn + "\""
                   " w:hAnsi=\"" + fn + "\""
                   " w:cs=\"" + fn + "\"/>";
    }
    else
    {
        def_rpr += "<w:rFonts w:asciiTheme=\"minorHAnsi\" w:eastAsiaTheme=\"minorEastAsia\" w:hAnsiTheme=\"minorHAnsi\" w:cstheme=\"minorBidi\"/>";
    }
    if(!default_style.color.empty() && default_style.color != "000000" && default_style.color != "auto")
    {
        def_rpr += "<w:color w:val=\"" + sanitize_token(default_style.color) + "\"/>";
    }
    unsigned int def_sz = (default_style.font_size > 0) ? default_style.font_size * 2u : 22u;
    def_rpr += "<w:sz w:val=\"" + std::to_string(def_sz) + "\"/><w:szCs w:val=\"" + std::to_string(def_sz) + "\"/>";
    def_rpr += "<w:lang w:val=\"en-US\" w:eastAsia=\"zh-CN\" w:bidi=\"ar-SA\"/>";

    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
           // 文档默认格式
           "<w:docDefaults>"
           "<w:rPrDefault><w:rPr>" + def_rpr + "</w:rPr></w:rPrDefault>"
           "<w:pPrDefault><w:pPr><w:spacing w:after=\"200\" w:line=\"276\" w:lineRule=\"auto\"/></w:pPr></w:pPrDefault>"
           "</w:docDefaults>"

           // 声明内置样式可用（Word/Pages 兼容性关键）
           "<w:latentStyles w:defLockedState=\"0\" w:defUIPriority=\"99\" w:defSemiHidden=\"0\""
           " w:defUnhideWhenUsed=\"0\" w:defQFormat=\"0\" w:count=\"376\">"
           "<w:lsdException w:name=\"Normal\" w:uiPriority=\"0\" w:qFormat=\"1\"/>"
           "<w:lsdException w:name=\"heading 1\" w:uiPriority=\"9\" w:qFormat=\"1\"/>"
           "<w:lsdException w:name=\"heading 2\" w:semiHidden=\"1\" w:uiPriority=\"9\" w:unhideWhenUsed=\"1\" w:qFormat=\"1\"/>"
           "<w:lsdException w:name=\"heading 3\" w:semiHidden=\"1\" w:uiPriority=\"9\" w:unhideWhenUsed=\"1\" w:qFormat=\"1\"/>"
           "<w:lsdException w:name=\"heading 4\" w:semiHidden=\"1\" w:uiPriority=\"9\" w:unhideWhenUsed=\"1\" w:qFormat=\"1\"/>"
           "<w:lsdException w:name=\"heading 5\" w:semiHidden=\"1\" w:uiPriority=\"9\" w:unhideWhenUsed=\"1\" w:qFormat=\"1\"/>"
           "</w:latentStyles>"

           // 默认段落样式 "Normal"
           "<w:style w:type=\"paragraph\" w:styleId=\"Normal\" w:default=\"1\">"
           "<w:name w:val=\"Normal\"/>"
           "<w:qFormat/>"
           "</w:style>"

           // Heading 1: 加粗 22pt
           "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\">"
           "<w:name w:val=\"heading 1\"/>"
           "<w:basedOn w:val=\"Normal\"/>"
           "<w:next w:val=\"Normal\"/>"
           "<w:link w:val=\"Heading1Char\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:qFormat/>"
           "<w:pPr><w:spacing w:before=\"480\" w:after=\"200\"/><w:outlineLvl w:val=\"0\"/></w:pPr>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"44\"/><w:szCs w:val=\"44\"/>"
           "</w:rPr></w:style>"

           // Heading 2: 加粗 16pt
           "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\">"
           "<w:name w:val=\"heading 2\"/>"
           "<w:basedOn w:val=\"Normal\"/>"
           "<w:next w:val=\"Normal\"/>"
           "<w:link w:val=\"Heading2Char\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:qFormat/>"
           "<w:pPr><w:spacing w:before=\"360\" w:after=\"160\"/><w:outlineLvl w:val=\"1\"/></w:pPr>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"32\"/><w:szCs w:val=\"32\"/>"
           "</w:rPr></w:style>"

           // Heading 3: 加粗 14pt
           "<w:style w:type=\"paragraph\" w:styleId=\"Heading3\">"
           "<w:name w:val=\"heading 3\"/>"
           "<w:basedOn w:val=\"Normal\"/>"
           "<w:next w:val=\"Normal\"/>"
           "<w:link w:val=\"Heading3Char\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:qFormat/>"
           "<w:pPr><w:spacing w:before=\"280\" w:after=\"120\"/><w:outlineLvl w:val=\"2\"/></w:pPr>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"28\"/><w:szCs w:val=\"28\"/>"
           "</w:rPr></w:style>"

           // Heading 4: 加粗 12pt
           "<w:style w:type=\"paragraph\" w:styleId=\"Heading4\">"
           "<w:name w:val=\"heading 4\"/>"
           "<w:basedOn w:val=\"Normal\"/>"
           "<w:next w:val=\"Normal\"/>"
           "<w:link w:val=\"Heading4Char\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:qFormat/>"
           "<w:pPr><w:spacing w:before=\"240\" w:after=\"80\"/><w:outlineLvl w:val=\"3\"/></w:pPr>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"24\"/><w:szCs w:val=\"24\"/>"
           "</w:rPr></w:style>"

           // Heading 5: 加粗 11pt
           "<w:style w:type=\"paragraph\" w:styleId=\"Heading5\">"
           "<w:name w:val=\"heading 5\"/>"
           "<w:basedOn w:val=\"Normal\"/>"
           "<w:next w:val=\"Normal\"/>"
           "<w:link w:val=\"Heading5Char\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:qFormat/>"
           "<w:pPr><w:spacing w:before=\"200\" w:after=\"80\"/><w:outlineLvl w:val=\"4\"/></w:pPr>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"22\"/><w:szCs w:val=\"22\"/>"
           "</w:rPr></w:style>"

           // 对应的字符样式（Heading1Char-Heading5Char，用于 w:link 双向链接）
           "<w:style w:type=\"character\" w:styleId=\"Heading1Char\">"
           "<w:name w:val=\"Heading 1 Char\"/>"
           "<w:link w:val=\"Heading1\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"44\"/><w:szCs w:val=\"44\"/>"
           "</w:rPr></w:style>"

           "<w:style w:type=\"character\" w:styleId=\"Heading2Char\">"
           "<w:name w:val=\"Heading 2 Char\"/>"
           "<w:link w:val=\"Heading2\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"32\"/><w:szCs w:val=\"32\"/>"
           "</w:rPr></w:style>"

           "<w:style w:type=\"character\" w:styleId=\"Heading3Char\">"
           "<w:name w:val=\"Heading 3 Char\"/>"
           "<w:link w:val=\"Heading3\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"28\"/><w:szCs w:val=\"28\"/>"
           "</w:rPr></w:style>"

           "<w:style w:type=\"character\" w:styleId=\"Heading4Char\">"
           "<w:name w:val=\"Heading 4 Char\"/>"
           "<w:link w:val=\"Heading4\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"24\"/><w:szCs w:val=\"24\"/>"
           "</w:rPr></w:style>"

           "<w:style w:type=\"character\" w:styleId=\"Heading5Char\">"
           "<w:name w:val=\"Heading 5 Char\"/>"
           "<w:link w:val=\"Heading5\"/>"
           "<w:uiPriority w:val=\"9\"/>"
           "<w:rPr><w:rFonts w:asciiTheme=\"majorHAnsi\" w:eastAsiaTheme=\"majorEastAsia\" w:hAnsiTheme=\"majorHAnsi\" w:cstheme=\"majorBidi\"/>"
           "<w:b/><w:bCs/><w:sz w:val=\"22\"/><w:szCs w:val=\"22\"/>"
           "</w:rPr></w:style>"

           "</w:styles>";
}

}