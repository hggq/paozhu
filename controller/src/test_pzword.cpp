#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzword.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_OFFICE
#include "pz_word.h"
#endif// ENABLE_OFFICE
namespace http
{
//@urlpath(null,test_pzword)
std::string test_pzword(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_OFFICE
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

    zipfile = file_conf + "ww.docx";

    client << "<p>=== Reading " << zipfile << " ===</p></p>";

    pz::word pzw;
    if (!pzw.read(zipfile))
    {
        client << "Read error: " << pzw.error_msg;
        client << "Trying to read from unzipped directory...";

        if (!pzw.read_from_unzipped("."))
        {
            client << "Read from unzipped error: " << pzw.error_msg;
            return "";
        }
        else
        {
            client << "Read from unzipped directory success";
        }
    }
    else
    {
        client << "Read from docx file success";
    }

    std::string html = pzw.to_html();

    std::ofstream html_file(file_conf + "word.html");
    if (html_file)
    {
        html_file << html;
        html_file.close();
        client << "\n=== HTML saved to word.html ===";
    }
    else
    {
        client << "Error: Unable to write word.html";
    }

    pz::word pzww;
    if (!pzww.read_html(file_conf + "word.html"))
    {
        client << "Read HTML error: " << pzww.error_msg;
        return "";
    }

    if (!pzww.write(file_conf + "www.docx"))
    {
        client << "Write DOCX error: " << pzww.error_msg;
        return "";
    }
    client << "<p>Successfully wrote www.docx</p>";
    client << html;
#else
    client << "<p>Please: cmake .. -DENABLE_OFFICE=ON </p>";
#endif// ENABLE_OFFICE

    return "";
}

}// namespace http