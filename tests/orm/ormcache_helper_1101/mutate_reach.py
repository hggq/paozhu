#!/usr/bin/env python3
# S3 可达性臂驱动器：只在 get_data_cache 体内把 try_get 改名，命中数必须为 1。
# 探针若因此编不过，才证明 S2 那三枚 -fsyntax-only 真的实例化到了这枚 helper。
# 切片按「函数锚 + 花括号配平」，不看列对齐、也不写死 meta 类型 ——
# prepped 副本里 `typename B_BASE::meta` 已被生成器换成具体类型，只有函数锚两边同形。
import io, sys

path = sys.argv[1]
src  = io.open(path, encoding="utf-8").read()

ANCHOR = "        bool get_data_cache(std::size_t cache_key_name)\n"
if src.count(ANCHOR) != 1:
    print("MUT_ANCHOR_HITS_UNEXPECTED %d in %s" % (src.count(ANCHOR), path))
    sys.exit(2)
i = src.index(ANCHOR)
j = src.index("{", i + len(ANCHOR))
depth, k = 0, j
while k < len(src):
    if src[k] == "{":
        depth += 1
    elif src[k] == "}":
        depth -= 1
        if depth == 0:
            break
    k += 1
body = src[j:k + 1]
n    = body.count("temp_cache.try_get(")
if n != 1:
    print("MUT_BODY_TRYGET_HITS_UNEXPECTED %d in %s" % (n, path))
    sys.exit(3)
io.open(path, "w", encoding="utf-8").write(src[:j] + body.replace("temp_cache.try_get(", "temp_cache.try_get_broken(") + src[k + 1:])
print("MUT3_RENAMED get_data_cache try_get -> try_get_broken body_len=%d" % len(body))
