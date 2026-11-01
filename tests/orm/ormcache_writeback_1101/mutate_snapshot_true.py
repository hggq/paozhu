#!/usr/bin/env python3
# S4 牙齿臂：把 4 处快照的判据内容换成恒真（// MUT- 标记），
# 结构臂必须恰红在"快照读驱动凭证"那一族断言上（预期 27 条，逐文件 9 条）。
import io, sys

SNAP = "bool read_is_clean = select_conn->error_msg.empty() && !select_conn->isclose;"
MUT = "bool read_is_clean = true; // MUT-"

path = sys.argv[1]
lines = io.open(path, encoding="utf-8").read().split("\n")
hits = [i for i, l in enumerate(lines) if l.strip().endswith(SNAP)]
if len(hits) != 4:
    sys.stderr.write("MUT_ANCHOR_HITS_UNEXPECTED %d in %s\n" % (len(hits), path))
    sys.exit(2)
for i in hits:
    indent = lines[i][:len(lines[i]) - len(lines[i].lstrip())]
    lines[i] = indent + MUT
io.open(path, "w", encoding="utf-8").write("\n".join(lines))
sys.stderr.write("MUT_SNAP_TRUE %s sites=%d\n" % (path, len(hits)))
