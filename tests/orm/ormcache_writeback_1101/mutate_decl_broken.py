#!/usr/bin/env python3
# S3 可达性臂：把 4 处快照声明改名（闸门仍引用旧名），
# 该方言探针必须编译失败，且错误恰落在 4 个不同行号=4 处写回点，
# 证明"类模板成员只有用到才实例化"这一轮 4 个点真被探针吃到。
import io, sys

DECL = "bool read_is_clean = "
BROKEN = "bool read_is_clean_broken = "

path = sys.argv[1]
lines = io.open(path, encoding="utf-8").read().split("\n")
hits = [i for i, l in enumerate(lines) if DECL in l and "//" not in l.split(DECL)[0]]
if len(hits) != 4:
    sys.stderr.write("MUT_DECL_HITS_UNEXPECTED %d in %s\n" % (len(hits), path))
    sys.exit(2)
for i in hits:
    lines[i] = lines[i].replace(DECL, BROKEN, 1) + " // MUT-"
io.open(path, "w", encoding="utf-8").write("\n".join(lines))
sys.stderr.write("MUT_DECL_RENAMED %s lines=%s\n" % (path, [i + 1 for i in hits]))
