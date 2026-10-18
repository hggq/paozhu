#!/usr/bin/env python3
# 从 build/compile_commands.json 取某个源文件原本的编译口径，供判据台单独重编一个 .o 用。
# 用法: python3 ccflags.py <仓库根> <仓库相对路径>   # 成功时把 flags 打成一行到 stdout
# 断言：该源在库里恰好出现一次；命令行以 "-o <它自己的 .o> -c <源>" 收尾（剥掉这段才得到纯 flags）；
# 含 -fsanitize=address（Debug 整档才是这个口径，拿不到就说明 build/ 不是判据想要的那棵树）；
# 不含 -fsyntax-only、剩余部分不含 " -o "（否则会把对象写回构建树）。
import json
import sys

if len(sys.argv) != 3:
    sys.exit("USAGE ccflags.py <repo_root> <source_rel_path>")
root, rel = sys.argv[1], sys.argv[2]

try:
    db = json.load(open(root + "/build/compile_commands.json"))
except Exception as e:
    sys.exit("CC_READ_FAIL %s: %s" % (root, e))

hits = [e for e in db if e["file"].endswith(rel)]
if len(hits) != 1:
    sys.exit("CC_ANCHOR_COUNT %s got=%d want=1" % (rel, len(hits)))
entry = hits[0]
cmd = entry["command"]
tail = " -o %s -c %s" % (entry["output"], entry["file"])
if not cmd.endswith(tail):
    sys.exit("CC_TAIL_MISMATCH %s" % rel)
flags = cmd[: -len(tail)]
if flags.startswith("/usr/bin/c++ "):
    flags = flags[len("/usr/bin/c++ "):]
else:
    sys.exit("CC_PREFIX_MISMATCH %s" % rel)
if "-fsanitize=address" not in flags:
    sys.exit("CC_NO_ASAN %s" % rel)
if "-fsyntax-only" in flags:
    sys.exit("CC_SYNTAX_ONLY %s" % rel)
if " -o " in flags:
    sys.exit("CC_STILL_HAS_O %s" % rel)
if "\n" in flags:
    sys.exit("CC_HAS_NEWLINE %s" % rel)
print(flags)
