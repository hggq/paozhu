#include "httppeer.h"
#include "serverconfig.h"
#include "server_localvar.h"
#include "test_pzzip.h"
#include "func.h"
#include <memory>
#include <string>
#ifdef ENABLE_ZIP
#include "pzzip.h"
#endif// ENABLE_ZIP
namespace http
{
namespace fs = std::filesystem;
#define PZZIPCHECK(cond)                                  \
    do                                                    \
    {                                                     \
        if (cond)                                         \
        {                                                 \
            client << "<p>  [PASS] " << #cond << "</p>";  \
        }                                                 \
        else                                              \
        {                                                 \
            client << "<p>   [FAIL] " << #cond << "</p>"; \
        }                                                 \
    } while (0)

// 验证文件内容一致
bool files_equal_zip(const std::string &path1, const std::string &path2)
{
    std::ifstream f1(path1, std::ios::binary | std::ios::ate);
    std::ifstream f2(path2, std::ios::binary | std::ios::ate);
    if (!f1.is_open() || !f2.is_open())
        return false;

    auto size1 = f1.tellg();
    auto size2 = f2.tellg();
    if (size1 != size2)
        return false;

    std::vector<char> d1(static_cast<size_t>(size1));
    std::vector<char> d2(static_cast<size_t>(size2));

    f1.seekg(0);
    f2.seekg(0);
    f1.read(d1.data(), d1.size());
    f2.read(d2.data(), d2.size());

    return d1 == d2;
}

//@urlpath(null,test_pzzip)
std::string test_pzzip(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
#ifdef ENABLE_ZIP
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

    client << "xlsx 解压→重压缩→再解压 比对";
    std::string nnfile  = file_conf + "nn.xlsx";
    std::string nnnfile = file_conf + "nnn.xlsx";
    fs::current_path(fs::absolute(nnfile).parent_path());
    // 1. 解压 nn.xlsx
    {
        pz::zip zip;
        PZZIPCHECK(zip.open_zipfile(nnfile));
        auto files = zip.files_list();
        client << "  nn.xlsx 包含 " << files.size() << " 个文件:" << "<br />";
        for (const auto &f : files)
        {
            client << "    - " << f << "<br />";
        }
        PZZIPCHECK(files.size() > 0);

        fs::remove_all(file_conf + "extract_nn");
        PZZIPCHECK(zip.extract_all(file_conf + "extract_nn"));
        zip.close();
    }

    // 2. 重新压缩为 mm.xlsx（保留目录结构）
    {
        pz::zip zip;
        PZZIPCHECK(zip.create_zipfile(nnnfile));

        // 遍历 extract_nn 目录，添加所有文件
        for (const auto &entry : fs::recursive_directory_iterator(file_conf + "extract_nn"))
        {
            if (entry.is_regular_file())
            {
                std::string disk_path   = entry.path().string();
                std::string stored_name = fs::relative(entry.path(), file_conf + "extract_nn").string();
                PZZIPCHECK(zip.add_file(disk_path, stored_name));
            }
        }
        zip.close();
        PZZIPCHECK(fs::exists(nnnfile));
    }

    // 3. 解压 mm.xlsx，与 extract_nn 比对
    {
        pz::zip zip;
        PZZIPCHECK(zip.open_zipfile(nnnfile));
        auto files = zip.files_list();
        client << "  mm.xlsx 包含 " << files.size() << " 个文件:" << "<br />";
        for (const auto &f : files)
        {
            client << "    - " << f << "<br />";
        }

        fs::remove_all(file_conf + "extract_mm");
        PZZIPCHECK(zip.extract_all(file_conf + "extract_mm"));
        zip.close();

        // 逐个比对文件
        for (const auto &entry : fs::recursive_directory_iterator(file_conf + "extract_nn"))
        {
            if (entry.is_regular_file())
            {
                std::string rel     = fs::relative(entry.path(), file_conf + "extract_nn").string();
                std::string path_mm = file_conf + "extract_mm/" + rel;
                bool eq             = files_equal_zip(entry.path().string(), path_mm);
                if (!eq)
                {
                    client << "  [FAIL] 文件不一致: " << rel << "<br />";
                }
                PZZIPCHECK(eq);
            }
        }
    }

    fs::remove_all(file_conf + "extract_nn");
    fs::remove_all(file_conf + "extract_mm");
    fs::remove(file_conf + "nnn.xlsx");

    client << "<p>Successfully wrote nnn.xlsx</p>";
#else
    client << "<p>Please: cmake .. -DENABLE_ZIP=ON </p>";
#endif// ENABLE_ZIP

    return "";
}

}// namespace http