/*
 * test_filename_safety.cpp — 文件名安全函数单元测试
 * 覆盖：safe_filename / str2safefile / get_filename 三个公开安全函数。
 * strip_crlf 在各 TU 是 static inline（httppeer / httpclient / http_rpcclient / http_websocket_client），
 * 本测试不直接覆盖——它是 erase(remove_if(\r\n)) 的一行逻辑，行为一眼可证。
 *
 * 输出：每组一行 PASS/FAIL，末行 SUMMARY；全过退出码 0，任一失败退出码 1。
 *
 * 编译：cmake 自动发现，func.cpp 通过 target_sources 编入。
 */

#include <iostream>
#include <string>
#include <string_view>
#include "func.h"

using namespace http;

static int g_failures = 0;
static int g_groups   = 0;

static void group_result(const char *name, bool ok)
{
    g_groups++;
    if (ok)
    {
        std::cout << "PASS: " << name << std::endl;
    }
    else
    {
        g_failures++;
        std::cout << "FAIL: " << name << std::endl;
    }
}

// safe_filename 的预期行为：basename + 空字节跳过 + 反斜杠视作分隔符 + 保留扩展名
static void test_safe_filename()
{
    std::cout << "\n=== safe_filename ===" << std::endl;

    // 路径穿越：正斜杠
    group_result("path traversal ../../etc/passwd → passwd",
                 safe_filename("../../etc/passwd") == "passwd");

    // 路径穿越：Windows 反斜杠（修复前不认，现应视作分隔符）
    group_result("path traversal backslash → passwd",
                 safe_filename("..\\..\\etc\\passwd") == "passwd");

    // 路径穿越：绝对路径
    group_result("absolute path /etc/passwd → passwd",
                 safe_filename("/etc/passwd") == "passwd");

    // 正常文件名：保留扩展名
    group_result("normal file avatar.png → avatar.png",
                 safe_filename("avatar.png") == "avatar.png");

    // 多层目录下的正常文件
    group_result("upload/2024/05/report.pdf → report.pdf",
                 safe_filename("upload/2024/05/report.pdf") == "report.pdf");

    // 空字节截断防御
    std::string with_nul = std::string("foo.png") + '\0' + "malicious.sh";
    group_result("nul byte → 跳过空字节，basename 后取末段",
                 safe_filename(with_nul) == "malicious.sh");

    // 隐藏文件：合法，basename 后保留
    group_result("hidden file .bashrc → .bashrc",
                 safe_filename(".bashrc") == ".bashrc");

    // 纯 "." 或 ".."：边界 case，basename 后只剩它们（可拼 prefix 不越界）
    group_result("dot only . → .",
                 safe_filename(".") == ".");
    group_result("dotdot only .. → ..",
                 safe_filename("..") == "..");

    // 多扩展名：全部保留（只有 get_filename 才拆扩展名）
    group_result("multi-ext foo.tar.gz → foo.tar.gz",
                 safe_filename("foo.tar.gz") == "foo.tar.gz");

    // 中文文件名：保留（和 str2safefile 白名单方案的关键差异）
    group_result("unicode → 保留",
                 safe_filename("中文密码.png") == "中文密码.png");

    // 空输入
    group_result("empty → empty",
                 safe_filename("").empty());
}

// str2safefile 的预期行为：白名单过滤 + 空字节跳过
static void test_str2safefile()
{
    std::cout << "\n=== str2safefile ===" << std::endl;

    // 路径穿越：/ \ . 都不在白名单（. 在白名单但会被 basename 方案处理；这里 str2safefile 是纯白名单）
    group_result("path traversal ../../etc/passwd → etcpasswd",
                 str2safefile("../../etc/passwd", 17) == "etcpasswd");

    group_result("path traversal backslash → etcpasswd",
                 str2safefile("..\\..\\etc\\passwd", 17) == "etcpasswd");

    // 正常文件名
    std::string t_normal = "avatar.png";
    group_result("normal file avatar.png → avatar.png",
                 str2safefile(t_normal.c_str(), t_normal.size()) == "avatar.png");

    // 多扩展名
    std::string t_multi = "foo.tar.gz";
    group_result("multi-ext foo.tar.gz → foo.tar.gz",
                 str2safefile(t_multi.c_str(), t_multi.size()) == "foo.tar.gz");

    // 空字节防御
    std::string with_nul = std::string("foo.png") + '\0' + "bar";
    group_result("nul byte → 被跳过",
                 str2safefile(with_nul.c_str(), with_nul.size()) == "foobar");

    // 中文：白名单里没有 UTF-8 字节，全清（这是 str2safefile 和 safe_filename 的设计差异）
    std::string t_unicode = "中文密码.png";   // 中×3 + .png = 12 + 4 = 16 bytes
    group_result("unicode → 白名单外，被清光只剩 .png",
                 str2safefile(t_unicode.c_str(), t_unicode.size()) == ".png");

    // 空输入
    group_result("empty → empty",
                 str2safefile("", 0).empty());

    // 空格不在白名单
    std::string t_space = "foo bar.txt";
    group_result("space → 被清",
                 str2safefile(t_space.c_str(), t_space.size()) == "foobar.txt");
}

// get_filename：拆扩展名 + 反斜杠修复验证（之前版本不认 \）
static void test_get_filename()
{
    std::cout << "\n=== get_filename ===" << std::endl;

    // 反斜杠分隔符（修复前不认，会返回 ..\\..\\etc；现应取最后一段）
    group_result("backslash path → passwd",
                 get_filename("..\\..\\etc\\passwd") == "passwd");

    // 正斜杠路径 + 拆扩展名
    group_result("avatar.png → avatar (ext stripped)",
                 get_filename("avatar.png") == "avatar");

    // .tar.gz 特例：连续 . 不清
    group_result("foo.tar.gz → 取主名 foo",
                 get_filename("foo.tar.gz") == "foo");

    // 纯 ".tar.gz"（无主名）
    group_result("weird name .tar.gz → 空（主名被清）",
                 get_filename(".tar.gz").empty());

    // 空字节防御：空字节跳过不影响 '.' 识别，最终返回 basename 不含扩展名
    std::string with_nul_g = std::string("foo.png") + '\0' + "bar";
    group_result("nul byte → foo（空字节跳过，.png 作 ext）",
                 get_filename(with_nul_g) == "foo");

    // 多层目录
    group_result("upload/2024/avatar.png → avatar",
                 get_filename("upload/2024/avatar.png") == "avatar");

    // 空输入
    group_result("empty → empty",
                 get_filename("").empty());
}

int main()
{
    test_safe_filename();
    test_str2safefile();
    test_get_filename();

    std::cout << "\n=== SUMMARY: " << (g_groups - g_failures) << "/" << g_groups << " passed ===" << std::endl;
    return g_failures == 0 ? 0 : 1;
}
