#!/usr/bin/env python3
# S5 漏改臂：只把 async_fetch 那一处的写回闸门退回裸 if (iscache)（快照留着），
# 模拟"12 处里漏掉一处"。结构臂必须恰红 3 条：FILE_GATE 计数、该切片 GATE、全文件 BARE_GATE_LEFT。
import io, sys

GATE = "if (iscache && read_is_clean)"
SNAP = "bool read_is_clean = select_conn->error_msg.empty() && !select_conn->isclose;"
WHICH = int(sys.argv[2]) if len(sys.argv) > 2 else 1

path = sys.argv[1]
lines = io.open(path, encoding="utf-8").read().split("\n")
snaps = [i for i, l in enumerate(lines) if l.strip().endswith(SNAP)]
if len(snaps) != 4:
    sys.stderr.write("MUT_SNAP_HITS_UNEXPECTED %d in %s\n" % (len(snaps), path))
    sys.exit(2)
start = snaps[WHICH]
gates = [i for i in range(start, min(start + 20, len(lines))) if lines[i].strip() == GATE]
if len(gates) != 1:
    sys.stderr.write("MUT_GATE_HITS_UNEXPECTED %d after snapshot line %d in %s\n" % (len(gates), start, path))
    sys.exit(3)
i = gates[0]
indent = lines[i][:len(lines[i]) - len(lines[i].lstrip())]
lines[i] = indent + "if (iscache) // MUT-"
io.open(path, "w", encoding="utf-8").write("\n".join(lines))
sys.stderr.write("MUT_REVERT_ONE %s which=%d line=%d\n" % (path, WHICH, i + 1))
