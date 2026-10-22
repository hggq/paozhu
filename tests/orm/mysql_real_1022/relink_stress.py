#!/usr/bin/env python3
# 用当前工作树重编的 mysql_conn.o 换链两个既有压测基线（同步 21,000 次 / 协程 248,000 次），
# 各自的测试对象沿用 build/ 里的（压测源码未改），产物只落 temp/，绝不重编 bin/ 或 test 目标。
import json
import os
import shlex
import subprocess

# ROOT 由脚本位置往上三层推出（tests/orm/<探针目录>/<脚本>.py），换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "temp/mysql_real_1022")
MC_SRC = os.path.join(ROOT, "vendor/httpserver/src/mysql_conn.cpp")
MC_OBJ = os.path.join(OUT, "mysql_conn_stressfix.o")
TARGETS = ["test_mysql_stress", "test_mysql_stress_mt"]

db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
mc_entry = next(e for e in db if e["file"].endswith("vendor/httpserver/src/mysql_conn.cpp"))
mcmd = shlex.split(mc_entry["command"])
mi = mcmd.index("-o")
assert mcmd[mi + 1].endswith("mysql_conn.cpp.o"), mcmd[mi + 1]
del mcmd[mi:mi + 2]
si = next(i for i, a in enumerate(mcmd) if a.endswith("vendor/httpserver/src/mysql_conn.cpp"))
mcmd[si] = MC_SRC
print("COMPILE driver", flush=True)
subprocess.run(mcmd + ["-o", MC_OBJ, "-c"], cwd=mc_entry["directory"], check=True)

for t in TARGETS:
    link_dir = os.path.join(BUILD, "tests")
    link = shlex.split(open(os.path.join(link_dir, "CMakeFiles/%s.dir/link.txt" % t)).read())
    oi = link.index("-o")
    assert link[oi + 1].endswith("bin/" + t), link[oi + 1]
    link[oi + 1] = os.path.join(OUT, t + "_fix")
    hits = [k for k, p in enumerate(link) if p.endswith("mysql_conn.cpp.o")]
    assert len(hits) == 1, "%s 链接行上 mysql_conn 对象数=%d，预期 1" % (t, len(hits))
    link[hits[0]] = MC_OBJ
    print("LINK %s -> %s" % (t, link[oi + 1]), flush=True)
    subprocess.run(link, cwd=link_dir, check=True)
print("OK", flush=True)
