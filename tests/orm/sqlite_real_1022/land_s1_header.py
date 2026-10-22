#!/usr/bin/env python3
# S-1 甲案：给 sqlite_conn.h 的两枚 sqlite_conn_base(ENABLE_SQLITE 版 / 存根版)
# 插入元组接口声明，并给 ON 版插入预编译内联包装。
# 按行号自下而上插入，每处先断言锚点，防止插错位置。
# 一次性落码脚本，行号锚点对应落码当时的产品源码；已落码，仅作留档。
import os
import sys

# 探针目录在 tests/orm/<name>/，仓库根由脚本所在目录再往上三层
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
HDR = os.path.join(ROOT, "vendor/httpserver/include/sqlite_conn.h")

TEXT_PATH = [
    "",
    "    // 写语句和它的自增主键必须在同一次提交里取完：",
    "    // last_insert_rowid 是会话级状态，本方言所有调用方共用一条会话，",
    "    // 分成两次提交会读到别人插入的 id",
    "    std::tuple<unsigned int, long long> exec_dml_with_rowid(const std::string &sql);",
    "",
    "    asio::awaitable<std::tuple<unsigned int, long long>> async_exec_dml_with_rowid(const std::string &sql);",
]

PREPARED_DECL = [
    "",
    "    // 同 exec_dml_with_rowid：影响行数和自增主键在同一次提交里取完",
    "    std::tuple<unsigned int, long long> exec_dml_prepared_with_rowid(",
    "        const std::string &sql, const std::vector<http::obj_val> &params);",
    "",
    "    asio::awaitable<std::tuple<unsigned int, long long>> async_exec_dml_prepared_with_rowid(",
    "        const std::string &sql, const std::vector<http::obj_val> &params);",
]

PRIVATE_TEXT_IMPL = [
    "    std::tuple<unsigned int, long long> exec_dml_with_rowid_impl(const std::string &sql);",
]

PRIVATE_PREP_IMPL = [
    "",
    "    std::tuple<unsigned int, long long> exec_dml_prepared_with_rowid_impl(",
    "        const std::string &sql, const std::vector<http::obj_val> &params);",
]

INLINE_SYNC_PREP = [
    "",
    "inline std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_prepared_with_rowid(",
    "    const std::string &sql, const std::vector<http::obj_val> &params)",
    "{",
    "    return this->submit_sync<std::tuple<unsigned int, long long>>(",
    "        [this, &sql, &params]() { return this->exec_dml_prepared_with_rowid_impl(sql, params); });",
    "}",
]

INLINE_ASYNC_PREP = [
    "",
    "inline asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_prepared_with_rowid(",
    "    const std::string &sql, const std::vector<http::obj_val> &params)",
    "{",
    "    return this->run_on_worker<std::tuple<unsigned int, long long>>(",
    "        [this, sql, params]() { return this->exec_dml_prepared_with_rowid_impl(sql, params); });",
    "}",
]

# (在“第 N 行之后”插入, 该行必须含的锚点子串, 文本)
EDITS = [
    (195, "long long last_insert_rowid();", TEXT_PATH),
    (254, "const std::vector<http::obj_val> &params);", PREPARED_DECL),
    (340, "long long last_insert_rowid_impl();", PRIVATE_TEXT_IMPL),
    (365, "const std::vector<http::obj_val> &params);", PRIVATE_PREP_IMPL),
    (534, "}", INLINE_SYNC_PREP),
    (572, "}", INLINE_ASYNC_PREP),
    (604, "long long last_insert_rowid();", TEXT_PATH),
    (663, "const std::vector<http::obj_val> &params);", PREPARED_DECL),
]

lines = open(HDR, encoding="utf-8").read().split("\n")
for after, anchor, block in sorted(EDITS, key=lambda e: -e[0]):
    got = lines[after - 1]
    if anchor not in got:
        sys.exit("锚点不符 %d: %r" % (after, got))
    lines[after:after] = block
    print("INSERT after %d (%d lines)" % (after, len(block)))

open(HDR, "w", encoding="utf-8").write("\n".join(lines))
print("OK %s" % HDR)
