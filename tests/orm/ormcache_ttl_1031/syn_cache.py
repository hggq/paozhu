#!/usr/bin/env python3
# 缓存头改动的语法面：跑真正吃得到 orm_cache.hpp 的 TU（不是那五个驱动 TU）。
# 判据形状照抄 temp/ormA_1031/synA.py：必须正好剥掉一枚 -o 与一枚 -c，否则拒绝读数。
# 血统证明：演示页里的 use_cache/check_cache 只可能来自生成模型 -> orm_cache.hpp，
# 所以它编得过 = 这枚头真被吃到。
import json, subprocess, sys, shlex

ROOT = "/Users/hzq/paozhu"
TARGETS = [
    "controller/src/testormcache.cpp",
    "vendor/httpserver/src/orm_query.cpp",
]

with open(ROOT + "/build/compile_commands.json") as f:
    db = json.load(f)

by_file = {}
for e in db:
    fp = e["file"]
    if fp.startswith(ROOT + "/"):
        rel = fp[len(ROOT) + 1:]
        if rel in TARGETS:
            by_file.setdefault(rel, e)

missing = [t for t in TARGETS if t not in by_file]
if missing:
    print("MISSING_IN_COMPILE_DB " + ",".join(missing))
    sys.exit(2)

total_fail = 0
for t in TARGETS:
    e = by_file[t]
    cmd = e.get("command")
    if cmd is None:
        print("NO_COMMAND_FIELD " + t)
        sys.exit(2)
    args = shlex.split(cmd)
    stripped = []
    removed_o = 0
    removed_c = 0
    i = 0
    while i < len(args):
        a = args[i]
        if a == "-o" and i + 1 < len(args):
            removed_o += 1
            i += 2
            continue
        if a == "-c":
            removed_c += 1
            i += 1
            continue
        stripped.append(a)
        i += 1
    if removed_o != 1 or removed_c != 1:
        print("GATE_STRIPPED_FAIL " + t + " -o=" + str(removed_o) + " -c=" + str(removed_c))
        sys.exit(2)
    stripped.append("-fsyntax-only")
    for w in ("-Wall", "-Wextra", "-Wpedantic", "-Wshadow"):
        stripped.append(w)
    r = subprocess.run(stripped, cwd=e["directory"], capture_output=True, text=True)
    diags = [ln for ln in (r.stderr + r.stdout).splitlines() if "warning:" in ln or "error:" in ln]
    print("SYNTAX " + t + " EXIT=" + str(r.returncode) + " DIAGLINES=" + str(len(diags)))
    for d in diags[:20]:
        print("  " + d)
    if r.returncode != 0 or diags:
        total_fail += 1

print("SYNTAX_TOTAL_TU=" + str(len(TARGETS)) + " BAD=" + str(total_fail))
sys.exit(0 if total_fail == 0 else 1)
