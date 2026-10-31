# prepfix 探针：编当前工作树的 sqlite_conn.cpp 为单枚 .o，换进 test_sqlite_stress 的链接行
# 用法: python3 build_prepfix.py [landed|mut|mut2|mut3]
#   landed=工作树原样 mut=把命名占位符守卫改回"名字非空" mut2=尾串注释扫描改回直接判第二条
#   mut3=错误文案不再复述实际占位符名
import json
import os
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, os.environ.get("PROBE_BUILD_DIR", "build"))
OUT = os.path.join(ROOT, "temp/sqlite_probe_1031")
SRC = os.path.join(HERE, "prepfix_probe.cpp")
OBJ = os.path.join(OUT, "prepfix_probe.o")
VARIANT = (sys.argv[1] if len(sys.argv) > 1 else "landed").lower()
assert VARIANT in ("landed", "mut", "mut2", "mut3"), VARIANT

# 牙齿臂：每枚只撤一处修复，红集合必须恰好落在该处功能面上
MUTATIONS = {
    "mut": [("if (pname != nullptr && pname[0] != '?')", "if (pname != nullptr)")],
    "mut2": [("if (ch == '-' && (unsigned char)tail[1] == '-')", "if (false)"),
             ("if (ch == '/' && (unsigned char)tail[1] == '*')", "if (false)")],
    # 保留 pname 的读取，只把复述的内容换成空串，红应恰落在"文案复述原文"这一面
    "mut3": [("+ (pname != nullptr ? pname : \"\")", "+ (pname != nullptr ? \"\" : \"\")")],
}

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
entry = next(e for e in db if e["file"].endswith("test_sqlite_stress.cpp"))

# ---------- 1. 驱动对象 ----------
drv_entry = next(e for e in db if e["file"].endswith("vendor/httpserver/src/sqlite_conn.cpp"))
dcmd = shlex.split(drv_entry["command"])
di = dcmd.index("-o")
assert dcmd[di + 1].endswith("sqlite_conn.cpp.o"), dcmd[di + 1]
del dcmd[di:di + 2]
si = next(i for i, a in enumerate(dcmd) if a.endswith("vendor/httpserver/src/sqlite_conn.cpp"))
DRV_SRC = os.path.join(ROOT, "vendor/httpserver/src/sqlite_conn.cpp")
DRVOBJ = os.path.join(OUT, "sqlite_conn_%s.o" % VARIANT)
if VARIANT != "landed":
    text = open(DRV_SRC, encoding="utf-8").read()
    mut_src = os.path.join(OUT, "sqlite_conn_%s.cpp" % VARIANT)
    for old, new in MUTATIONS[VARIANT]:
        n = text.count(old)
        assert n == 1, "突变锚点 %r 命中数=%d，预期 1" % (old, n)
        text = text.replace(old, new)
    open(mut_src, "w", encoding="utf-8").write(text)
    DRV_SRC = mut_src
dcmd[si] = DRV_SRC
print("COMPILE driver %s" % DRV_SRC, flush=True)
subprocess.run(dcmd + ["-o", DRVOBJ, "-c"], cwd=drv_entry["directory"], check=True)

# ---------- 2. 探针 TU ----------
cmd = shlex.split(entry["command"])
i = cmd.index("-o")
assert cmd[i + 1].endswith("test_sqlite_stress.cpp.o"), cmd[i + 1]
del cmd[i:i + 2]
cmd = [c for c in cmd if not c.endswith("test_sqlite_stress.cpp")]
print("COMPILE probe", flush=True)
subprocess.run(cmd + ["-o", OBJ, SRC], cwd=entry["directory"], check=True)

# ---------- 3. 链接 ----------
LINK_DIR = os.path.join(BUILD, "tests")
link = shlex.split(open(os.path.join(LINK_DIR, "CMakeFiles/test_sqlite_stress.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/test_sqlite_stress"), link[oi + 1]
link[oi + 1] = os.path.join(OUT, "prepfix_" + VARIANT)
hits = [k for k, p in enumerate(link) if p.endswith("test_sqlite_stress.cpp.o")]
assert len(hits) == 1, "链接行上 test_sqlite_stress 对象数=%d，预期 1" % len(hits)
link[hits[0]] = OBJ
hits = [k for k, p in enumerate(link) if p.endswith("sqlite_conn.cpp.o")]
assert len(hits) == 1, "链接行上 sqlite_conn 对象数=%d，预期 1" % len(hits)
link[hits[0]] = DRVOBJ
print("SWAP sqlite_conn 对象 -> %s" % DRVOBJ, flush=True)
print("LINK", flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)
print("OK", link[oi + 1], flush=True)
