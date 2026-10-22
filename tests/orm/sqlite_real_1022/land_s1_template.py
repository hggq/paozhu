#!/usr/bin/env python3
# S-1 甲案：sqliteorm.hpp 模板 12 处"写 + 取 rowid"调用点改为元组接口。
# 逐行断言原内容后整行替换，自下而上，防止行号漂移。
# 一次性落码脚本，行号锚点对应落码当时的模板；已落码，仅作留档。
import os
import sys

# 探针目录在 tests/orm/<name>/，仓库根由脚本所在目录再往上三层
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
SRC = os.path.join(ROOT, "vendor/httpserver/include/sqliteorm.hpp")

CMT16 = "                // exec_dml 无法接收 RETURNING 结果集：直接执行原 SQL，影响行数与自增主键在同一次提交里取回"
CMT20 = "                    // exec_dml 无法接收 RETURNING 结果集：直接执行原 SQL，影响行数与自增主键在同一次提交里取回"

NEW = {
    6943: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    6944: ("unsigned int affected = edit_conn->exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = edit_conn->exec_dml_with_rowid(sqlstring);"),
    6963: ("insert_last_id = edit_conn->last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7015: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7016: ("unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = co_await edit_conn->async_exec_dml_with_rowid(sqlstring);"),
    7035: ("insert_last_id = co_await edit_conn->async_last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7087: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7088: ("unsigned int affected = edit_conn->exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = edit_conn->exec_dml_with_rowid(sqlstring);"),
    7107: ("insert_last_id = edit_conn->last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7159: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7160: ("unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = co_await edit_conn->async_exec_dml_with_rowid(sqlstring);"),
    7179: ("insert_last_id = co_await edit_conn->async_last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7231: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7232: ("unsigned int affected = edit_conn->exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = edit_conn->exec_dml_with_rowid(sqlstring);"),
    7251: ("insert_last_id = edit_conn->last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7303: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7304: ("unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = co_await edit_conn->async_exec_dml_with_rowid(sqlstring);"),
    7323: ("insert_last_id = co_await edit_conn->async_last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7456: (("// exec_dml 无法接收 RETURNING 结果集", CMT16)),
    7457: ("unsigned int affected = edit_conn->exec_dml(sqlstring);",
           "                auto [affected, insert_last_id_raw] = edit_conn->exec_dml_with_rowid(sqlstring);"),
    7476: ("insert_last_id = edit_conn->last_insert_rowid();",
           "                    insert_last_id = insert_last_id_raw;"),
    7616: (("// exec_dml 无法接收 RETURNING 结果集", CMT20)),
    7617: ("unsigned int affected = co_await edit_conn->async_exec_dml(sqlstring);",
           "                    auto [affected, insert_last_id_raw] = co_await edit_conn->async_exec_dml_with_rowid(sqlstring);"),
    7636: ("insert_last_id = co_await edit_conn->async_last_insert_rowid();",
           "                        insert_last_id = insert_last_id_raw;"),

    11613: ("unsigned int affected = conn->exec_dml_prepared(sql, params);",
            "            auto [affected, prep_rowid] = conn->exec_dml_prepared_with_rowid(sql, params);"),
    11621: ("last_id = conn->last_insert_rowid();",
            "                last_id = prep_rowid;"),
    11692: ("unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);",
            "                auto [affected, prep_rowid] = co_await edit_conn->async_exec_dml_prepared_with_rowid(sql, params);"),
    11709: ("long long last_id = co_await edit_conn->async_last_insert_rowid();",
            "                long long last_id = prep_rowid;"),
    11791: ("unsigned int affected = conn->exec_dml_prepared(sql, params);",
            "            auto [affected, prep_rowid] = conn->exec_dml_prepared_with_rowid(sql, params);"),
    11799: ("first_id = conn->last_insert_rowid();",
            "                first_id = prep_rowid;"),
    11891: ("unsigned int affected = co_await edit_conn->async_exec_dml_prepared(sql, params);",
            "                auto [affected, prep_rowid] = co_await edit_conn->async_exec_dml_prepared_with_rowid(sql, params);"),
    11908: ("long long first_id = co_await edit_conn->async_last_insert_rowid();",
            "                long long first_id = prep_rowid;"),
}

lines = open(SRC, encoding="utf-8").read().split("\n")
n = 0
for no in sorted(NEW, reverse=True):
    anchor, repl = NEW[no]
    old = lines[no - 1]
    if anchor not in old:
        sys.exit("锚点不符 %d: %r" % (no, old))
    lines[no - 1] = repl
    n += 1
    print("REPLACE %d" % no)

open(SRC, "w", encoding="utf-8").write("\n".join(lines))
print("OK %d 处 %s" % (n, SRC))
