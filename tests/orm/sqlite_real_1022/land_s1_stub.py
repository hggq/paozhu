#!/usr/bin/env python3
# S-1 甲案配套：存根(ENABLE_SQLITE=OFF)侧同名实现，
# 保证重新生成后的 lite 模型在 OFF 档仍然编译。
# 一次性落码脚本，行号锚点对应落码当时的产品源码；已落码，仅作留档。
import os
import sys

# 探针目录在 tests/orm/<name>/，仓库根由脚本所在目录再往上三层
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
SRC = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn_stub.cpp")

SYNC_TEXT = [
    "",
    "std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_with_rowid(const std::string & /*sql*/)",
    "{",
    "    error_msg = SQLITE_NOT_COMPILED_MSG;",
    "    return {static_cast<unsigned int>(-1), 0};",
    "}",
]

SYNC_PREP = [
    "",
    "std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_prepared_with_rowid(",
    "    const std::string & /*sql*/, const std::vector<http::obj_val> & /*params*/)",
    "{",
    "    error_msg = SQLITE_NOT_COMPILED_MSG;",
    "    return {static_cast<unsigned int>(-1), 0};",
    "}",
]

ASYNC_PREP = [
    "",
    "asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_prepared_with_rowid(",
    "    const std::string & /*sql*/, const std::vector<http::obj_val> & /*params*/)",
    "{",
    "    error_msg = SQLITE_NOT_COMPILED_MSG;",
    "    co_return std::tuple<unsigned int, long long>{static_cast<unsigned int>(-1), 0};",
    "}",
]

ASYNC_TEXT = [
    "",
    "asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_with_rowid(const std::string & /*sql*/)",
    "{",
    "    error_msg = SQLITE_NOT_COMPILED_MSG;",
    "    co_return std::tuple<unsigned int, long long>{static_cast<unsigned int>(-1), 0};",
    "}",
]

EDITS = [
    (132, "}", SYNC_TEXT),
    (171, "}", SYNC_PREP),
    (178, "}", ASYNC_PREP),
    (257, "}", ASYNC_TEXT),
]

lines = open(SRC, encoding="utf-8").read().split("\n")
for after, anchor, block in sorted(EDITS, key=lambda e: -e[0]):
    if lines[after - 1] != anchor:
        sys.exit("锚点不符 %d: %r" % (after, lines[after - 1]))
    lines[after:after] = block
    print("INSERT after %d (%d lines)" % (after, len(block)))

open(SRC, "w", encoding="utf-8").write("\n".join(lines))
print("OK %s" % SRC)
