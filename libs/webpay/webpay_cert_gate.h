#ifndef __WEB_PAY_CERT_GATE_H__
#define __WEB_PAY_CERT_GATE_H__
// 有意不加 ENABLE_WEBPAY，也不引支付 SDK：只判长度、时钟和进程内冷却表，
// 关闭支付编译时也要能单独编译、单独判。

#include <string>

namespace http
{
namespace webpay
{

// APIv3 密钥能否用于解密平台证书（长度 32 字节）。why 返回定值文案，不含密钥或实际长度。
bool apiv3_key_decryptable(const std::string &key, std::string &why);

// 下载平台证书的冷却窗口（秒）：同一商户号失败后 300 秒内不再去网关重试。
constexpr unsigned int cert_fetch_cooldown_sec = 300;

// 单调时钟秒数（不用墙上时钟，避免 NTP 回拨问题）。
unsigned long long steady_seconds();

// 当前商户号能否去下载（只查询，不消耗额度）。
bool cert_fetch_allowed(const std::string &mch_id, unsigned long long now_sec);

// 报告下载结果：成功清冷却，失败把下次允许时间推到 now_sec + cert_fetch_cooldown_sec。
void cert_fetch_report(const std::string &mch_id, unsigned long long now_sec, bool ok);

// 证书下载决策三闸合一（本地证书在位 > 密钥可用 > 冷却窗口 > fetch）。
enum class cert_fetch_plan
{
    use_local,    // 本地已有序列号对应证书，不发
    key_unusable, // APIv3 密钥不合规，发了也解不出
    cooling_down, // 冷却窗口内
    fetch         // 三道闸放行
};

cert_fetch_plan plan_cert_fetch(bool local_cert_present, const std::string &api_v3_key,
                                const std::string &mch_id, unsigned long long now_sec,
                                std::string &why);

// ---- 证书（以及其它凭据文件）在盘上的位置与读写 ----
// 有意不加 ENABLE_WEBPAY：路径与文件读写跟支付 SDK 无关。
// 写证书那一侧和读证书那一侧必须用同一套拼法，否则下载成功而回调读不到；
// 所以路径规则只有 platform_cert_path() 这一处实现。

// 平台证书路径。锚在"已加载的那份 webpay.conf 所在目录"，cwd 不参与（daemon 模式下 cwd
// 未必是项目根）。conf_file 为空 = 配置没加载，回落成 conf/ 下的相对路径，让页面文案有得指。
// serial 里带目录分隔符或 ".." 时给空串 —— 证书不许落到 conf 目录以外，
// 调用方把空串当"没有这张证书"处理。
std::string platform_cert_path(const std::string &conf_file, const std::string &serial);

// 回调读侧：本地这张证书在位就返回同一个路径，不在返回空串。与 platform_cert_path 同源。
std::string local_platform_cert(const std::string &conf_file, const std::string &serial);

// 普通文件且真打得开。stat 失败（父目录没权限、路径超长）当作"不在"，不抛。
bool file_is_readable(const std::string &path);

// 凭据可读 = 要么是 PEM 正文（含 "-----BEGIN"），要么是可读文件路径。商户私钥/证书
// 在 conf 里两种形态都支持，判"能不能用"必须先过这一条。
// 有意不改 file_is_readable：V2 侧按纯文件语义在用它。
bool credential_readable(const std::string &path_or_pem);

// 只是"是不是个普通文件"（不打开）。给需要再读正文内容的调用方用，免得先判后开两趟。
bool file_exists_regular(const std::string &path);

// 整份覆盖写（证书落盘用）。false = 没写开或没写完，具体文案由调用方出（页面/应答各不同）。
bool write_text_file(const std::string &path, const std::string &text);
}  // namespace webpay
}  // namespace http

#endif
