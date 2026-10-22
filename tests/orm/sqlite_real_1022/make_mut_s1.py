# 生成 S-1 元组接口的牙齿臂突变源。
#   S1A: 文本通路(exec_dml_with_rowid / async_exec_dml_with_rowid)拆回两次提交
#   S1B: 预编译通路(头文件里的 inline twin)拆回两次提交
#   clear: 撤掉突变产物，回到落码后的工作树
# 用法: python3 tests/orm/sqlite_real_1022/make_mut_s1.py S1A|S1B|clear
import os
import sys

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
OUT = os.path.join(ROOT, "temp/sqlite_real_1022")
DRIVER = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn.cpp")
HEADER = os.path.join(ROOT, "vendor/httpserver/include/sqlite_conn.h")
MUT_CPP = os.path.join(OUT, "sqlite_conn_mut.cpp")
FIXINC = os.path.join(OUT, "fixinc")
MUT_H = os.path.join(FIXINC, "sqlite_conn.h")

MODE = (sys.argv[1] if len(sys.argv) > 1 else "").upper()
assert MODE in ("S1A", "S1B", "CLEAR"), MODE

if MODE == "CLEAR":
    for p in (MUT_CPP, MUT_H):
        if os.path.exists(p):
            os.remove(p)
    print("CLEARED mut artifacts (工作树未被改动)")
    sys.exit(0)

SPLIT_TEXT = '''
// MUT-S1A 牙齿臂：写语句和取 rowid 拆成两次提交，会话级 rowid 会被别人的插入覆写
std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_with_rowid(const std::string &sql)
{
    unsigned int affected = this->submit_sync<unsigned int>([this, &sql]()
                                                            { return this->exec_dml_impl(sql); });
    long long rowid       = this->submit_sync<long long>([this]() { return this->last_insert_rowid_impl(); });
    return {affected, rowid};
}
'''

SPLIT_ASYNC_TEXT = '''
// MUT-S1A 牙齿臂：同上，协程版拆成两次 run_on_worker
asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_with_rowid(
    const std::string &sql)
{
    unsigned int affected =
        co_await this->run_on_worker<unsigned int>([this, sql]() { return this->exec_dml_impl(sql); });
    long long rowid = co_await this->run_on_worker<long long>([this]() { return this->last_insert_rowid_impl(); });
    co_return std::tuple<unsigned int, long long>{affected, rowid};
}
'''

SPLIT_PREP = '''
// MUT-S1B 牙齿臂：预编译写法与它的 rowid 拆成两次提交
inline std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_prepared_with_rowid(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    unsigned int affected = this->submit_sync<unsigned int>(
        [this, &sql, &params]() { return this->exec_dml_prepared_impl(sql, params); });
    long long rowid       = this->submit_sync<long long>([this]() { return this->last_insert_rowid_impl(); });
    return {affected, rowid};
}
'''

SPLIT_ASYNC_PREP = '''
// MUT-S1B 牙齿臂：同上，协程版拆成两次 run_on_worker
inline asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_prepared_with_rowid(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    unsigned int affected = co_await this->run_on_worker<unsigned int>(
        [this, sql, params]() { return this->exec_dml_prepared_impl(sql, params); });
    long long rowid = co_await this->run_on_worker<long long>([this]() { return this->last_insert_rowid_impl(); });
    co_return std::tuple<unsigned int, long long>{affected, rowid};
}
'''


def replace_once(text, old, new, tag):
    n = text.count(old)
    if n != 1:
        sys.exit("锚点 %s 命中 %d 处(预期 1)，突变未落码: %s" % (tag, n, old.strip().splitlines()[-1]))
    return text.replace(old, new)


drv = open(DRIVER, encoding="utf-8").read()
hdr = open(HEADER, encoding="utf-8").read()


# 落码态的四处实现，逐字面匹配（不用"到下一个函数为止"的区间猜测：
# 上一版这样切，把中间三百多行一起替换掉了，产物只剩 3/131 个符号）
OLD_SYNC_TEXT = '''std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_with_rowid(const std::string &sql)
{
    return this->submit_sync<std::tuple<unsigned int, long long>>(
        [this, &sql]() { return this->exec_dml_with_rowid_impl(sql); });
}'''

OLD_ASYNC_TEXT = '''asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_with_rowid(const std::string &sql)
{
    co_return co_await this->run_on_worker<std::tuple<unsigned int, long long>>(
        [this, sql]() { return this->exec_dml_with_rowid_impl(sql); });
}'''

OLD_SYNC_PREP = '''inline std::tuple<unsigned int, long long> sqlite_conn_base::exec_dml_prepared_with_rowid(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    return this->submit_sync<std::tuple<unsigned int, long long>>(
        [this, &sql, &params]() { return this->exec_dml_prepared_with_rowid_impl(sql, params); });
}'''

OLD_ASYNC_PREP = '''inline asio::awaitable<std::tuple<unsigned int, long long>> sqlite_conn_base::async_exec_dml_prepared_with_rowid(
    const std::string &sql, const std::vector<http::obj_val> &params)
{
    return this->run_on_worker<std::tuple<unsigned int, long long>>(
        [this, sql, params]() { return this->exec_dml_prepared_with_rowid_impl(sql, params); });
}'''

for old, src, where in ((OLD_SYNC_TEXT, drv, DRIVER), (OLD_ASYNC_TEXT, drv, DRIVER),
                        (OLD_SYNC_PREP, hdr, HEADER), (OLD_ASYNC_PREP, hdr, HEADER)):
    if old not in src:
        sys.exit("落码态的 %s 在 %s 里逐字面找不到，先核对工作树是否仍是落码形状" % (old.splitlines()[0][:70], where))

os.makedirs(FIXINC, exist_ok=True)

mut_drv = drv
mut_hdr = hdr
if MODE == "S1A":
    mut_drv = replace_once(mut_drv, OLD_SYNC_TEXT, SPLIT_TEXT, "S1A 文本 sync")
    mut_drv = replace_once(mut_drv, OLD_ASYNC_TEXT, SPLIT_ASYNC_TEXT, "S1A 文本 async")
    # 头文件不动：删掉副本，让探针/驱动/池三枚对象都用落码态的工作树头
    if os.path.exists(MUT_H):
        os.remove(MUT_H)
else:
    # 驱动用落码原样，只在副本上留一条标记注释(build_probe.py 要求突变源含 MUT- 锚点)
    mut_drv = replace_once(mut_drv, OLD_SYNC_TEXT, "\n// MUT-S1B 标记：本臂只改头文件里的预编译 twin\n" + OLD_SYNC_TEXT,
                           "S1B 驱动标记")
    mut_hdr = replace_once(mut_hdr, OLD_SYNC_PREP, SPLIT_PREP, "S1B 预编译 sync")
    mut_hdr = replace_once(mut_hdr, OLD_ASYNC_PREP, SPLIT_ASYNC_PREP, "S1B 预编译 async")

if "MUT-" not in mut_drv:
    sys.exit("突变驱动副本没有 MUT- 锚点")
if MODE == "S1B" and "MUT-" not in mut_hdr:
    sys.exit("突变头文件副本没有 MUT- 锚点")

if abs(mut_drv.count("\n") - drv.count("\n")) > 12 or abs(mut_hdr.count("\n") - hdr.count("\n")) > 12:
    sys.exit("突变副本与工作树行数差超过 12 行，替换吃掉了不该动的代码：%d/%d cpp，%d/%d h" %
             (mut_drv.count("\n"), drv.count("\n"), mut_hdr.count("\n"), hdr.count("\n")))

open(MUT_CPP, "w", encoding="utf-8").write(mut_drv)
if MODE == "S1B":
    open(MUT_H, "w", encoding="utf-8").write(mut_hdr)
print("MODE=%s" % MODE)
print("  driver=%s 头副本=%s" % (MUT_CPP, MUT_H if MODE == "S1B" else "(不写，用工作树头)"))
print("  driver 含 MUT 标记: %s / 头含 MUT 标记: %s" % ("MUT-" in mut_drv, "MUT-" in mut_hdr if MODE == "S1B" else False))
print("  S1A 生效: %s / S1B 生效: %s" % ("MUT-S1A" in mut_drv, "MUT-S1B" in mut_hdr))
