import json
import os
import shlex
import subprocess
import sys

# ROOT 由本脚本位置往上三层推出（tests/orm/<探针目录>/<脚本>.py），换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, os.environ.get("PROBE_BUILD_DIR", "build"))
# 源码留在 tests/orm/，产物（.o、可执行、驱动副本、读数日志）一律落 temp/
OUT = os.path.join(ROOT, "temp/sqlite_real_1022")
SRC = os.path.join(HERE, "sqlite_real_probe.cpp")
OBJ = os.path.join(OUT, "sqlite_real_probe.o")
DRIVER = "vendor/httpserver/src/sqlite_conn.cpp"

# base   = 用 build/ 里现成的驱动对象(改前血统)；fix = 用候选补丁驱动的副本
# mut    = 用只摘掉一条既有守卫的突变驱动副本(牙齿臂)
# landed = 用落码后的工作树驱动(整轮改动的回归臂 + S-1 元组接口的新判据)
# 各产物的探针源码完全相同，只换链接进去的那枚 .o，且产物只落 temp/(不动 bin/)
VARIANT = (sys.argv[1] if len(sys.argv) > 1 else "base").lower()
assert VARIANT in ("base", "fix", "mut", "landed"), VARIANT
MUT_SRC = os.path.join(OUT, "sqlite_conn_mut.cpp")
WORK_DRIVER = os.path.join(ROOT, DRIVER)

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
# 压测源已搬到 tests/orm/，compile_commands 里的路径可能是旧 tests/ 或新 tests/orm/，
# 所以按文件名后缀取编译行，两种布局都能用（只借它的旗标）
entry = next(e for e in db if e["file"].endswith("test_sqlite_stress.cpp"))

MC_OBJ = None
FIXINC = os.path.join(OUT, "fixinc")
HEAD = os.path.join(FIXINC, "sqlite_conn.h")
# 头文件副本只要存在就前置 -I：探针、驱动、连接池三枚对象必须用同一份头
HEADER_PATCHED = False
if VARIANT in ("fix", "mut", "landed"):
    if VARIANT == "landed":
        FIX_SRC = WORK_DRIVER
    elif VARIANT == "mut":
        FIX_SRC = MUT_SRC
    else:
        FIX_SRC = os.path.join(OUT, "sqlite_conn_fix.cpp")
    if not os.path.exists(FIX_SRC):
        sys.exit("%s 版驱动副本不存在，先跑生成脚本(不许静默退回工作树驱动)" % VARIANT)
    if VARIANT == "mut" and "MUT-" not in open(FIX_SRC, encoding="utf-8").read():
        sys.exit("突变副本里没有 MUT- 锚点，说明摘除没落进产物")
    HEADER_PATCHED = os.path.exists(HEAD)
    if VARIANT == "mut" and HEADER_PATCHED:
        if "MUT-" not in open(HEAD, encoding="utf-8").read():
            sys.exit("头文件副本存在却不含 MUT- 锚点，S1B 臂的突变没落进头文件")
    if VARIANT != "mut" and HEADER_PATCHED:
        sys.exit("%s 版不该带头文件副本，先删 %s" % (VARIANT, HEAD))
    print("HEADER_PATCHED=%s src=%s" % (HEADER_PATCHED, FIX_SRC), flush=True)
    mc_entry = next(e for e in db if e["file"].endswith(DRIVER))
    mcmd = shlex.split(mc_entry["command"])
    mi = mcmd.index("-o")
    assert mcmd[mi + 1].endswith("sqlite_conn.cpp.o"), mcmd[mi + 1]
    del mcmd[mi:mi + 2]
    si = next(i for i, a in enumerate(mcmd) if a.endswith(DRIVER))
    mcmd[si] = FIX_SRC
    # 前置 -I：让补丁版头文件副本优先于 vendor/httpserver/include
    if HEADER_PATCHED:
        mcmd.insert(1, FIXINC)
        mcmd.insert(1, "-I")
    MC_OBJ = os.path.join(OUT, "sqlite_conn_%s.o" % VARIANT)
    print("COMPILE driver(%s 副本)" % VARIANT, flush=True)
    subprocess.run(mcmd + ["-o", MC_OBJ, "-c"], cwd=mc_entry["directory"], check=True)

cmd = shlex.split(entry["command"])
i = cmd.index("-o")
assert cmd[i + 1].endswith("test_sqlite_stress.cpp.o"), cmd[i + 1]
del cmd[i: i + 2]
cmd = [c for c in cmd if not c.endswith("test_sqlite_stress.cpp")]
if HEADER_PATCHED:
    cmd.insert(1, FIXINC)
    cmd.insert(1, "-I")
print("COMPILE", flush=True)
subprocess.run(cmd + ["-o", OBJ, SRC], cwd=entry["directory"], check=True)

LINK_DIR = os.path.join(BUILD, "tests")
link = shlex.split(open(os.path.join(LINK_DIR, "CMakeFiles/test_sqlite_stress.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/test_sqlite_stress"), link[oi + 1]
name = os.path.join(OUT, "sqlite_real_probe" + ("_" + VARIANT if VARIANT != "base" else ""))
link[oi + 1] = name

# 压测源搬到 tests/orm/ 后 CMake 把对象放进 CMakeFiles/<t>.dir/orm/，按后缀定位而非写死路径
hits = [k for k, p in enumerate(link) if p.endswith("test_sqlite_stress.cpp.o")]
assert len(hits) == 1, "链接行上 test_sqlite_stress 对象数=%d，预期 1" % len(hits)
link[hits[0]] = OBJ
if MC_OBJ is not None:
    hits = [k for k, p in enumerate(link) if p.endswith("sqlite_conn.cpp.o")]
    assert len(hits) == 1, "链接行上 sqlite_conn 对象数=%d，预期 1" % len(hits)
    link[hits[0]] = MC_OBJ
    print("SWAP sqlite_conn 对象 -> %s" % MC_OBJ, flush=True)

if HEADER_PATCHED:
    # 头文件副本多了一枚成员 -> 对象体积变化；池里 make_shared<sqlite_conn_base> 的
    # 那枚对象必须一起用补丁头重编，否则新旧布局混链
    pe = next(e for e in db if e["file"].endswith("vendor/httpserver/src/orm_conn_pool.cpp"))
    pcmd = shlex.split(pe["command"])
    pi = pcmd.index("-o")
    assert pcmd[pi + 1].endswith("orm_conn_pool.cpp.o"), pcmd[pi + 1]
    del pcmd[pi:pi + 2]
    pcmd.insert(1, FIXINC)
    pcmd.insert(1, "-I")
    POOL_OBJ = os.path.join(OUT, "orm_conn_pool_fix.o")
    print("COMPILE orm_conn_pool(补丁头) ", flush=True)
    subprocess.run(pcmd + ["-o", POOL_OBJ, "-c"], cwd=pe["directory"], check=True)
    phits = [k for k, p in enumerate(link) if p.endswith("orm_conn_pool.cpp.o")]
    assert len(phits) == 1, "链接行上 orm_conn_pool 对象数=%d，预期 1" % len(phits)
    link[phits[0]] = POOL_OBJ
    print("SWAP orm_conn_pool 对象 -> %s" % POOL_OBJ, flush=True)
print("LINK", flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)
print("OK", link[oi + 1], flush=True)
