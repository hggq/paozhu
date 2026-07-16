#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzexcel.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_MINIZIP
#include "pzexcel.h"
#endif// ENABLE_MINIZIP
namespace http
{
//@urlpath(null,test_pzexcel)
std::string test_pzexcel(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_MINIZIP
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.config_path.size() < 5)
    {
        client << "<p> static_server_var.config_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.config_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/");

    std::string zipfile;

    zipfile = file_conf + "tt.xlsx";

    client << "<p>=== Reading " << zipfile << " ===</p></p>";

    pz::excel pza;
    if (!pza.read(zipfile))
    {
        client << "<p>Read error: " << pza.error_msg << "</p>";
        return "";
    }

    client << "<p>\n---------------------\n</p>";
    client << "<p>pz_excel shared_strings size:" << pza.shared_strings.size() << "</p>";
    client << "<p>\n---------------------\n</p>";
    for (unsigned int i = 0; i < pza.shared_strings.size(); i++)
        client << i << ":" << pza.shared_strings[i] << "</p>";

    client << "<p>\n---------------------\n</p>";
    client << "<p>pz_excel sheets size:" << pza.sheets.size() << "</p>";
    client << "<p>\n---------------------\n</p><pre>";

    for (unsigned int i = 0; i < pza.sheets.size(); i++)
    {
        client << i << ":" << pza.sheets[i].name;
        client << i << ":" << pza.sheets[i].sheet_index;

        client << i << "max_row:" << pza.sheets[i].rows;
        client << i << "max_col:" << pza.sheets[i].cols;
    }

    client << "</pre><p>\n---------------------\n</p>";
    for (auto &[first, second] : pza.workbook_mapfiles)
        client << first << " " << second;

    pza.setActiveSheet(1);
    client << "<p>A1: " << pza.getCell<std::string>("A1") << "</p>";
    client << "<p>B2: " << pza.getCell<std::string>("B2") << "</p>";
    client << "<p>B4: " << pza.getCell<std::string>("B4") << "</p>";
    client << "<p>C4: " << pza.getCell<std::string>("C4") << "</p>";

    client << "<p>A5: " << pza.getCell<double>("A5") << "</p>";
    client << "<p>B5: " << pza.getCell<double>("B5") << "</p>";

    client << "<p>\n=== Writing qq.xlsx ===</p>";

    pz::excel pzw;
    pzw.setCell("A1", std::string("姓名"));
    pzw.setCell("B1", std::string("年龄"));
    pzw.setCell("C1", std::string("成绩"));
    pzw.setCell("A2", std::string("张三"));
    pzw.setCell("B2", 25);
    pzw.setCell("C2", 95.5);
    pzw.setCell("A3", std::string("李四"));
    pzw.setCell("B3", 30);
    pzw.setCell("C3", 88.75);
    pzw.setCell("A4", std::string("王五"));
    pzw.setCell("B4", 28);
    pzw.setCell("C4", 92.0);

    pzw.addSheet("Sheet2");
    pzw.setActiveSheet(1);
    pzw.setCell("A1", std::string("第二页数据"));
    pzw.setCell("B2", 12345);
    pzw.setCell("C3", 3.14159);

    std::string qqfile = file_conf + "qq.xlsx";
    if (pzw.write(qqfile))
    {
        client << "<p>Successfully wrote qq.xlsx</p>";
    }
    else
    {
        client << "<p>Write error: " << pzw.error_msg << "</p>";
        return "";
    }

    client << "<p>\n=== Verifying qq.xlsx by reading back ===</p>";
    pz::excel pzv;
    if (pzv.read(qqfile))
    {
        client << "<p>Sheets: " << pzv.sheets.size() << "</p>";
        for (unsigned int i = 0; i < pzv.sheets.size(); i++)
        {
            client << "<p>Sheet " << i + 1 << ": " << pzv.sheets[i].name << ", rows=" << pzv.sheets[i].rows << ", cols=" << pzv.sheets[i].cols << "</p>";
        }
        pzv.setActiveSheet(0);
        client << "<p>A1: " << pzv.getCell<std::string>("A1") << "</p>";
        client << "<p>B2: " << pzv.getCell<int>("B2") << "</p>";
        client << "<p>C2: " << pzv.getCell<double>("C2") << "</p>";
        pzv.setActiveSheet(1);
        client << "<p>Sheet2 A1: " << pzv.getCell<std::string>("A1") << "</p>";
        client << "<p>Sheet2 B2: " << pzv.getCell<int>("B2") << "</p>";
    }
    else
    {
        client << "<p>Verify error: " << pzv.error_msg << "</p>";
    }

    client << "<p>\n=== Reading mm.xlsx and writing mmm.xlsx with setCellColor ===</p>";
    pz::excel pzm;
    std::string mmfile = file_conf + "mm.xlsx";
    if (pzm.read(mmfile))
    {
        client << "<p>Successfully read mm.xlsx" << "</p>";
        client << "<p>Sheets: " << pzm.sheets.size() << "</p>";
        client << "<p>Shared strings: " << pzm.shared_strings.size() << "</p>";
        client << "<p>Fonts count: " << pzm.fonts.size() << "</p>";
        client << "<p>Styles count: " << pzm.styles.size() << "</p>";

        pzm.setActiveSheet(0);
        client << "<p>A1: " << pzm.getCell<std::string>("A1") << "</p>";
        client << "<p>B2: " << pzm.getCell<std::string>("B2") << "</p>";

        pzm.setCellColor("A1", "FFFF0000");
        pzm.setCellColor("B2", "FF00FF00");
        pzm.setCellColor("B4", "FF0000FF");
        pzm.setCellColor("C4", "FFFF2278");

        std::string mmmfile = file_conf + "mmm.xlsx";
        if (pzm.write(mmmfile))
        {
            client << "<p>Successfully wrote mmm.xlsx" << "</p>";

            pz::excel pzverify;
            if (pzverify.read(mmmfile))
            {
                client << "<p>Verified mmm.xlsx read back ok</p>";
                client << "<p>Fonts count: " << pzverify.fonts.size() << "</p>";
                client << "<p>Styles count: " << pzverify.styles.size() << "</p>";
            }
        }
        else
        {
            client << "<p>Write mmm.xlsx error: " << pzm.error_msg << "</p>";
        }
    }
    else
    {
        client << "<p>Read mm.xlsx error: " << pzm.error_msg << "</p>";
    }

    client << "<p>\n=== Reading kk.xlsx (LibreOffice) and writing kk_out.xlsx ===</p>";
    {
        pz::excel pzk;
        std::string kkfile = file_conf + "kk.xlsx";
        if (pzk.read(kkfile))
        {
            client << "<p>Read kk.xlsx OK, shared strings: " << pzk.shared_strings.size() << "</p>";
            client << "<p>A1: " << pzk.getCell<std::string>("A1") << "</p>";
            client << "<p>B2: " << pzk.getCell<std::string>("B2") << "</p>";
            client << "<p>B4: " << pzk.getCell<std::string>("B4") << "</p>";
            client << "<p>C4: " << pzk.getCell<std::string>("C4") << "</p>";
            client << "<p>A5: " << pzk.getCell<double>("A5") << "</p>";
            client << "<p>B5: " << pzk.getCell<double>("B5") << "</p>";

            pzk.setCellColor("A1", "FF0000");
            pzk.setCellColor("B4", "0000FF");

            std::string kkoutfile = file_conf + "kk_out.xlsx";
            if (pzk.write(kkoutfile))
            {
                client << "<p>Wrote kk_out.xlsx OK</p>";
                pz::excel v;
                if (v.read(kkoutfile))
                {
                    client << "<p>Read back kk_out.xlsx OK</p>";
                    client << "<p>A1: " << v.getCell<std::string>("A1") << "</p>";
                    client << "<p>B4: " << v.getCell<std::string>("B4") << "</p>";
                    client << "<p>Fonts with color: </p>";
                    for (size_t i = 0; i < v.fonts.size(); i++)
                    {
                        if (v.fonts[i].color_type == 10)
                            client << "<p>[" << v.fonts[i].color << "] </p>";
                    }
                }
            }
        }
        else
        {
            client << "<p>Read kk.xlsx error: " << pzk.error_msg << "</p>";
        }
    }
#else
    client << "<p>Please: cmake .. -DENABLE_MINIZIP=ON </p>";
#endif// ENABLE_MINIZIP

    return "";
}

}// namespace http