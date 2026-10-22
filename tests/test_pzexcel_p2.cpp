/*
 * test_pzexcel_p2.cpp — pzexcel xlsx 读写 P0/P1 修复验证 + P2 已知缺口确认
 *
 * 用 pzzip 直接构造含自定义 XML 的最小合法 xlsx，覆盖：
 *   P0-1  sheetId ≠ 文件名序号时的 sheet 归属（workbook_mapfiles 反表）
 *   P0-2  write 路径 content-types / workbook_rels / sheet 文件名统一用数组序号
 *   P0-3  自闭合样式空单元格 read→write→read 闭环不丢样式
 *   P1-1  dimension ref="B2"（单格无冒号）正确解析
 *   P1-2  cellXfs 第 300 条样式索引不截断到 255
 *   已知 gap：无 r 属性单元格 / sheetData 自闭合 / rich-text 多 run / t="b"/"d"/"e"
 *
 * 输出：每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 *
 * 编译：cmake -DENABLE_TESTS=ON -DENABLE_OFFICE=ON ..
 */

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include "pzexcel.h"
#include "pzzip.h"

static int g_failures = 0;
static int g_cases    = 0;

static void report_case(const std::string &name, bool pass,
                        const std::string &expected, const std::string &actual)
{
    g_cases++;
    if (pass)
    {
        std::cout << "[PASS] " << name << std::endl;
    }
    else
    {
        g_failures++;
        std::cout << "[FAIL] " << name << " | expected=\"" << expected
                  << "\" got=\"" << actual << "\"" << std::endl;
    }
}

static void report_case(const std::string &name, bool pass)
{
    report_case(name, pass, "true", pass ? "true" : "false");
}

// =============================================================================
// 测试辅助：用 pzzip 直接构造含自定义 XML 的最小合法 xlsx 文件
// =============================================================================

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
<sheets><sheet name="%s" sheetId="%d" r:id="rId1"/></sheets>
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

// 把上面模板拼成一个完整的 zip 文件
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
        std::snprintf(buf, sizeof(buf), kWORKBOOK_RIDS_TEMPLATE, "Sheet1", 1);
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

// 构造 workbook.xml 带自定义 name + sheetId
static std::string make_workbook_xml(const char *name, unsigned int sheet_id)
{
    char buf[512];
    std::snprintf(buf, sizeof(buf), kWORKBOOK_RIDS_TEMPLATE, name, sheet_id);
    return buf;
}

// 生成 N 个 cellXfs 的 styles.xml（用于 P1-2 样式索引 >255 测试）
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

// =============================================================================
// 测试入口
// =============================================================================

int main()
{
    const std::string file_conf = "./docs/p2test/";
    std::filesystem::create_directories(file_conf);

    // --- Case 1: P0-3 styled-empty-cell roundtrip ---
    std::cout << "\n=== Case 1: P0-3 styled-empty-cell roundtrip ===" << std::endl;
    {
        std::string p03a = file_conf + "p03_a.xlsx";
        std::string p03b = file_conf + "p03_b.xlsx";

        pz::excel w1;
        w1.addSheet("Sheet1");
        w1.setCellColor("A1", "FFFF0000");

        bool build_ok = w1.write(p03a);
        report_case("Case1 write p03_a.xlsx", build_ok);

        if (build_ok)
        {
            pz::excel r1;
            bool read1 = r1.read(p03a);
            report_case("Case1 read p03_a.xlsx", read1);

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
            report_case("Case1 p03_a: A1 s_type preserved", a1_s_type > 0, ">0",
                        std::to_string(a1_s_type));
            report_case("Case1 p03_a: A1 value empty (expected for styled-only)",
                        a1_str.empty(), "", a1_str);

            bool build2 = r1.write(p03b);
            report_case("Case1 write p03_b.xlsx (2nd roundtrip)", build2);

            if (build2)
            {
                pz::excel r2;
                bool read2 = r2.read(p03b);
                report_case("Case1 read p03_b.xlsx", read2);

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
                report_case("Case1 p03_b: A1 s_type still preserved after 2nd roundtrip",
                            a1_s2 > 0, ">0", std::to_string(a1_s2));
            }
        }
    }

    // --- Case 2: P1-1 dimension single-cell ref ---
    std::cout << "\n=== Case 2: P1-1 dimension single-cell ref ===" << std::endl;
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
        report_case("Case2 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case2 read", ok);

            unsigned int rows = 0, cols = 0;
            std::string b2    = "";
            if (ok && r.sheets.size() > 0)
            {
                rows = r.sheets[0].rows;
                cols = r.sheets[0].cols;
                r.setActiveSheet(0);
                b2 = r.getCell<std::string>("B2");
            }
            report_case("Case2 rows=2", rows == 2, "2", std::to_string(rows));
            report_case("Case2 cols=2", cols == 2, "2", std::to_string(cols));
            report_case("Case2 B2 value=99", b2 == "99", "99", b2);
        }
    }

    // --- Case 3: P0-1 sheetId mismatch vs filename ---
    std::cout << "\n=== Case 3: P0-1 sheetId mismatch vs filename ===" << std::endl;
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData>
<row r="1"><c r="A1" t="s"><v>0</v></c></row>
</sheetData>
</worksheet>)";
        std::string shared_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="1" uniqueCount="1">
<si><t>hello_p01</t></si>
</sst>)";
        std::string wb = make_workbook_xml("MyData", 99);

        std::string path = file_conf + "p01_sheetid99.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, shared_xml, wb);
        report_case("Case3 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case3 read", ok);

            if (ok)
            {
                report_case("Case3 sheets.size()=1", r.sheets.size() == 1, "1",
                            std::to_string(r.sheets.size()));

                std::string name = r.sheets.empty() ? "" : r.sheets[0].name;
                unsigned int sid = r.sheets.empty() ? 0 : r.sheets[0].sheet_index;
                report_case("Case3 sheet name=MyData", name == "MyData", "MyData", name);
                report_case("Case3 sheet_index=99", sid == 99, "99", std::to_string(sid));

                r.setActiveSheet(0);
                std::string a1 = r.getCell<std::string>("A1");
                report_case("Case3 A1 value preserved", a1 == "hello_p01", "hello_p01", a1);
            }
        }
    }

    // --- Case 4: P1-2 style index > 255 ---
    std::cout << "\n=== Case 4: P1-2 style index > 255 ===" << std::endl;
    {
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
            zf.add_file_from_string("xl/workbook.xml", make_workbook_xml("Sheet1", 1));
            zf.add_file_from_string("xl/styles.xml", styles_xml);
            zf.add_file_from_string("xl/sharedStrings.xml", kSHARED_EMPTY);
            zf.add_file_from_string("xl/worksheets/sheet1.xml", sheet_xml);
            zf.close();
            built = true;
        }
        report_case("Case4 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case4 read", ok);

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
            report_case("Case4 A1.s_type == 300", a1_s_type == 300, "300",
                        std::to_string(a1_s_type));
            report_case("Case4 styles.size() >= 301", r.styles.size() >= 301, ">=301",
                        std::to_string(r.styles.size()));
            report_case("Case4 A1 value=42", a1_val == "42", "42", a1_val);
        }
    }

    // --- Case 5: P2-1 cell without r attribute (KNOWN GAP) ---
    std::cout << "\n=== Case 5: P2-1 cell without r attribute (KNOWN GAP) ===" << std::endl;
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
        report_case("Case5 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case5 read", ok);

            unsigned int cells = 0;
            std::string b1_val = "";
            if (ok && r.sheets.size() > 0)
            {
                r.setActiveSheet(0);
                for (auto &rp : r.sheets[0].values)
                    cells += rp.second.size();
                b1_val = r.getCell<std::string>("B1");
            }
            report_case("Case5 B1 still parsed correctly (has r attr)", b1_val == "200", "200",
                        b1_val);
            report_case("Case5 cells count >= 1", cells >= 1, ">=1",
                        std::to_string(cells));
            std::cout << "  (Known gap: the <c> without r= attribute is dropped because "
                         "row_num/col_num stay 0 and the store condition filters it out.)"
                      << std::endl;
        }
    }

    // --- Case 6: P2-2 sheetData self-closing (KNOWN GAP) ---
    std::cout << "\n=== Case 6: P2-2 sheetData self-closing (KNOWN GAP) ===" << std::endl;
    {
        std::string sheet_xml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<dimension ref="A1"/>
<sheetData/>
</worksheet>)";
        std::string path = file_conf + "p22_sheetdata_slash.xlsx";
        bool built = build_min_xlsx(path, sheet_xml, kSHARED_EMPTY, "");
        report_case("Case6 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case6 read", ok);

            unsigned int rows = 0, cols = 0;
            if (ok && r.sheets.size() > 0)
            {
                rows = r.sheets[0].rows;
                cols = r.sheets[0].cols;
            }
            report_case("Case6 rows from dimension", rows == 1, "1", std::to_string(rows));
            report_case("Case6 cols from dimension", cols == 1, "1", std::to_string(cols));
            std::cout << "  (Known gap: <sheetData/> self-closing prefix match on \"<sheetData\" "
                         "fails for trailing / — but since no row data is present anyway, "
                         "dimension still supplies rows/cols.)"
                      << std::endl;
        }
    }

    // --- Case 7: P2-3 sharedStrings rich-text runs (KNOWN GAP) ---
    std::cout << "\n=== Case 7: P2-3 sharedStrings rich-text runs (KNOWN GAP) ===" << std::endl;
    {
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
        report_case("Case7 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case7 read", ok);

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
            report_case("Case7 A2 simple text ok", a2 == "second", "second", a2);
            report_case("Case7 sharedStrings[1] == \"second\"", s1 == "second", "second", s1);

            std::cout << "A1 (rich-text) expected=\"BoldPartNormalPart\" got=\"" << a1 << "\""
                      << std::endl;
            std::cout << "sharedStrings[0] expected=\"BoldPartNormalPart\" got=\"" << s0 << "\""
                      << std::endl;

            bool gap_reproduced = (s0 == "NormalPart" || a1 == "NormalPart");
            report_case("Case7 known gap reproduced (last run wins)", gap_reproduced);
            std::cout << "  (Known gap: process_shared_strings clears temp_s_str on every inner "
                         "<t> tag, so the last run's text overwrites earlier runs.)"
                      << std::endl;
        }
    }

    // --- Case 8: P2-4 t="b"/"t="d"/"t="e" types (KNOWN GAP) ---
    std::cout << "\n=== Case 8: P2-4 t=\"b\"/\"t=\"d\"/\"t=\"e\" types (KNOWN GAP) ===" << std::endl;
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
        report_case("Case8 build xlsx", built);

        if (built)
        {
            pz::excel r;
            bool ok = r.read(path);
            report_case("Case8 read", ok);

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
            report_case("Case8 A1 boolean TRUE value=1", a1 == "1", "1", a1);
            report_case("Case8 B1 boolean FALSE value=0", b1 == "0", "0", b1);
            report_case("Case8 A1 t_type stored as 'b'", a1_t == 'b', "'b'",
                        std::string(1, static_cast<char>(a1_t)));

            report_case("Case8 C1 ISO-8601 value is NOT full timestamp (KNOWN GAP)",
                        c1 != "2024-01-15T10:00:00Z", "NOT \"2024-01-15...\"", c1);
            report_case("Case8 C1 t_type stored as 'd'", c1_t == 'd', "'d'",
                        std::string(1, static_cast<char>(c1_t)));

            report_case("Case8 D1 error string NOT correctly returned (KNOWN GAP)",
                        d1 == "0", "0", d1);
            report_case("Case8 D1 t_type stored as 'e'", d1_t == 'e', "'e'",
                        std::string(1, static_cast<char>(d1_t)));

            std::cout << "  (Known gaps: t=\"d\" ISO-8601 dates and t=\"e\" error strings are "
                         "fed through parse_ll which returns 0; t_type is preserved but "
                         "getCell has no branch to interpret it.)"
                      << std::endl;
        }
    }

    // --- Case 9: P0-2 write roundtrip keeps xlsx openable ---
    std::cout << "\n=== Case 9: P0-2 write roundtrip keeps xlsx openable ===" << std::endl;
    {
        std::string p01_path = file_conf + "p01_sheetid99.xlsx";
        std::string p01_back = file_conf + "p01_back.xlsx";

        pz::excel r1;
        bool ok = r1.read(p01_path);
        report_case("Case9 read p01_sheetid99.xlsx", ok);

        if (ok && !r1.sheets.empty())
        {
            bool w = r1.write(p01_back);
            report_case("Case9 write p01_back.xlsx", w);

            if (w)
            {
                pz::excel r2;
                bool ok2 = r2.read(p01_back);
                report_case("Case9 re-read p01_back.xlsx after roundtrip", ok2);

                if (ok2)
                {
                    r2.setActiveSheet(0);
                    std::string a1 = r2.getCell<std::string>("A1");
                    report_case("Case9 A1 preserved across read→write→read",
                                a1 == "hello_p01", "hello_p01", a1);

                    unsigned int sid = r2.sheets[0].sheet_index;
                    report_case("Case9 re-read sheet_index is array ordinal (1)",
                                sid == 1, "1", std::to_string(sid));
                }
            }
        }
    }

    // --- SUMMARY ---
    std::cout << "\n=== SUMMARY: " << (g_cases - g_failures) << "/" << g_cases
              << " passed, " << g_failures << " failed ===" << std::endl;
    return g_failures == 0 ? 0 : 1;
}
