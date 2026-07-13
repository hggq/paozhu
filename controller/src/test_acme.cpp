#include "serverconfig.h"
#include "server_localvar.h"
#include "httppeer.h"
#include "test_acme.h"
#include "acme_run.h"
#include "acme_config.h"
#include "acme_client.h"
#include "http_domain.h"
#include "acme_crypto.hpp"
#include <memory>
#include <string>
#include <filesystem>

namespace http
{
namespace fs = std::filesystem;

//@urlpath(null,test_acme)
std::string test_acme(std::shared_ptr<httppeer> peer)
{
    httppeer &client                   = peer->get_peer();
    server_loaclvar &static_server_var = get_server_global_var();

    client << "<html><head><meta charset=\"utf-8\"><title>ACME 证书申请</title>";
    client << "<style>body{font-family:monospace;margin:20px;background:#f5f5f5;}";
    client << "h2{color:#333;} pre{background:#fff;padding:10px;border-radius:5px;overflow-x:auto;}";
    client << "a{color:#0066cc;}</style></head><body>";

    // 获取域名参数
    std::string domain = client.get["domain"].to_string();
    std::string email  = client.get["email"].to_string();

    if (domain.empty())
    {
        client << "<h2>ACME 证书申请测试</h2>";
        client << "<p>用法: <a href=\"/test_acme?domain=www.example.com\">";
        client << "/test_acme?domain=www.example.com</a></p>";
        client << "<p>可选参数: email=your@email.com</p>";
        client << "<p>示例: <a href=\"/test_acme?domain=www.example.com&email=admin@example.com\">";
        client << "/test_acme?domain=www.example.com&email=admin@example.com</a></p>";
        client << "<hr><p>流程说明:</p>";
        client << "<pre>";
        client << "1. 加载 conf/acme.conf 配置\n";
        client << "2. 检查 cert.pem 是否存在且未过期 (>10天)\n";
        client << "3. 若需申请，执行完整 ACME 流程\n";
        client << "   - 获取 EAB 凭证 (自动或从配置读取)\n";
        client << "   - 目录发现 -> 创建账号 -> 新建订单\n";
        client << "   -> HTTP-01 挑战 -> 提交 CSR -> 下载证书\n";
        client << "4. 保存 eab_kid/eab_hmac_key 回 acme.conf\n";
        client << "</pre>";
        client << "</body></html>";
        return "";
    }

    // 获取配置文件路径
    std::string conf_path = static_server_var.config_path;
    if (conf_path.size() > 0 && conf_path.back() != '/')
    {
        conf_path.push_back('/');
    }
    std::string acme_conf_file = conf_path + "acme.conf";

    client << "<h2>ACME 证书申请: " << domain << "</h2>";
    client << "<pre>";

    // 创建 ACME 运行器
    acme_run acme;

    // 设置配置路径
    acme.set_acme_config_path(conf_path);

    // 设置邮箱 (如果 URL 参数提供了)
    if (!email.empty())
    {
        acme.set_email(email);
    }

    // 加载 acme.conf
    client << "[加载配置] " << acme_conf_file << "\n";
    if (!acme.load_acme_config(acme_conf_file))
    {
        client << "[错误] 无法加载配置文件: " << acme_conf_file << "\n";
        client << "请确保 conf/acme.conf 文件存在并配置了 email 和 acme_path\n";
        client << "</pre></body></html>";
        return "";
    }
    // 设置 webroot (用于 HTTP-01 验证)
    std::string webroot = "/www/acme/www/xcxtiyu";

    if (!std::filesystem::exists(webroot))
    {
        client << "<p>[配置信息]:acme.set_webroot(\""<<webroot<<"\");</p>";
        client << "<p>必须设置为你实际域名www目录</p>";
        return "";
    }

    if (!std::filesystem::exists(acme.acme_path))
    {
        client << "<p>[配置信息]:conf/acme.conf [info] acme_path=;</p>";
        client << "<p>必须设置为你保存证书目录，一般设置为项目下 acme </p>";
        return "";
    }

    acme.set_webroot(webroot);

    // 设置域名
    acme.domains.push_back(domain);

    client << "[配置信息]\n";
    client << "  email:      " << acme.email << "\n";
    client << "  acme_path:  " << acme.acme_path << "\n";
    client << "  server_url: " << acme.acme_ini.get_server_url() << "\n";
    client << "  eab_kid:    " << (acme.acme_ini.has_eab_credentials() ? "(已配置)" : "(未配置)") << "\n";
    client << "\n";

    // 检查证书有效期
    std::string primary_domain = domain;
    acme.output_dir = acme.acme_path + "/" + primary_domain;

    try
    {
        cert_validity_t cv = acme.check_domain_cert_pem(primary_domain);
        if (!cv.is_expired)
        {
            client << "[证书检查] 剩余天数: " << cv.days_remaining << "\n";

            if (cv.days_remaining > acme.days_remain)
            {
                client << "\n证书仍在有效期内 (剩余 " << cv.days_remaining << " 天)，无需申请\n";
                client << "</pre></body></html>";
                return "";
            }
            client << "证书即将过期，开始重新申请...\n\n";
        }
        else
        {
            client << "[证书检查] 证书不存在或已过期，开始申请...\n\n";
        }
    }
    catch (const std::exception &e)
    {
        client << "[警告] 检查证书失败: " << e.what() << "\n";
        client << "将继续申请新证书...\n\n";
    }

    // 执行 ACME 申请流程
    client << "[开始申请] 请查看服务器终端输出获取详细日志...\n";

    bool ok = acme.run();

    client << "\n";
    if (ok)
    {
        client << "[结果] 证书申请成功!\n";
        client << "\n证书文件保存在: " << acme.output_dir << "\n";
        client << "  account_key.pem  - ACME 账号私钥\n";
        client << "  privkey.pem      - 证书私钥\n";
        client << "  cert.pem         - 服务器证书\n";
        client << "  chain.pem        - CA 中间证书链\n";
        client << "  fullchain.pem    - 证书 + CA 链\n";
    }
    else
    {
        client << "[结果] 证书申请失败! 请检查服务器终端日志\n";
    }

    client << "</pre>";
    client << "<hr><a href=\"/test_acme\">返回</a>";
    client << "</body></html>";

    return "";
}

} // namespace http
