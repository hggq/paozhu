#ifndef __WEB_PAY_QR_H__
#define __WEB_PAY_QR_H__

#include <string>

namespace http
{
namespace webpay
{

// 二维码输出两种形态：SVG（内联 HTML，默认）和 PNG（落盘，k_qr_png_enabled 开关）。
// PNG 文件名必须按单号隔离：固定名会让两个人同时扫同一个码 ⇒ 资金错配。
// PNG 启用时还要在支付成功落账后调 qr_png_remove 删文件。
inline constexpr bool k_qr_png_enabled = false;

// 未编译 ENABLE_IMAGE 时返回空串。
std::string qr_svg(const std::string &text);

// PNG 落盘路径：<sitepath>/upload/qr_<wxorder>.png（返回文件系统路径，不是 URL）。
std::string qr_png_path(const std::string &sitepath, const std::string &wxorder);

// 存 PNG。成功与否吃 save() 返回值，不吃 exists()（上一单残留会骗过去）。
// wxorder 只接受 [0-9A-Za-z_-]{1,64}：直接进文件名，含 '/' 或 ".." 能爬到 upload/ 外写文件。
bool qr_png_save(const std::string &sitepath, const std::string &wxorder, const std::string &text);

// 支付成功落账后删 PNG。文件不存在是正常分支（JSAPI/小程序本来就不生成）。
void qr_png_remove(const std::string &sitepath, const std::string &wxorder);

} // namespace webpay
}//namespace http
#endif
