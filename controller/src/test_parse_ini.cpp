#include "serverconfig.h"
#include "server_localvar.h"
#include "httppeer.h"
#include "test_parse_ini.h"
#include "parse_ini.h"
#include <memory>
#include <string>

namespace http
{
//@urlpath(null,test_parse_ini)
std::string test_parse_ini(std::shared_ptr<httppeer> peer)
{
    httppeer &client                   = peer->get_peer();
    server_loaclvar &static_server_var = get_server_global_var();

    if (static_server_var.config_path.size() < 5)
    {
        client << " static_server_var.config_path empty ";
        return "";
    }

    std::string file_conf = static_server_var.config_path;

    if (file_conf.size() > 0 && file_conf.back() != '/')
    {
        file_conf.push_back('/');
    }
    file_conf.append("test.conf");

    std::ofstream out(file_conf);
    if (!out.is_open())
    {
        client << "Failed to create " << file_conf;
        return "";
    }
    out << "[default]\n";
    out << "wwwpath = /www/paozhu/www/default ;项目目录\n\n";
    out << "[www.abc.com]\n";
    out << ";域名web的根目录\n";
    out << "wwwpath = /www/paozhu/www/abc ;域名目录\n";
    out.close();

    // 辅助函数：打印文件内容到web
    auto print_file = [](const std::string &filename, std::shared_ptr<httppeer> peer)
    {
        httppeer &client = peer->get_peer();
        std::ifstream in(filename);
        if (!in.is_open())
        {
            client << "Cannot open " << filename << "\n";
            return;
        }
        std::string line;
        client << "<p>=== Content of " << filename << " ===</p>\n";
        while (std::getline(in, line))
        {
            client << "<p>" << line << "</p>\n";
        }
        client << "<p>===================================</p>\n";
        in.close();
    };

    print_file(file_conf, peer);

    parse_ini ini(file_conf);

    client << "Parsed test.conf.\n";

    // 3. 执行添加和删除操作
    client << "\n--- Adding values ---\n";
    bool ret;

    ret = ini.add_value("default", "siteid", "1");
    client << "Add siteid=1 to [default]: " << (ret ? "OK" : "Failed") << "<br>";

    ret = ini.add_value("www.abc.com", "siteid", "2");
    client << "Add siteid=2 to [www.abc.com]: " << (ret ? "OK" : "Failed") << "<br>";

    ret = ini.add_value("www.abc.com", "themes", "flash");
    client << "Add themes=flash to [www.abc.com]: " << (ret ? "OK" : "Failed") << "<br>";

    ret = ini.add_value("www.abc.com", "csspath", "flash");
    client << "Add csspath=flash to [www.abc.com]: " << (ret ? "OK" : "Failed") << "<br>";

    std::cout << "<br>\n--- After additions ---\n<br>";
    print_file(file_conf, peer);

    // 4. 删除 csspath
    client << "<br>\n--- Deleting csspath ---\n<br>";
    ret = ini.delete_value("www.abc.com", "csspath");
    client << "Delete csspath from [www.abc.com]: " << (ret ? "OK" : "Failed") << "\n<br>";

    client << "<br>\n--- Final file content ---\n<br>";
    print_file(file_conf, peer);

    // 5. 验证内存中的配置
    client << "<br>\n--- Memory config verification ---\n<br>";
    client << "default::siteid = " << ini.get_value("default", "siteid") << "\n<br>";
    client << "www.abc.com::siteid = " << ini.get_value("www.abc.com", "siteid") << "\n<br>";
    client << "www.abc.com::themes = " << ini.get_value("www.abc.com", "themes") << "\n<br>";
    client << "www.abc.com::csspath = " << ini.get_value("www.abc.com", "csspath", "(not found)") << "\n<br>";

    client << "<p>----raw key name-----</p>";
    for(auto[first,second]:ini.config)
    {
        client << "<p>----begin-----</p>";
        client << "<p>|"<<first<<"|</p>";
        client << "<p>------------begin-2----</p>";
        for(auto[aaaa,bbbb]:second)
        {
            client << "<p>|"<<aaaa<<":"<<bbbb<<"|</p>";
        }
        client << "<p>------------end-2----</p>";
        client << "<p>----end-----</p>";
    }

    return "";
}

}// namespace http