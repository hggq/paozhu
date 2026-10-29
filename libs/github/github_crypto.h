#ifndef __GITHUB_CRYPTO_H__
#define __GITHUB_CRYPTO_H__

#include <string>

namespace http
{
namespace github
{

// 配置里的 master key：必须恰好 64 个 hex 字符，解出 32 字节（AES-256 的钥匙长度）。
// 短了补零、长了截断都会让同一把"配错的钥匙"照样能加解密，代价是每一行 client_secret
// 都解不开且没人报错。合格时 key_out 被填成 32 字节二进制并返回 true。
// 这两个函数都不打日志：密文与明文都可能从它们的栈帧里过一遍。
bool crypto_key_from_hex(const std::string &hex, std::string &key_out);

// AES-256-GCM 加密。输出布局固定为 IV(12) + tag(16) + ciphertext，解密按偏移切分。
// IV 每次用 RAND_bytes 重新生成，绝不复用——同一把钥匙下 IV 重复会直接打掉 GCM 的认证性。
// key 长度不是 32 字节、或随机源失败时抛 std::runtime_error；调用方据此拒写，不降级成明文。
std::string gcm_encrypt(const std::string &plaintext, const std::string &key);

// gcm_encrypt 的逆。长度不合法、tag 校验不过都抛 std::runtime_error（篡改过的密文解出明文
// 等于加密形同虚设，这一条不能只是"返回空串"）。
std::string gcm_decrypt(const std::string &cipher, const std::string &key);

// 落库前的 last_error 文本闸：把 gho_/ghp_/ghu_/ghs_/ghr_ 前缀和 client_secret= /
// access_token= / refresh_token= 这类"键名=值"片段连同其值换成 [redacted]，再截到列宽。
// GitHub 的 error_description 会把你提交的东西原样带回来，所以这一刀必须在写库之前。
std::string scrub_error_text(const std::string &in);

} // namespace github
} // namespace http
#endif
