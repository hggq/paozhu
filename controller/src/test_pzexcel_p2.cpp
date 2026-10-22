#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "func.h"
#include "test_pzexcel_p2.h"
#include <memory>
#include <string>
#include <filesystem>

#ifdef ENABLE_OFFICE
#include "pzexcel.h"
#include "pzzip.h"
#endif// ENABLE_OFFICE

namespace http
{

// =============================================================================
// 测试辅助：用 pzzip 直接构造含自定义 XML 的最小合法 xlsx 文件
// =============================================================================
#ifdef ENABLE_OFFICE

// --- 最小合法 xlsx 各条目模板（OOXML 基础，经 Office/LibreOffice 验证可打开） ---
static const char *kCT_TEMPLATE = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>)";

static const char *kROOT_RELS = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>)";

static const char *kWORKBOOK_RIDS_TEMPLATE =
    R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets><sheet name="Sheet1" sheetId="%d" r:id="rId1"/></sheets>
</workbook>)";

static const char *kWORKBOOK_RELS =
    R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>)";

static const char *kSTYLES_MIN = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="1"><font><sz val="11"/><color theme="1"/><name val="Calibri"/></font></fonts>
<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills>
<borders count="1"><border/></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>
</styleSheet>)";

static const char *kSHARED_EMPTY =
    R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="0" uniqueCount="0"/>
)";

// --- 把上面模板拼成一个完整的 zip 文件 ---
static bool build_min_xlsx(const std::string &path, const std::string &sheet_xml,
                           const std::string &shared_xml, const std::string &workbook_xml)
{
    pz::zip zf;
    if (!zf.create_zipfile(path))
        return false;
    zf.add_file_from_string("[Content_Types].xml", kCT_TEMPLATE);
    zf.add_file_from_string("_rels/.rels", kROOT_RELS);
    zf.add_file_from_string("xl/_rels/workbook.xml.rels", kWORKBOOK_RELS);
    if (workbook_xml.empty())
    {
        char buf[512];
        std::snprintf(buf, sizeof(buf), kWORKBOOK_RIDS_TEMPLATE, 1);
        zf.add_file_from_string("xl/workbook.xml", buf);
    }
    else
    {
        zf.add_file_from_string("xl/workbook.xml", workbook_xml);
    }
    zf.add_file_from_string("xl/styles.xml", kSTYLES_MIN);
    zf.add_file_from_string("xl/sharedStrings.xml", shared_xml);
    zf.add_file_from_string("xl/worksheets/sheet1.xml", sheet_xml);
    zf.close();
    return true;
}

// --- 构造 workbook.xml 带自定义 sheetId（用于 P0-1 测试 sheetId ≠ 文件名序号） ---
static std::string make_workbook_xml(unsigned int sheet_id)
{
    char buf[512];
    std::snprintf(buf, sizeof(buf), kWORKBOOK_RIDS_TEMPLATE, sheet_id);
    return buf;
}

// --- 生成 N 个 cellXfs 的 styles.xml（用于 P1-2 样式索引 >255 测试） ---
static std::string make_styles_xml_with_xfs(unsigned int num_xfs)
{
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml += "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
    xml += "<fonts count=\"1\"><font><sz val=\"11\"/><color theme=\"1\"/><name val=\"Calibri\"/></font></fonts>";
    xml += "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>";
    xml += "<borders count=\"1\"><border/></borders>";
    xml += "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>";
    xml += "<cellXfs count=\"" + std::to_string(num_xfs) + "\">";
    for (unsigned int i = 0; i < num_xfs; i++)
    {
        xml += "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>";
    }
    xml += "</cellXfs></styleSheet>";
    return xml;
}

// --- 把 styles.xml 写成 zip 文件（build_min_xlsx 默认只放 kSTYLES_MIN；
//     这里额外暴露带自定义 styles.xml 的版本，用于 P1-2 测试） ---
[[maybe_unused]]
static bool build_xlsx_with_styles(const std::string &path, const std::string &sheet_xml,
                                   const std::string &shared_xml, const std::string &styles_xml,
                                   const std::string &workbook_xml)
{
    pz::zip zf;
    if (!zf.create_zipfile(path))
        return false;
    zf.add_file_from_string("[Content_Types].xml", kCT_TEMPLATE);
    zf.add_file_from_string("_rels/.rels", kROOT_RELS);
    zf.add_file_from_string("xl/_rels/workbook.xml.rels", kWORKBOOK_RELS);
    zf.add_file_from_string("xl/workbook.xml", workbook_xml);
    zf.add_file_from_string("xl/styles.xml", styles_xml);
    zf.add_file_from_string("xl/sharedStrings.xml", shared_xml);
    zf.add_file_from_string("xl/worksheets/sheet1.xml", sheet_xml);
    zf.close();
    return true;
}

// --- 简单的 case 输出助手 ---
static void report_case(httppeer &client, const std::string &name, bool pass,
                        const std::string &expected, const std::string &actual)
{
    client << "<p>" << (pass ? "[PASS] " : "[FAIL] ") << name;
    if (!pass)
        client << " | expected=\"" << expected << "\" got=\"" << actual << "\"";
    client << "</p>";
}

static void report_case(httppeer &client, const std::string &name, bool pass)
{
    client << "<p>" << (pass ? "[PASS] " : "[FAIL] ") << name << "</p>";
}

#endif// ENABLE_OFFICE


// =============================================================================
// HTTP 测试入口
// =============================================================================
//@urlpath(null, test_pzexcel_p2)
std::string test_pzexcel_p2(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_OFFICE
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.config_path.size() < 5)
    {
        client << "<p> config_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.config_path);
    if (!file_conf.empty() && file_conf.back() != '/')
        file_conf.push_back('/');
    file_conf.append("docs/p2test/");
    std::filesystem::create_directories(file_conf);

    // --- Case 1: P0-3 修复验证 —— 带样式的空单元格在 write→read→write→read 后样式不丢失 ---
    client << "<p><b>=== Case 1: P0-3 styled-empty-cell roundtrip ===</b></p>";
    {
        std::string p03a = file_conf + "p03_a.xlsx";
        std::string p03b = file_conf + "p03_b.xlsx";

        pz::excel w1;
        w1.addSheet("Sheet1");
        w1.setCellColor("A1", "FFFF0000");     // A1 只有样式无值

        bool build_ok = w1.write(p03a);
        report_case(client, "Case1 write p03_a.xlsx", build_ok);

        if (build_ok)
        {
            pz::excel r1;
            bool read1 = r1.read(p03a);
            report_case(client, "Case1 read p03_a.xlsx", read1);

            unsigned int a1_s_type = 0;
            std::string a1_str     = "";
            if (read1 && r1.sheets.size() > 0)
            {
                r1.setActiveSheet(0);
                auto it = r1.sheets[0].values.find(1);
                if (it != r1.sheets[0].values.end())
                {
                    auto ci = it->second.find(1);
                    if (ci != it->second.end())
                    {
                        a1_s_type = ci->second.s_type;
                        a1_str    = r1.getCell<std::string>("A1");
                    }
                }
            }
            // A1 应该有 s_type > 0（样式保留），但 v_type 没设 → getCell 走 v_type<10 分支返回空串
            report_case(client, "Case1 p03_a: A1 s_type preserved", a1_s_type > 0, ">0",
                        std::to_string(a1_s_type));
            report_case(client, "Case1 p03_a: A1 value empty (expected for styled-only)", a1_str.empty(),
                        "", a1_str);

            // 第二次 write + read，确认样式不会在第二次 roundtrip 后丢失
            bool build2 = r1.write(p03b);
            report_case(client, "Case1 write p03_b.xlsx (2nd roundtrip)", build2);

            if (build2)
            {
                pz::excel r2;
                bool read2 = r2.read(p03b);
                report_case(client, "Case1 read p03_b.xlsx", read2);

                unsigned int a1_s2 = 0;
                if (read2 && r2.sheets.size() > 0)
                {
                    r2.setActiveSheet(0);
                    auto it = r2.sheets[0].values.find(1);
                    if (it != r2.sheets[0].values.end())
                    {
                        auto ci = it->second.find(1);
                        if (ci != it->second.end())
                            a1_s2 = ci->second.s_type;
                    }
                }
                report_case(client, "Case1 p03_b: A1 s_type still preserved after 2nd roundtrip", a1_s2 > 0,
                            ">0", std::to_string(a1_s2));
            }
        }
    }

    // --- Case 2: P1-1 修复验证 —— dimension ref="B2"（单格无冒号）正确解析 ---
    client << "<p><b>=== Case 2: P1-1 dimension single-cell ref ===</b></p>";
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="B2"/>
<sheetData>
<row r="2"><c r="B2"><v>99</v></c></row>
</sheetData>
</worksheet>)";

        std::string path = file_conf + "p11_dim_single.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, "");
        report_case(client, "Case2 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case2 read", ok);

            unsigned int rows = 0, cols = 0;
            std::string b2    = "";
            if (ok && r.sheets.size() > 0)
            {
                rows = r.sheets[0].rows;
                cols = r.sheets[0].cols;
                r.setActiveSheet(0);
                b2 = r.getCell<std::string>("B2");
            }
            report_case(client, "Case2 rows=2", rows == 2, "2", std::to_string(rows));
            report_case(client, "Case2 cols=2", cols == 2, "2", std::to_string(cols));
            report_case(client, "Case2 B2 value=99", b2 == "99", "99", b2);
        }
    }

    // --- Case 3: P0-1 修复验证 —— sheetId=99 ≠ sheet1.xml 文件名序号 ---
    client << "<p><b>=== Case 3: P0-1 sheetId mismatch vs filename ===</b></p>";
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData>
<row r="1"><c r="A1"><v>hello_p01</v></c></row>
</sheetData>
</worksheet>)";
        // workbook: sheet name="MyData" sheetId=99 r:id=rId1
        std::string wb = make_workbook_xml(99);
        // workbook.xml.rels 保持不变: rId1 → worksheets/sheet1.xml

        std::string path = file_conf + "p01_sheetid99.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, wb);
        report_case(client, "Case3 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case3 read", ok);

            if (ok)
            {
                // 应有 1 个 sheet，名称为 MyData（通过反表正确归属）
                report_case(client, "Case3 sheets.size()=1", r.sheets.size() == 1, "1",
                            std::to_string(r.sheets.size()));

                std::string name  = r.sheets.empty() ? "" : r.sheets[0].name;
                unsigned int sid  = r.sheets.empty() ? 0 : r.sheets[0].sheet_index;
                report_case(client, "Case3 sheet name=MyData", name == "MyData", "MyData", name);
                report_case(client, "Case3 sheet_index=99", sid == 99, "99", std::to_string(sid));

                r.setActiveSheet(0);
                std::string a1 = r.getCell<std::string>("A1");
                report_case(client, "Case3 A1 value preserved", a1 == "hello_p01", "hello_p01", a1);
            }
        }
    }

    // --- Case 4: P1-2 修复验证 —— cellXfs 第 300 条样式索引可正确读写 ---
    client << "<p><b>=== Case 4: P1-2 style index > 255 ===</b></p>";
    {
        // 构造 301 个 cellXfs（0..300），A1 用 s="300"
        std::string styles_xml = make_styles_xml_with_xfs(301);
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData>
<row r="1"><c r="A1" s="300"><v>42</v></c></row>
</sheetData>
</worksheet>)";

        std::string path = file_conf + "p12_style300.xlsx";
        pz::zip zf;
        bool built = false;
        if (zf.create_zipfile(path))
        {
            zf.add_file_from_string("[Content_Types].xml", kCT_TEMPLATE);
            zf.add_file_from_string("_rels/.rels", kROOT_RELS);
            zf.add_file_from_string("xl/_rels/workbook.xml.rels", kWORKBOOK_RELS);
            zf.add_file_from_string("xl/workbook.xml", make_workbook_xml(1));
            zf.add_file_from_string("xl/styles.xml", styles_xml);
            zf.add_file_from_string("xl/sharedStrings.xml", kSHARED_EMPTY);
            zf.add_file_from_string("xl/worksheets/sheet1.xml", sheet_xml);
            zf.close();
            built = true;
        }
        report_case(client, "Case4 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case4 read", ok);

            unsigned int a1_s_type = 0;
            std::string a1_val     = "";
            if (ok && r.sheets.size() > 0)
            {
                r.setActiveSheet(0);
                auto it = r.sheets[0].values.find(1);
                if (it != r.sheets[0].values.end())
                {
                    auto ci = it->second.find(1);
                    if (ci != it->second.end())
                        a1_s_type = ci->second.s_type;
                }
                a1_val = r.getCell<std::string>("A1");
            }
            // 关键断言：s_type 应为 300 而非 255（修复前截断到 255）
            report_case(client, "Case4 A1.s_type == 300", a1_s_type == 300, "300",
                        std::to_string(a1_s_type));
            report_case(client, "Case4 styles.size() >= 301", r.styles.size() >= 301, ">=301",
                        std::to_string(r.styles.size()));
            report_case(client, "Case4 A1 value=42", a1_val == "42", "42", a1_val);
        }
    }

    // --- Case 5: P2-1 缺口 —— <c> 无 r 属性，单元格被丢弃（已知 gap） ---
    client << "<p><b>=== Case 5: P2-1 cell without r attribute (KNOWN GAP) ===</b></p>";
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="B1"/>
<sheetData>
<row r="1">
  <c s="0"><v>100</v></c>
  <c r="B1"><v>200</v></c>
</row>
</sheetData>
</worksheet>)";
        std::string path = file_conf + "p21_no_r_attr.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, "");
        report_case(client, "Case5 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case5 read", ok);

            unsigned int cells = 0;
            std::string b1_val = "";
            if (ok && r.sheets.size() > 0)
            {
                r.setActiveSheet(0);
                for (auto &rp : r.sheets[0].values)
                    cells += rp.second.size();
                b1_val = r.getCell<std::string>("B1");
            }
            // 期望：B1 正常解析到值 "200"；A1 因缺 r 属性被丢弃（已知 gap）
            report_case(client, "Case5 B1 still parsed correctly (has r attr)", b1_val == "200", "200",
                        b1_val);
            report_case(client, "Case5 cells count >= 1", cells >= 1, ">=1",
                        std::to_string(cells));
            client << "<p><i>  (Known gap: the &lt;c&gt; without r= attribute is dropped because "
                      "row_num/col_num stay 0 and the store condition filters it out.)</i></p>";
        }
    }

    // --- Case 6: P2-2 缺口 —— <sheetData/> 自闭合空表 ---
    client << "<p><b>=== Case 6: P2-2 sheetData self-closing (KNOWN GAP) ===</b></p>";
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData/>
</worksheet>)";
        std::string path = file_conf + "p22_sheetdata_slash.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, "");
        report_case(client, "Case6 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case6 read", ok);

            unsigned int rows = 0, cols = 0;
            if (ok && r.sheets.size() > 0)
            {
                rows = r.sheets[0].rows;
                cols = r.sheets[0].cols;
            }
            // 无 row 数据时 dimension 给出的 rows/cols 仍应被解析
            report_case(client, "Case6 rows from dimension", rows == 1, "1", std::to_string(rows));
            report_case(client, "Case6 cols from dimension", cols == 1, "1", std::to_string(cols));
            client << "<p><i>  (Known gap: &lt;sheetData/&gt; self-closing prefix match "
                      "on \"&lt;sheetData\" fails for trailing / — but since no row data is present "
                      "anyway, dimension still supplies rows/cols.)</i></p>";
        }
    }

    // --- Case 7: P2-3 缺口 —— sharedStrings 富文本多 run ---
    client << "<p><b>=== Case 7: P2-3 sharedStrings rich-text runs (KNOWN GAP) ===</b></p>";
    {
        // index 0: 带两个 <r> 的富文本 "加粗普通"
        // index 1: 纯文本 "second"
        std::string shared_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="2" uniqueCount="2">
<si>
<r><rPr><b/></rPr><t>BoldPart</t></r>
<r><t>NormalPart</t></r>
</si>
<si><t>second</t></si>
</sst>)";
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A2"/>
<sheetData>
<row r="1"><c r="A1" t="s"><v>0</v></c></row>
<row r="2"><c r="A2" t="s"><v>1</v></c></row>
</sheetData>
</worksheet>)";

        std::string path = file_conf + "p23_rich_text.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, shared_xml, "");
        report_case(client, "Case7 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case7 read", ok);

            std::string a1 = "";
            std::string a2 = "";
            std::string s0 = "";
            std::string s1 = "";
            if (ok && r.sheets.size() > 0)
            {
                r.setActiveSheet(0);
                a1 = r.getCell<std::string>("A1");
                a2 = r.getCell<std::string>("A2");
                if (r.shared_strings.size() > 0) s0 = r.shared_strings[0];
                if (r.shared_strings.size() > 1) s1 = r.shared_strings[1];
            }
            // A1 期望得到 "BoldPartNormalPart"，当前实现已知 gap：后一个 run 覆盖前一个 → 得到 "NormalPart"
            report_case(client, "Case7 A2 simple text ok", a2 == "second", "second", a2);
            report_case(client, "Case7 sharedStrings[1] == \"second\"", s1 == "second", "second", s1);

            // s0 / A1：gap 判定
            client << "<p>A1 (rich-text) expected=\"BoldPartNormalPart\" got=\"" << a1 << "\"</p>";
            client << "<p>sharedStrings[0] expected=\"BoldPartNormalPart\" got=\"" << s0 << "\"</p>";
            // 已知 gap：temp_s_str.clear() 在每个 <t> inner 循环里，后 run 覆盖前 run
            bool gap_reproduced = (s0 == "NormalPart" || a1 == "NormalPart");
            report_case(client, "Case7 known gap reproduced (last run wins)", gap_reproduced);
            client << "<p><i>  (Known gap: process_shared_strings clears temp_s_str on every inner "
                      "&lt;t&gt; tag, so the last run's text overwrites earlier runs.)</i></p>";
        }
    }

    // --- Case 8: P2-4 缺口 —— t="b"/"t="d"/"t="e" 类型 ---
    client << "<p><b>=== Case 8: P2-4 t=\"b\"/\"t=\"d\"/\"t=\"e\" types (KNOWN GAP) ===</b></p>";
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="D1"/>
<sheetData>
<row r="1">
<c r="A1" t="b"><v>1</v></c>
<c r="B1" t="b"><v>0</v></c>
<c r="C1" t="d"><v>2024-01-15T10:00:00Z</v></c>
<c r="D1" t="e"><v>#VALUE!</v></c>
</row>
</sheetData>
</worksheet>)";
        std::string path = file_conf + "p24_misc_types.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, "");
        report_case(client, "Case8 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case(client, "Case8 read", ok);

            std::string a1 = "", b1 = "", c1 = "", d1 = "";
            unsigned int a1_t = 0, c1_t = 0, d1_t = 0;
            if (ok && r.sheets.size() > 0)
            {
                r.setActiveSheet(0);
                a1 = r.getCell<std::string>("A1");
                b1 = r.getCell<std::string>("B1");
                c1 = r.getCell<std::string>("C1");
                d1 = r.getCell<std::string>("D1");

                auto get_type = [&](const char *ref) -> unsigned int {
                    auto [rn, cn] = r.cellstr_to_num(ref);
                    auto it = r.sheets[0].values.find(rn);
                    if (it != r.sheets[0].values.end())
                    {
                        auto ci = it->second.find(cn);
                        if (ci != it->second.end())
                            return ci->second.t_type;
                    }
                    return 0;
                };
                a1_t = get_type("A1");
                c1_t = get_type("C1");
                d1_t = get_type("D1");
            }
            // 布尔 t="b"：v_type 走整数路径 → getCell 得到 "1"/"0"（正确值，但类型信息丢失）
            report_case(client, "Case8 A1 boolean TRUE value=1", a1 == "1", "1", a1);
            report_case(client, "Case8 B1 boolean FALSE value=0", b1 == "0", "0", b1);
            report_case(client, "Case8 A1 t_type stored as 'b'", a1_t == 'b', "'b'",
                        std::string(1, static_cast<char>(a1_t)));

            // 日期 t="d"：ISO 8601 字符串 parse_ll 返回 0 → "0"（已知 gap）
            report_case(client, "Case8 C1 ISO-8601 NOT correctly parsed (KNOWN GAP)", c1 == "0", "0", c1);
            report_case(client, "Case8 C1 t_type stored as 'd'", c1_t == 'd', "'d'",
                        std::string(1, static_cast<char>(c1_t)));

            // 错误 t="e"：parse_ll("#VALUE!") 返回 0 → "0"（已知 gap）
            report_case(client, "Case8 D1 error string NOT correctly returned (KNOWN GAP)",
                        d1 == "0", "0", d1);
            report_case(client, "Case8 D1 t_type stored as 'e'", d1_t == 'e', "'e'",
                        std::string(1, static_cast<char>(d1_t)));

            client << "<p><i>  (Known gaps: t=\"d\" ISO-8601 dates and t=\"e\" error strings are "
                      "fed through parse_ll which returns 0; t_type is preserved but "
                      "getCell has no branch to interpret it.)</i></p>";
        }
    }

    // --- Case 9: P0-2 修复验证 —— read external → write → re-read 不损坏 ---
    client << "<p><b>=== Case 9: P0-2 write roundtrip keeps xlsx openable ===</b></p>";
    {
        // 接 Case 3 已创建的 p01_sheetid99.xlsx，sheet_index=99
        std::string p01_path = file_conf + "p01_sheetid99.xlsx";
        std::string p01_back = file_conf + "p01_back.xlsx";

        pz::excel r1;
        bool ok = r1.read(p01_path);
        report_case(client, "Case9 read p01_sheetid99.xlsx", ok);

        if (ok && !r1.sheets.empty())
        {
            bool w = r1.write(p01_back);
            report_case(client, "Case9 write p01_back.xlsx", w);

            if (w)
            {
                pz::excel r2;
                bool ok2 = r2.read(p01_back);
                report_case(client, "Case9 re-read p01_back.xlsx after roundtrip", ok2);

                if (ok2)
                {
                    r2.setActiveSheet(0);
                    std::string a1 = r2.getCell<std::string>("A1");
                    report_case(client, "Case9 A1 preserved across read→write→read",
                                a1 == "hello_p01", "hello_p01", a1);

                    // 写出的 sheetId 应是数组序号（1），不再是旧的 99（因为 write 侧统一用 i+1）
                    unsigned int sid = r2.sheets[0].sheet_index;
                    report_case(client, "Case9 re-read sheet_index is array ordinal (1)",
                                sid == 1, "1", std::to_string(sid));
                }
            }
        }
    }

    client << "<hr><p><b>P2 test suite finished.</b></p>";

#else
    client << "<p>Please: cmake .. -DENABLE_OFFICE=ON </p>";
#endif// ENABLE_OFFICE

    return "";
}

}// namespace http
