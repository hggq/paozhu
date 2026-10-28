/*
 * POST 表单 obj_val 访问语义探针
 * 对应 docx/request_h_post_form_validation_2026-10-27.md 的 V1/V2/V3/V4/V5/V8/V10/V11/V12
 *
 * 编译（由 tests/postform/run_probe.sh 调用，带 ASan）:
 *   clang++ -std=c++20 -g -fsanitize=address \
 *     -I vendor/httpserver/include vendor/httpserver/src/request.cpp \
 *     tests/test_post_form_probe.cpp -o probe
 *
 * ENABLE_TESTS=ON 时本文件由 tests/CMakeLists.txt 自动发现，request.cpp 由同名规则单独编入。
 *
 * 用法: ./probe <v1|v2|v3|v4|v5|v8|v10|v11|v12>
 * 退出码: 0 = 该用例通过, 1 = 失败, 2 = 未知用例
 */
#include "request.h"
#include <cstdio>
#include <string>

using namespace http;

static int g_fail = 0;

static void check(bool cond, const char *what)
{
    if (cond)
        std::printf("  PASS %s\n", what);
    else
    {
        std::printf("  FAIL %s\n", what);
        g_fail++;
    }
}

// V1: cc 多 key 时, [22] 必须命中文本 key "22", 不能按位置取第二个成员
static int case_v1()
{
    obj_val post;
    post["cc"]["22"].set_array();
    post["cc"]["22"].push(std::string("A"));
    post["cc"]["aabbcc"].set_array();
    post["cc"]["aabbcc"].push(std::string("B"));

    check(post["cc"][22][0].to_string() == "A", "post[\"cc\"][22][0] == A");
    check(post["cc"]["22"][0].to_string() == "A", "post[\"cc\"][\"22\"][0] == A");
    check(post["cc"][22].is_array(), "post[\"cc\"][22] is array");
    check(&post["cc"][22] == &post["cc"]["22"], "post[\"cc\"][22] and [\"22\"] same object");
    check(post["cc"]["aabbcc"][0].to_string() == "B", "aabbcc still reachable by text key");
    check(post["cc"].size() == 2, "cc has exactly 2 keys");
    return g_fail == 0 ? 0 : 1;
}

// V2: 顶层对象 post[1] == post["1"]
static int case_v2()
{
    obj_val post;
    post["1"] = std::string("a");
    post["2"] = std::string("b");

    check(post[1].to_string() == "a", "post[1] == \"a\"");
    check(post[2].to_string() == "b", "post[2] == \"b\"");
    check(&post[1] == &post["1"], "post[1] and post[\"1\"] same object");
    check(post["1"].get_type() == obj_type::STRING, "post[\"1\"] remains string");
    return g_fail == 0 ? 0 : 1;
}

// V3: 空表单(NIL) 下 post[1] 应与 post["1"] 一样自动建对象
static int case_v3()
{
    obj_val post; // NIL
    bool threw = false;
    try
    {
        post[1] = std::string("x");
    }
    catch (const std::exception &e)
    {
        threw = true;
        std::printf("  INFO threw: %s\n", e.what());
    }
    check(!threw, "post[1] on NIL does not throw");
    check(post["1"].to_string() == "x", "post[\"1\"] == \"x\" after post[1]");
    return g_fail == 0 ? 0 : 1;
}

// V4: 数组的 [n] 与 ["n"] 都按位置
static int case_v4()
{
    obj_val arr;
    arr.set_array();
    arr.push(std::string("x"));
    arr.push(std::string("y"));

    check(arr[0].to_string() == "x", "arr[0] == x");
    check(arr["0"].to_string() == "x", "arr[\"0\"] == x");
    check(arr[1].to_string() == "y", "arr[1] == y");
    check(arr["1"].to_string() == "y", "arr[\"1\"] == y");

    bool threw = false;
    try
    {
        (void)arr[2];
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    check(threw, "arr[2] out of range throws");
    return g_fail == 0 ? 0 : 1;
}

// V5: 表单 aa[] aa[] aa[8] 的真实结构: OBJECT + 数字文本 key, 不补空位
static int case_v5()
{
    // post_form_to_postfield 的产出: {"aa":{"0":"A1","1":"A2","8":"A8"}}
    obj_val post;
    post["aa"]["0"] = std::string("A1");
    post["aa"]["1"] = std::string("A2");
    post["aa"]["8"] = std::string("A8");

    check(post["aa"].is_object(), "aa is OBJECT, parser never creates ARRAY");
    check(post["aa"].size() == 3, "sparse index: size is 3, not 9");
    check(post["aa"][0].to_string() == "A1", "aa[0] == aa[\"0\"] == A1");
    check(post["aa"][1].to_string() == "A2", "aa[1] == aa[\"1\"] == A2");
    check(post["aa"][8].to_string() == "A8", "aa[8] == aa[\"8\"] == A8");
    check(&post["aa"][8] == &post["aa"]["8"], "numeric and text subscript hit same member");

    // 读不存在的下标会隐式插入 NIL, 所以探测存在性要用 isset/at
    check(!post["aa"].isset("2"), "key 2 absent before read");
    (void)post["aa"][2];
    check(post["aa"].size() == 4 && post["aa"]["2"].is_null(), "operator[] read inserts NIL member");

    bool threw = false;
    try
    {
        (void)post["aa"].at("3");
    }
    catch (const std::exception &)
    {
        threw = true;
    }
    check(threw, "at(\"3\") throws instead of creating");
    return g_fail == 0 ? 0 : 1;
}

// V8: 持有引用后继续插入, 写回旧引用必须安全.
// 定稿方案 B 是 vector + 引用契约, 这是故意违约的用法, 预期 ASan UAF, 不算缺陷.
static int case_v8()
{
    obj_val post;
    post["cc"]["22"].set_array();
    post["cc"]["22"].push(std::string("A"));

    obj_val &cc = post["cc"];
    const void *addr_before = &cc;
    for (int i = 0; i < 64; i++)
        post["k" + std::to_string(i)] = i;

    cc["aabbcc"].set_array();
    cc["aabbcc"].push(std::string("B"));

    check(&post["cc"] == addr_before, "post[\"cc\"] address stable");
    check(post["cc"]["aabbcc"][0].to_string() == "B", "write through old reference visible");
    return g_fail == 0 ? 0 : 1;
}

// V10: 表单 key 注入, to_json 必须可原样 reparse
static int case_v10()
{
    const std::string key = "a\"b\nc\\d";
    obj_val post;
    post[key] = 1;
    post["中文"] = 2;

    const std::string j = post.to_json();
    obj_val back;
    bool ok = true;
    try
    {
        back.from_json(j);
    }
    catch (const std::exception &e)
    {
        ok = false;
        std::printf("  INFO reparse threw: %s\n", e.what());
    }
    check(ok, "to_json output reparses");
    check(back.isset(key), "injected key survives round-trip");
    check(back[key].to_int() == 1, "injected key value survives");
    check(back["中文"].to_int() == 2, "utf8 key survives");
    return g_fail == 0 ? 0 : 1;
}

// V11: 重复字段的对象层模拟: 标量 last-wins, 数组字段追加
static int case_v11()
{
    obj_val post;
    post["textfield2"] = std::string("A");
    post["textfield2"] = std::string("B");
    check(post["textfield2"].to_string() == "B", "scalar last-wins");

    obj_val arr;
    arr.set_array();
    arr.push(std::string("A"));
    arr.push(std::string("B"));
    check(arr.size() == 2, "array field appended");
    check(arr[0].to_string() == "A" && arr[1].to_string() == "B", "array order preserved");
    return g_fail == 0 ? 0 : 1;
}

// V12: 超长字符串与深嵌套不崩溃
static int case_v12()
{
    const std::size_t n = 1024 * 1024;
    obj_val big(std::string(n, 'x'));
    bool threw = false;
    std::string j;
    try
    {
        j = big.to_json();
    }
    catch (const std::exception &e)
    {
        threw = true;
        std::printf("  INFO to_json threw: %s\n", e.what());
    }
    check(!threw && j.size() >= n, "1MB string serializes");

    obj_val back;
    bool parsed = true;
    try
    {
        back.from_json(j);
    }
    catch (const std::exception &)
    {
        parsed = false;
    }
    check(!parsed || back.size() == n, "1MB string either accepted with full length or rejected cleanly");

    // 200 层嵌套: to_json 应受 JSON_MAX_DEPTH 限制而抛错, 而不是栈溢出
    obj_val root;
    obj_val *p = &root;
    for (int i = 0; i < 200; i++)
    {
        (*p)["a"].set_object();
        p = &(*p)["a"];
    }
    bool depth_threw = false;
    try
    {
        (void)root.to_json();
    }
    catch (const std::exception &)
    {
        depth_threw = true;
    }
    check(depth_threw, "200-level nesting rejected by depth limit");
    return g_fail == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::printf("usage: %s <v1|v2|v3|v4|v5|v8|v10|v11|v12>\n", argv[0]);
        return 2;
    }
    const std::string c = argv[1];
    if (c == "v1") return case_v1();
    if (c == "v2") return case_v2();
    if (c == "v3") return case_v3();
    if (c == "v4") return case_v4();
    if (c == "v5") return case_v5();
    if (c == "v8") return case_v8();
    if (c == "v10") return case_v10();
    if (c == "v11") return case_v11();
    if (c == "v12") return case_v12();
    std::printf("unknown case: %s\n", c.c_str());
    return 2;
}
