#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include "webpay_cert_gate.h"

namespace http
{
namespace webpay
{

bool apiv3_key_decryptable(const std::string &key, std::string &why)
{
    static const char *fixed_why = "本地 api_v3_key 不可用（未配置，或长度不是 32 字节）";
    if (key.size() != 32)
    {
        why = fixed_why; // 定值文案，不含密钥或实际长度（会进回调应答体）
        return false;
    }
    return true;
}

namespace
{
// 冷却表：商户号 → 下次允许去网关试的单调秒。
std::mutex g_cert_mtx;
std::map<std::string, unsigned long long> g_cert_next_try;
} // namespace

unsigned long long steady_seconds()
{
    return (unsigned long long)std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool cert_fetch_allowed(const std::string &mch_id, unsigned long long now_sec)
{
    std::lock_guard<std::mutex> lock(g_cert_mtx);
    auto it = g_cert_next_try.find(mch_id);
    return it == g_cert_next_try.end() || now_sec >= it->second;
}

void cert_fetch_report(const std::string &mch_id, unsigned long long now_sec, bool ok)
{
    std::lock_guard<std::mutex> lock(g_cert_mtx);
    if (ok)
    {
        g_cert_next_try.erase(mch_id);
        return;
    }
    g_cert_next_try[mch_id] = now_sec + cert_fetch_cooldown_sec;
}

cert_fetch_plan plan_cert_fetch(bool local_cert_present, const std::string &api_v3_key,
                                const std::string &mch_id, unsigned long long now_sec,
                                std::string &why)
{
    why.clear();
    if (local_cert_present) return cert_fetch_plan::use_local;

    std::string keywhy;
    if (!apiv3_key_decryptable(api_v3_key, keywhy))
    {
        why = "本地无平台证书，未向微信发起下载：" + keywhy;
        return cert_fetch_plan::key_unusable;
    }
    if (!cert_fetch_allowed(mch_id, now_sec))
    {
        why = "本地无平台证书，下载在冷却窗口内（同一商户号失败后 " +
              std::to_string(cert_fetch_cooldown_sec) + " 秒内不重试）";
        return cert_fetch_plan::cooling_down;
    }
    return cert_fetch_plan::fetch;
}

namespace
{
// 序列号是网关给的值，回调侧那个还来自请求头。带分隔符就不许拼进路径。
bool cert_serial_usable(const std::string &serial)
{
    if (serial.empty()) return false;
    if (serial.find("..") != std::string::npos) return false;
    return serial.find('/') == std::string::npos && serial.find('\\') == std::string::npos;
}
} // namespace

std::string platform_cert_path(const std::string &conf_file, const std::string &serial)
{
    if (!cert_serial_usable(serial)) return "";
    std::error_code ec;
    std::filesystem::path dir = conf_file.empty()
                                    ? std::filesystem::path("conf")
                                    : std::filesystem::absolute(conf_file, ec).parent_path();
    if (ec || dir.empty()) return "";// 连 conf 目录都定不出来 ⇒ 当成"没有这条路径"，不猜 cwd
    return (dir / ("wechatpay_" + serial + ".pem")).lexically_normal().string();
}

std::string local_platform_cert(const std::string &conf_file, const std::string &serial)
{
    std::string path = platform_cert_path(conf_file, serial);
    return file_exists_regular(path) ? path : std::string();
}

bool file_is_readable(const std::string &path)
{
    // 先要"是个普通文件"：macOS/Linux 上 ifstream 打开目录也报 good()，
    // 光看流会把配置里写错成目录的路径判成"能读"，到取正文那一刻才失败。
    if (!file_exists_regular(path)) return false;
    std::ifstream f(path, std::ios::binary);
    return f.good();
}

bool credential_readable(const std::string &path_or_pem)
{
    if (path_or_pem.find("-----BEGIN") != std::string::npos) return true;
    return file_is_readable(path_or_pem);
}

bool file_exists_regular(const std::string &path)
{
    if (path.empty()) return false;
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

bool write_text_file(const std::string &path, const std::string &text)
{
    if (path.empty()) return false;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out << text;
    out.close();
    return !out.fail();
}

} // namespace webpay
} // namespace http
