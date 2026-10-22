#!/usr/bin/env python3
# 用 build/ 的编译参数与链接行，把当前(或突变后的) mysql_conn.cpp 编成 temp/ 里的单 .o，
# 换掉链接行里 build/ 的那枚 mysql_conn.cpp.o，再链出 temp/ 下的探针二进制。
# 只产出 temp/ 里的可执行文件，绝不重编/重链 bin/paozhu。
# 用法: python3 tests/orm/mysql_real_1022/build_d1_probe.py [fix|mut|mut2|base]
#   mut  = 摘掉 ROWS 相位的"满块不算终结包"排除
#   mut2 = 摘掉 COLUMNS 相位的"满块交还 ROWS" Hand-off（= 改前那一条裸 0xFE 判据）
import json
import os
import re
import shlex
import subprocess
import sys

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "temp/mysql_real_1022")
VARIANT = (sys.argv[1] if len(sys.argv) > 1 else "fix").lower()
assert VARIANT in ("fix", "mut", "mut2", "base"), VARIANT

MC_SRC = os.path.join(ROOT, "vendor/httpserver/src/mysql_conn.cpp")
PROBE_SRC = os.path.join(HERE, "d1_fix_probe.cpp")
PROBE_OBJ = os.path.join(OUT, "d1_probe.o")
BIN = os.path.join(OUT, "d1_probe_" + VARIANT)

os.makedirs(OUT, exist_ok=True)


def md5(p):
    return subprocess.run(["md5", "-q", p], capture_output=True, text=True).stdout.strip()


def compile_one(entry_file, src_path, obj_path, label):
    db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
    entry = next(e for e in db if e["file"].endswith(entry_file))
    cmd = shlex.split(entry["command"])
    oi = cmd.index("-o")
    assert cmd[oi + 1].endswith(os.path.basename(entry["file"]) + ".o"), cmd[oi + 1]
    del cmd[oi:oi + 2]
    si = next(i for i, a in enumerate(cmd) if a.endswith(entry_file))
    cmd[si] = src_path
    print("COMPILE %s %s" % (label, os.path.basename(src_path)), flush=True)
    subprocess.run(cmd + ["-o", obj_path, "-c"], cwd=entry["directory"], check=True)


# ---------- 1. 驱动层对象 ----------
mc_obj = os.path.join(OUT, "mysql_conn_%s.o" % VARIANT)
if VARIANT == "base":
    link_mc_obj = "CMakeFiles/paozhu_test_orm.dir/__/vendor/httpserver/src/mysql_conn.cpp.o"
    print("USE build/ 的既有对象（改前血统）:", link_mc_obj, flush=True)
elif VARIANT == "mut":
    text = open(MC_SRC, encoding="utf-8").read()
    mut = text.replace("status == 0xFE && pkt_len < 0xFFFFFFu", "status == 0xFE")
    assert mut != text, "MUT-1 未命中：新终结判据不在磁盘上"
    assert mut.count("status == 0xFE && pkt_len") == 0, "MUT-1 摘不干净"
    mut_src = os.path.join(OUT, "mysql_conn_MUT1.cpp")
    open(mut_src, "w", encoding="utf-8").write(mut)
    compile_one("vendor/httpserver/src/mysql_conn.cpp", mut_src, mc_obj, "MUT-1 摘掉 ROWS 满块排除")
    link_mc_obj = mc_obj
elif VARIANT == "mut2":
    text = open(MC_SRC, encoding="utf-8").read()
    old = "if (pkt_len == 0xFFFFFFu && expected_cols > 0 && col_names.size() >= expected_cols)"
    assert text.count(old) == 2, "MUT-2 锚点命中 %d 处，预期 2（sync+async COLUMNS）" % text.count(old)
    mut = text.replace(old, "if (false && " + old[len("if ("):-1] + ")")
    assert mut.count("if (false && pkt_len == 0xFFFFFFu") == 2, "MUT-2 摘不干净"
    mut_src = os.path.join(OUT, "mysql_conn_MUT2.cpp")
    open(mut_src, "w", encoding="utf-8").write(mut)
    compile_one("vendor/httpserver/src/mysql_conn.cpp", mut_src, mc_obj, "MUT-2 摘掉 COLUMNS 交还点")
    link_mc_obj = mc_obj
else:
    compile_one("vendor/httpserver/src/mysql_conn.cpp", MC_SRC, mc_obj, "当前工作树")
    link_mc_obj = mc_obj

# ---------- 2. 探针对象 ----------
# 压测源搬到 tests/orm/ 后按后缀定位编译条目，两种布局都能找到
compile_one("test_mysql_stress.cpp", PROBE_SRC, PROBE_OBJ, "探针")

# ---------- 3. 换链 ----------
LINK_DIR = os.path.join(BUILD, "tests")
link = shlex.split(open(os.path.join(LINK_DIR, "CMakeFiles/test_mysql_stress.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/test_mysql_stress"), link[oi + 1]
link[oi + 1] = BIN

# 压测源搬到 tests/orm/ 后 CMake 把对象放进 CMakeFiles/<t>.dir/orm/，按后缀定位而非写死路径
hits_test = [i for i, p in enumerate(link) if p.endswith("test_mysql_stress.cpp.o")]
assert len(hits_test) == 1, "链接行上 test_mysql_stress 对象数=%d，预期 1" % len(hits_test)
link[hits_test[0]] = PROBE_OBJ

if VARIANT != "base":
    hits = [i for i, p in enumerate(link) if p.endswith("mysql_conn.cpp.o")]
    assert len(hits) == 1, "链接行上 mysql_conn 对象数=%d，预期 1" % len(hits)
    link[hits[0]] = mc_obj
    print("SWAP mysql_conn 对象 -> %s" % mc_obj, flush=True)
else:
    print("KEEP build/ 的 mysql_conn 对象（改前对照）", flush=True)

print("LINK ->", BIN, flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)

print("READ mc_src_md5=%s" % md5(MC_SRC), flush=True)
print("READ bin_md5=%s" % md5(BIN), flush=True)
print("READ bin_mtime=%s" % os.path.getmtime(BIN), flush=True)
print("OK", VARIANT, flush=True)
