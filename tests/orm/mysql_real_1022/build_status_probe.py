import json
import os
import shlex
import subprocess

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
OUT = os.path.join(ROOT, "temp/mysql_real_1022")
SRC = os.path.join(HERE, "status_probe.cpp")
OBJ = os.path.join(OUT, "status_probe.o")

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
# 按后缀找编译条目：压测源在 tests/ 或 tests/orm/ 都能命中
entry = next(e for e in db if e["file"].endswith("test_mysql_stress.cpp"))

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
link[oi + 1] = os.path.join(OUT, "status_probe")

# 压测源搬到 tests/orm/ 后 CMake 把对象放进 CMakeFiles/<t>.dir/orm/，按后缀定位而非写死路径
hits = [k for k, p in enumerate(link) if p.endswith("test_mysql_stress.cpp.o")]
assert len(hits) == 1, "链接行上 test_mysql_stress 对象数=%d，预期 1" % len(hits)
link[hits[0]] = OBJ
print("LINK", flush=True)
subprocess.run(link, cwd=LINK_DIR, check=True)
print("OK", link[oi + 1], flush=True)
