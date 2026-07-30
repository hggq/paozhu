#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_webpdf.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_PDF
#include "webpdf.h"
#endif// ENABLE_PDF
namespace http
{
namespace fs = std::filesystem;
//@urlpath(null,test_webpdf)
std::string test_webpdf(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_PDF
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.log_path.size() < 5)
    {
        client << "<p> static_server_var.log_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.log_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/");

    if (peer->pathinfos.size() == 2 && peer->pathinfos[1] == "cn")
    {
        try
        {
            std::string html_file = file_conf + "webpdf_tutorial.html";
            std::string pdf_file  = file_conf + "webpdf_tutorial.pdf";
            // OTF (bare CID-keyed CFF) subsetting path. content_otf.html uses
            // the family name "SourceHanSerifSC-Light" directly.
            std::string font_file = "AlibabaPuHuiTi-Light.otf";
            std::string font_name = "AlibabaPuHuiTi-Light";

            fs::path html_path = fs::absolute(html_file);
            fs::path pdf_path  = fs::absolute(pdf_file);
            fs::path font_dir  = fs::absolute(file_conf + "font");

            std::ifstream f(html_file, std::ios::binary);
            if (!f.is_open())
            {
                client << "Cannot open file: " << html_file;
                return "";
            }

            // 获取文件大小
            f.seekg(0, std::ios::end);
            std::streamsize size = f.tellg();
            f.seekg(0, std::ios::beg);

            std::string html;
            html.resize(static_cast<size_t>(size));
            if (size > 0 && !f.read(html.data(), size))
            {
                client << "Failed to read file: " << html_file;
                return "";
            }

            auto pdf = std::make_unique<pz::webpdf>();
            pdf->setImagesPath(file_conf);
            pdf->AddPage();
            pdf->AddFont(font_name, "", font_file, font_dir.string());
            pdf->SetFont(font_name, "", 14);
            fs::current_path(fs::absolute(html_file).parent_path());
            pdf->WriteHTML(html);

            peer->type("application/pdf");
            pdf->Output("F", pdf_path.string());
            peer->output = pdf->Output("D", pdf_path.string());
            //peer->output = pdf->move_result();
        }
        catch (const std::exception &e)
        {
            client << "Error: " << e.what();
            return "";
        }
    }
    else
    {
        try
        {
            std::string html_file = file_conf + "webpdf_tutorial_en.html";
            std::string pdf_file  = file_conf + "webpdf_tutorial_en.pdf";
            std::string font_file = "AlibabaSans-Regular.otf";
            std::string font_name = "AlibabaSans-Regular";

            // Resolve everything to absolute paths up front, before we change the
            // working directory to the HTML's location for relative asset lookups.
            fs::path html_path = fs::absolute(html_file);
            fs::path pdf_path  = fs::absolute(pdf_file);
            fs::path font_dir  = fs::absolute(file_conf + "font");

            std::ifstream f(html_file, std::ios::binary);
            if (!f.is_open())
            {
                client << "Cannot open file: " << html_file;
                return "";
            }

            // 获取文件大小
            f.seekg(0, std::ios::end);
            std::streamsize size = f.tellg();
            f.seekg(0, std::ios::beg);

            std::string html;
            html.resize(static_cast<size_t>(size));
            if (size > 0 && !f.read(html.data(), size))
            {
                client << "Failed to read file: " << html_file;
                return "";
            }

            auto pdf = std::make_unique<pz::webpdf>();
            pdf->setImagesPath(file_conf);
            pdf->AddFont(font_name, "", font_file, font_dir.string());
            pdf->SetFont(font_name, "", 14);

            // Resources (images, etc.) in the HTML are relative to the HTML file.
            //fs::current_path(html_path.parent_path());

            pdf->WriteHTML(html);
            peer->type("application/pdf");
            pdf->Output("F", pdf_path.string());
            peer->output = pdf->Output("D", pdf_path.string());
            //peer->output = pdf->move_result();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return "";
        }
    }
#else
    client << "<p>Please: cmake .. -DENABLE_PDF=ON </p>";
#endif// ENABLE_PDF

    return "";
}

//@urlpath(null,test_ttfpdf)
std::string test_otfpdf(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_PDF
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.log_path.size() < 5)
    {
        client << "<p> static_server_var.log_path empty </p>";
        return "";
    }

    std::string file_conf = dir_name(static_server_var.log_path);

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("docs/");

    if (peer->pathinfos.size() == 2 && peer->pathinfos[1] == "cn")
    {
        try
        {
            std::string html_file = file_conf + "webpdf_tutorial.html";
            std::string pdf_file  = file_conf + "webpdf_tutorial_ttf.pdf";
            // OTF (bare CID-keyed CFF) subsetting path. content_otf.html uses
            // the family name "SourceHanSerifSC-Light" directly.
            std::string font_file = "AlibabaPuHuiTi-Light.ttf";
            std::string font_name = "AlibabaPuHuiTi-Light";

            fs::path html_path = fs::absolute(html_file);
            fs::path pdf_path  = fs::absolute(pdf_file);
            fs::path font_dir  = fs::absolute(file_conf + "font");

            std::ifstream f(html_file, std::ios::binary);
            if (!f.is_open())
            {
                client << "Cannot open file: " << html_file;
                return "";
            }

            // 获取文件大小
            f.seekg(0, std::ios::end);
            std::streamsize size = f.tellg();
            f.seekg(0, std::ios::beg);

            std::string html;
            html.resize(static_cast<size_t>(size));
            if (size > 0 && !f.read(html.data(), size))
            {
                client << "Failed to read file: " << html_file;
                return "";
            }

            auto pdf = std::make_unique<pz::webpdf>();
            pdf->setImagesPath(file_conf);
            pdf->AddPage();
            pdf->AddFont(font_name, "", font_file, font_dir.string());
            pdf->SetFont(font_name, "", 14);

            // Resources (images, etc.) in the HTML are relative to the HTML file.
            fs::current_path(html_path.parent_path());

            pdf->WriteHTML(html);

            peer->type("application/pdf");
            pdf->Output("F", pdf_path.string());
            peer->output = pdf->Output("D", pdf_path.string());
            //peer->output = pdf->move_result();
        }
        catch (const std::exception &e)
        {
            client << "Error: " << e.what();
            return "";
        }
    }
    else
    {
        try
        {
            std::string html_file = file_conf + "webpdf_tutorial_en.html";
            std::string pdf_file  = file_conf + "webpdf_tutorial_en_ttf.pdf";
            std::string font_file = "AlibabaSans-Regular.ttf";
            std::string font_name = "AlibabaSans-Regular";

            // Resolve everything to absolute paths up front, before we change the
            // working directory to the HTML's location for relative asset lookups.
            fs::path html_path = fs::absolute(html_file);
            fs::path pdf_path  = fs::absolute(pdf_file);
            fs::path font_dir  = fs::absolute(file_conf + "font");

            std::ifstream f(html_file, std::ios::binary);
            if (!f.is_open())
            {
                client << "Cannot open file: " << html_file;
                return "";
            }

            // 获取文件大小
            f.seekg(0, std::ios::end);
            std::streamsize size = f.tellg();
            f.seekg(0, std::ios::beg);

            std::string html;
            html.resize(static_cast<size_t>(size));
            if (size > 0 && !f.read(html.data(), size))
            {
                client << "Failed to read file: " << html_file;
                return "";
            }

            auto pdf = std::make_unique<pz::webpdf>();
            pdf->setImagesPath(file_conf);
            pdf->AddFont(font_name, "", font_file, font_dir.string());
            pdf->SetFont(font_name, "", 14);

            // Resources (images, etc.) in the HTML are relative to the HTML file.
            fs::current_path(html_path.parent_path());

            pdf->WriteHTML(html);
            peer->type("application/pdf");
            pdf->Output("F", pdf_path.string());
            peer->output = pdf->Output("D", pdf_path.string());
            //peer->output = pdf->move_result();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return "";
        }
    }
#else
    client << "<p>Please: cmake .. -DENABLE_PDF=ON </p>";
#endif// ENABLE_PDF

    return "";
}

//@urlpath(null,test_table_linebreak)
std::string test_table_linebreak(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_PDF
    try
    {
        std::string html_content = R"(
<!DOCTYPE html>
<html>
<head>
<style>
table { border-collapse: collapse; width: 100%; }
td { border: 1px solid #000; padding: 5mm; font-size: 10pt; }
</style>
</head>
<body>
<table>
<tr><td>第一行<br>第二行<br>第三行</td></tr>
<tr><td>第一行\n第二行\n第三行</td></tr>
<tr><td><p>第一段内容</p><p>第二段内容</p></td></tr>
</table>
</body>
</html>
)";

        auto pdf = std::make_unique<pz::webpdf>();
        pdf->AddPage();
        pdf->SetFont("Helvetica", "", 12);
        pdf->WriteHTML(html_content);

        std::string output_path = "/Users/hzq/paozhu/temp/table_test.pdf";
        pdf->Output("F", output_path);

        client << "<p>PDF generated: " << output_path << "</p>";
        client << "<p><a href=\"/temp/table_test.pdf\" target=\"_blank\">Download PDF</a></p>";
    }
    catch (const std::exception &e)
    {
        client << "Error: " << e.what();
        return "";
    }
#else
    client << "<p>Please: cmake .. -DENABLE_PDF=ON </p>";
#endif// ENABLE_PDF

    return "";
}

}// namespace http