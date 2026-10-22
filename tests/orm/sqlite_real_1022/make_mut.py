# 牙齿臂：从"未改产品码"血统的驱动出发，只摘掉一条既有守卫，看判据是否恰好在预期条目变红。
# 用法: python3 make_mut.py MUT-R|MUT-B|MUT-T   (产物 temp/sqlite_real_1022/sqlite_conn_mut.cpp)
# 产品源码不动；每条臂一次构建一次运行，红集合必须命中预登记判据，否则该判据无判别力。
import os
import sys

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
GEN = os.path.join(ROOT, "temp/sqlite_real_1022")
SRC = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn.cpp")
OUT = os.path.join(GEN, "sqlite_conn_mut.cpp")

ARM = (sys.argv[1] if len(sys.argv) > 1 else "").upper()
assert ARM in ("MUT-R", "MUT-B", "MUT-T"), "用法: make_mut.py MUT-R|MUT-B|MUT-T"

src = open(SRC, encoding="utf-8").read()
base_md5 = __import__("hashlib").md5(src.encode()).hexdigest()


def sub(anchor, repl):
    global src
    n = src.count(anchor)
    assert n == 1, "锚点命中 %d 次，预期 1:\n%s" % (n, anchor[:80])
    src = src.replace(anchor, repl)


if ARM == "MUT-R":
    # 摘掉 fetch_directly_impl 出口处的 reset(读快照会钉住 WAL)：预登记红=E3a
    sub("""    // reset 不会丢失错误信息（errcode 仍可从 db_ 读取）。
    sqlite3_reset(stmt);""",
        """    // MUT-R 摘掉本函数出口 reset（预登记：E3a 必须变红）""")
elif ARM == "MUT-B":
    # 摘掉绑定前的参数个数门禁：占位符多于参数时 SQLite 自身不报错(漏绑成 NULL)
    # 预登记红=B4d(参数不足必须显式拒)
    sub("""    int max_param = sqlite3_bind_parameter_count(stmt);
    if (params.size() != static_cast<size_t>(max_param))
    {
        return SQLITE_RANGE;
    }""",
        """    int max_param = sqlite3_bind_parameter_count(stmt);// MUT-B 摘掉参数个数门禁
    (void)max_param;""")
else:
    # 摘掉 exec_sql_impl 成功路径的事务标记同步：裸 BEGIN/COMMIT 不再被追踪
    # 预登记红=C4a/C4c/C4e/C4f/C4g/C4h 这一组
    sub("""    // 以 sqlite3_get_autocommit 为准同步事务标记与事务锁：
    // 本次调用打开/关闭了事务窗口（裸 BEGIN/COMMIT/ROLLBACK）时，
    // 引擎状态是唯一权威，事务锁必须与之一致，防止跨调用方事务串扰
    this->sync_transaction_state_after_exec();""",
        """    // MUT-T 摘掉事务标记同步（预登记：C4 裸事务追踪一组变红）""")

open(OUT, "w", encoding="utf-8").write(src)
assert "MUT-" in open(OUT, encoding="utf-8").read(), "突变锚点未落进产物"
sys.stdout.write("base_md5=%s arm=%s out=%s\n" % (base_md5, ARM, OUT))
