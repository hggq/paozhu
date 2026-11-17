import json
import os
import shlex
import subprocess

# 源码在 tests/orm/，生成物落 temp/；ROOT 由脚本位置往上三层推出，换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, os.environ.get("PROBE_BUILD_DIR", "build"))
OUT = os.path.join(ROOT, "temp/pg_prepared_1117")
SRC = os.path.join(HERE, "prepared_probe.cpp")
OBJ = os.path.join(OUT, "prepared_probe.o")
BIN = os.path.join(OUT, "prepared_probe")

os.makedirs(OUT, exist_ok=True)
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))
# 用框架 server.cpp 的编译条目拿宏集（tests 目标的宏集与框架不一致，会读错位成员）
entry = next(e for e in db if e["file"].endswith("vendor/httpserver/src/server.cpp"))
cmd = shlex.split(entry["command"])
i = cmd.index("-o")
del cmd[i : i + 2]
cmd = [c for c in cmd if c != "-c" and not c.endswith("vendor/httpserver/src/server.cpp")]
print("COMPILE", flush=True)
subprocess.run(cmd + ["-c", SRC, "-o", OBJ], cwd=entry["directory"], check=True)

# 全链接面：db_conn 模板 + obj_val + 连接池闭合到整个 app，换入口对象重链
link = shlex.split(open(os.path.join(BUILD, "CMakeFiles/paozhu.dir/link.txt")).read())
oi = link.index("-o")
assert link[oi + 1].endswith("bin/paozhu"), link[oi + 1]
link[oi + 1] = BIN
MAIN = "CMakeFiles/paozhu.dir/startup/main_dev.cpp.o"
assert MAIN in link, "main_dev object missing from link line"
link = [OBJ if p == MAIN else p for p in link]
print("LINK", flush=True)
subprocess.run(link, cwd=BUILD, check=True)
print("BUILT", BIN, flush=True)