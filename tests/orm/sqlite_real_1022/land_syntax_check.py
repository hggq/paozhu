#!/usr/bin/env python3
# 落码后语法检查: ON 档驱动 / OFF 档存根 / 模板本体(探针 TU 直接 include sqliteorm.hpp)
# 构建树可能是 ON 档,所以每组各自把 -DENABLE_SQLITE 掰到需要的状态。
import json
import os
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
BUILD = os.path.join(ROOT, "build")
db = json.load(open(os.path.join(BUILD, "compile_commands.json")))


def entry_for(suffix, want_out="CMakeFiles/paozhu.dir"):
    hits = [e for e in db if e["file"].endswith(suffix) and want_out in e["command"]]
    if len(hits) != 1:
        sys.exit("%s 命中 %d 条，预期 1" % (suffix, len(hits)))
    return hits[0]


def syntax_check(name, suffix, want_sqlite):
    e = entry_for(suffix)
    cmd = shlex.split(e["command"])
    i = cmd.index("-o")
    del cmd[i:i + 2]
    j = cmd.index("-c")
    del cmd[j:j + 1]
    had = "-DENABLE_SQLITE" in cmd
    cmd = [p for p in cmd if p != "-DENABLE_SQLITE"]
    if want_sqlite:
        cmd.append("-DENABLE_SQLITE")
    cmd.insert(1, "-fsyntax-only")
    print("[%s] 构建树带 ENABLE_SQLITE=%s，本臂要求=%s" % (name, had, want_sqlite), flush=True)
    r = subprocess.run(cmd, cwd=e["directory"], capture_output=True, text=True)
    print("[%s] exit=%d" % (name, r.returncode), flush=True)
    out = (r.stdout + r.stderr).strip()
    if out:
        print(out, flush=True)
    return r.returncode


def template_probe():
    """模板本体: 借 ON 档驱动的编译开关, 编 temp 里的探针 TU(直接 include 改过的模板)"""
    e = entry_for("vendor/httpserver/src/sqlite_conn.cpp")
    cmd = shlex.split(e["command"])
    i = cmd.index("-o")
    del cmd[i:i + 2]
    j = cmd.index("-c")
    del cmd[j:j + 1]
    cmd = cmd[:1] + ["-fsyntax-only"] + cmd[1:]
    cmd = [p for p in cmd if p != e["file"]]
    src = os.path.join(HERE, "probe_s1_template.cpp")
    prepped = os.path.join(ROOT, "temp/ormprobe1018/prepped")
    if not os.path.isdir(prepped):
        sys.exit("缺少模板头副本目录 %s，缺它会把生成码的头找不到当成语法错误" % prepped)
    cmd += ["-I" + prepped, "-I" + os.path.join(ROOT, "orm/lite/include"), src]
    r = subprocess.run(cmd, cwd=e["directory"], capture_output=True, text=True)
    print("[模板 12 处调用点探针] exit=%d" % r.returncode, flush=True)
    out = (r.stdout + r.stderr).strip()
    if out:
        print(out, flush=True)
    return r.returncode


rc = 0
rc |= syntax_check("ON  sqlite_conn.cpp", "vendor/httpserver/src/sqlite_conn.cpp", True)
rc |= syntax_check("OFF sqlite_conn_stub.cpp", "vendor/httpserver/src/sqlite_conn_stub.cpp", False)
rc |= template_probe()
print("SYNTAX rc=%d" % rc)
sys.exit(rc)
