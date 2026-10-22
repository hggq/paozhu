#!/usr/bin/env python3
# S-1 甲案：在驱动里加"写 + 自增主键同一次提交"的元组接口
# (文本通路 + 预编译通路)，按行号自下而上插入，每处先断言锚点。
# 一次性落码脚本，行号锚点对应落码当时的产品源码；已落码，仅作留档。
import os
import sys

# 探针目录在 tests/orm/<name>/，仓库根由脚本所在目录再往上三层
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
SRC = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn.cpp")

IMPL_TEXT = [
    "",
    "// 同一次提交里取影响行数和自增主键；拆成两次提交会读到别的调用方刚插入的 rowid",
    "std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_with_rowid_impl(const std::string &sql)",
    "{",
    "    unsigned int affected = this->exec_dml_impl(sql);",
    "    if (affected == static_cast<unsigned int>(-1))",
    "    {",
    "        return {affected, 0};",
    "    }",
    "    return {affected, this->last_insert_rowid_impl()};",
    "}",
]

SYNC_TEXT = [
    "",
    "std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_with_rowid(const std::string &sql)",
    "{",
    "    return this->submit_sync<std::tuple<unsigned int, long long>>(",
    "        [this, &sql]() { return this->exec_dml_with_rowid_impl(sql); });",
    "}",
]

ASYNC_TEXT = [
    "",
    "asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_with_rowid(const std::string &sql)",
    "{",
    "    co_return co_await this->run_on_worker<std::tuple<unsigned int, long long>>(",
    "        [this, sql]() { return this->exec_dml_with_rowid_impl(sql); });",
    "}",
]

IMPL_PREP = [
    "",
    "// 同 exec_dml_with_rowid_impl：预编译写法和它的 rowid 在一次提交里取完",
    "std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_prepared_with_rowid_impl(",
    "    const std::string &sql, const std::vector<http::obj_val> &params)",
    "{",
    "    unsigned int affected = this->exec_dml_prepared_impl(sql, params);",
    "    if (affected == static_cast<unsigned int>(-1))",
    "    {",
    "        return {affected, 0};",
    "    }",
    "    return {affected, this->last_insert_rowid_impl()};",
    "}",
]

EDITS = [
    (610, "}", IMPL_TEXT),
    (1319, "}", SYNC_TEXT),
    (1693, "}", ASYNC_TEXT),
    (2370, "}", IMPL_PREP),
]

lines = open(SRC, encoding="utf-8").read().split("\n")
for after, anchor, block in sorted(EDITS, key=lambda e: -e[0]):
    got = lines[after - 1]
    if got != anchor:
        sys.exit("锚点不符 %d: %r" % (after, got))
    lines[after:after] = block
    print("INSERT after %d (%d lines)" % (after, len(block)))

open(SRC, "w", encoding="utf-8").write("\n".join(lines))
print("OK %s" % SRC)
