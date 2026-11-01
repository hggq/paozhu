#!/usr/bin/env python3
# S4 回退臂驱动器：把副本里 get_record_cache 的整个函数体换成旧的「try + get() + catch」形状。
# 整段替换（不依赖列对齐），只动这一枚，其余五片必须不动 —— 用来证明结构臂有牙且归因不串。
import io, sys

path = sys.argv[1]
src  = io.open(path, encoding="utf-8").read()

ANCHOR = "        bool get_record_cache(std::size_t cache_key_name)\n"
OLD_BODY = """        {
            try
            {
                model_meta_cache<std::vector<typename B_BASE::meta>> &temp_cache = model_meta_cache<std::vector<typename B_BASE::meta>>::getinstance();
                B_BASE::record                                                   = temp_cache.get(cache_key_name);
                return true;
            }
            catch (const std::exception &e)
            {
            }

            B_BASE::record.clear();
            return false;
        }
"""

if src.count(ANCHOR) != 1:
    print("MUT_ANCHOR_HITS_UNEXPECTED %d in %s" % (src.count(ANCHOR), path))
    sys.exit(2)
i     = src.index(ANCHOR)
j     = src.index("{", i + len(ANCHOR))
depth, k = 0, j
while k < len(src):
    if src[k] == "{":
        depth += 1
    elif src[k] == "}":
        depth -= 1
        if depth == 0:
            break
    k += 1
end = k + 1
while end < len(src) and src[end] == "\n":
    end += 1
io.open(path, "w", encoding="utf-8").write(src[:j] + OLD_BODY + src[end:])
print("MUT4_REVERTED get_record_cache -> throw shape")
