import json
import os
import shlex
import subprocess
import sys

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "temp/mysql_real_1022")
SRC = os.path.join(HERE, "mysql_real_probe.cpp")
OBJ = os.path.join(OUT, "mysql_real_probe.o")
# fix = 用当前工作树重编的驱动对象换掉链接行里 build/ 的那枚（改前血统），产物仍只落 temp/
VARIANT = (sys.argv[1] if len(sys.argv) > 1 else "base").lower()
assert VARIANT in ("base", "fix"), VARIANT

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
# 按后缀找编译条目：压测源在 tests/ 或 tests/orm/ 都能命中
entry = next(e for e in db if e["file"].endswith("test_mysql_stress.cpp"))

MC_OBJ = None
if VARIANT == "fix":
    mc_entry = next(e for e in db if e["file"].endswith("vendor/httpserver/src/mysql_conn.cpp"))
    mcmd = shlex.split(mc_entry["command"])
    mi = mcmd.index("-o")
    assert mcmd[mi + 1].endswith("mysql_conn.cpp.o"), mcmd[mi + 1]
    del mcmd[mi:mi + 2]
    si = next(i for i, a in enumerate(mcmd) if a.endswith("vendor/httpserver/src/mysql_conn.cpp"))
    mcmd[si] = os.path.join(ROOT, "vendor/httpserver/src/mysql_conn.cpp")
    MC_OBJ = os.path.join(OUT, "mysql_conn_%s.o" % VARIANT)
    print("COMPILE driver", flush=True)
    subprocess.run(mcmd + ["-o", MC_OBJ, "-c"], cwd=mc_entry["directory"], check=True)

cmd = shlex.split(entry["command"])
i = cmd.index("-o")
assert cmd[i + 1].endswith("test_mysql_stress.cpp.o"), cmd[i + 1]
del cmd[i : i + 2]
cmd = [c for c in cmd if not c.endswith("test_mysql_stress.cpp")]
print("COMPILE", flush=True)
subprocess.run(cmd + ["-o", OBJ, SRC], cwd=entry["directory"], check=True)

LINK_DIR = os.path.join(BUILD, "tests")
link = shlex.split(open(os.path.join(LINK_DIR, "CMakeFiles/test_mysql_stress.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/test_mysql_stress"), link[oi + 1]
link[oi + 1] = os.path.join(OUT, "mysql_real_probe" + ("_fix" if VARIANT == "fix" else ""))

# 压测源搬到 tests/orm/ 后 CMake 把对象放进 CMakeFiles/<t>.dir/orm/，按后缀定位而非写死路径
hits = [k for k, p in enumerate(link) if p.endswith("test_mysql_stress.cpp.o")]
assert len(hits) == 1, "链接行上 test_mysql_stress 对象数=%d，预期 1" % len(hits)
link[hits[0]] = OBJ
if MC_OBJ is not None:
    hits = [k for k, p in enumerate(link) if p.endswith("mysql_conn.cpp.o")]
    assert len(hits) == 1, "链接行上 mysql_conn 对象数=%d，预期 1" % len(hits)
    link[hits[0]] = MC_OBJ
    print("SWAP mysql_conn 对象 -> %s" % MC_OBJ, flush=True)
print("LINK", flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)
print("OK", link[oi + 1], flush=True)
