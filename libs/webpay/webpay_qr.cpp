#include "webpay_qr.h"
#include <filesystem>
#ifdef ENABLE_IMAGE
#include "qrcode.h"
#include "pzpng.h"
#endif

namespace http
{
namespace webpay
{

std::string qr_svg(const std::string &text)
{
#ifdef ENABLE_IMAGE
    qr::qrcode q;
    q.text(text, qr::Ecc::M, 1);

    const int border = 2;
    const int n      = q.size > 0 ? q.size : 0;
    if (n == 0) return "";

    std::string s;
    s.reserve(2048 + (size_t)n * n * 8);
    s.append("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"232\" height=\"232\" viewBox=\"0 0 ");
    s.append(std::to_string(n + border * 2));
    s.append(" ");
    s.append(std::to_string(n + border * 2));
    s.append("\" shape-rendering=\"crispEdges\" style=\"background:#fff;border:1px solid #ddd\">");
    s.append("<rect width=\"100%\" height=\"100%\" fill=\"#ffffff\"/><g fill=\"#000000\">");

    for (int y = 0; y < n; y++)
    {
        for (int x = 0; x < n; x++)
        {
            if (!q.at(x, y)) continue;
            s.append("<rect x=\"");
            s.append(std::to_string(x + border));
            s.append("\" y=\"");
            s.append(std::to_string(y + border));
            s.append("\" width=\"1\" height=\"1\"/>");
        }
    }
    s.append("</g></svg>");
    return s;
#else
    (void)text;
    return "";
#endif
}

// 路径拼接，不校验 wxorder —— 校验在 save/remove 入口（真写真删的地方）。
std::string qr_png_path(const std::string &sitepath, const std::string &wxorder)
{
    return sitepath + "/upload/qr_" + wxorder + ".png";
}

// wxorder 进文件名，防路径穿越。单号是自己生成的，这层守卫防的是以后有人把请求参数直接递进来。
static bool qr_png_name_ok(const std::string &wxorder)
{
    if (wxorder.empty() || wxorder.size() > 64) return false;
    for (char c : wxorder)
    {
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-')
            continue;
        return false;
    }
    return true;
}

bool qr_png_save(const std::string &sitepath, const std::string &wxorder, const std::string &text)
{
#ifdef ENABLE_IMAGE
    if (!qr_png_name_ok(wxorder)) return false;

    qr::qrcode q;
    q.text(text, qr::Ecc::M, 1);
    if (q.size <= 0) return false;

    image::png img;
    unsigned char scale = 10;
    img.create(q.width() * scale + 40, q.height() * scale + 40, 6, 8);
    img.fillColor({255, 255, 255, 255});
    img.qrdata(q.data, q.width(), q.height(), scale, 20, 20);

    namespace fs                    = std::filesystem;
    const std::string fullpath      = qr_png_path(sitepath, wxorder);
    const fs::path upload_dir       = fs::path(fullpath).parent_path();
    // 全走 error_code 重载：HTTP 路由里调磁盘操作，stat/mkdir/chmod 任何异常都不该飞出去。
    std::error_code fec;
    if (!upload_dir.empty() && !fs::exists(upload_dir, fec))
    {
        fec.clear();
        fs::create_directories(upload_dir, fec);
        if (!fec)
        {
            fs::permissions(upload_dir,
                            fs::perms::owner_all | fs::perms::group_all | fs::perms::others_read,
                            fs::perm_options::add, fec);
        }
    }
    return img.save(fullpath);
#else
    (void)sitepath;
    (void)wxorder;
    (void)text;
    return false;
#endif
}

void qr_png_remove(const std::string &sitepath, const std::string &wxorder)
{
    if (!qr_png_name_ok(wxorder)) return;
    std::error_code ec;
    std::filesystem::remove(qr_png_path(sitepath, wxorder), ec); // 不抛：没这文件是正常分支
}

} // namespace webpay
}//namespace http
