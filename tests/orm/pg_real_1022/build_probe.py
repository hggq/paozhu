import json
import os
import shlex
import subprocess

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "temp/pg_real_1022")
SRC = os.path.join(HERE, "pg_real_probe.cpp")
OBJ = os.path.join(OUT, "pg_real_probe.o")

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
# 按后缀找编译条目：压测源在 tests/ 或 tests/orm/ 都能命中
entry = next(e for e in db if e["file"].endswith("test_pg_stress.cpp"))

cmd = shlex.split(entry["command"])
i = cmd.index("-o")
assert cmd[i + 1].endswith("test_pg_stress.cpp.o"), cmd[i + 1]
del cmd[i : i + 2]
cmd = [c for c in cmd if not c.endswith("test_pg_stress.cpp")]
print("COMPILE", flush=True)
subprocess.run(cmd + ["-o", OBJ, SRC], cwd=entry["directory"], check=True)

LINK_DIR = os.path.join(BUILD, "tests")
link = shlex.split(open(os.path.join(LINK_DIR, "CMakeFiles/test_pg_stress.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/test_pg_stress"), link[oi + 1]
override = os.environ.get("PG_CONN_OVERRIDE")
name = "pg_real_probe_fix" if override else "pg_real_probe"
if override and os.path.realpath(override) == os.path.realpath(
        os.path.join(ROOT, "vendor/httpserver/src/pg_conn.cpp")):
    name = "pg_real_probe_landed"
link[oi + 1] = os.path.join(OUT, name)

if override:
    pe = next(e for e in db if e["file"].endswith("vendor/httpserver/src/pg_conn.cpp"))
    pcmd = shlex.split(pe["command"])
    pi = pcmd.index("-o")
    assert pcmd[pi + 1].endswith("vendor/httpserver/src/pg_conn.cpp.o"), pcmd[pi + 1]
    del pcmd[pi : pi + 2]
    pcmd = [c for c in pcmd if not c.endswith("vendor/httpserver/src/pg_conn.cpp")]
    POBJ = os.path.join(OUT, "pg_conn_override.o")
    print("COMPILE-OVERRIDE", override, flush=True)
    subprocess.run(pcmd + ["-o", POBJ, override], cwd=pe["directory"], check=True)
    POLD = "CMakeFiles/paozhu_test_orm.dir/__/vendor/httpserver/src/pg_conn.cpp.o"
    assert POLD in link, "pg_conn object missing from link line"
    link = [POBJ if p == POLD else p for p in link]

# 压测源搬到 tests/orm/ 后 CMake 把对象放进 CMakeFiles/<t>.dir/orm/，按后缀定位而非写死路径
hits = [k for k, p in enumerate(link) if p.endswith("test_pg_stress.cpp.o")]
assert len(hits) == 1, "链接行上 test_pg_stress 对象数=%d，预期 1" % len(hits)
link[hits[0]] = OBJ
print("LINK", flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)
print("OK", link[oi + 1], flush=True)
