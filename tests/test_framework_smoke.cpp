/*
 * test_framework_smoke.cpp — 新框架冒烟测试
 * 验证 test_framework.hpp 的 TEST_CASE / REQUIRE / REQUIRE_EQ / REQUIRE_NE / TEST_MAIN 全部正常工作。
 */

#include "test_framework.hpp"

#include <string>

TEST_CASE(pass_case)
{
    REQUIRE(1 + 1 == 2);
    REQUIRE_EQ(2 + 3, 5);
    REQUIRE_NE(1, 2);
    REQUIRE_EQ(std::string("hello").size(), 5u);
}

TEST_CASE(another_pass_case)
{
    int x = 10;
    REQUIRE_EQ(x, 10);
    REQUIRE(x > 0);
}

TEST_MAIN()
