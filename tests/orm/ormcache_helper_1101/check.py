#!/usr/bin/env python3
# S1 结构臂 / S4 回退臂的读数器：按函数锚切体 + 花括号配平，不吃全文件计数。
# 用法：check.py [模板目录]   缺省 = 工作树的 vendor/httpserver/include
import io, os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/Users/hzq/paozhu/vendor/httpserver/include"
FILES = ["mysqlorm.hpp", "postgresqlorm.hpp", "sqliteorm.hpp"]
ANCHORS = {
    "get_data_cache": "        bool get_data_cache(std::size_t cache_key_name)\n",
    "get_record_cache": "        bool get_record_cache(std::size_t cache_key_name)\n",
}
CALLS_PER_HELPER = 6   # 实测：每方言每枚 6 个调用点 + 1 处定义 = 7 处出现

def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)

def slice_body(src, anchor):
    """返回 (函数体明文, 定义行号)；锚不唯一则返回 (None, 命中数)。"""
    hits = src.count(anchor)
    if hits != 1:
        return None, hits
    i = src.index(anchor)
    j = src.index("{", i + len(anchor))
    depth, k = 0, j
    while k < len(src):
        if src[k] == "{":
            depth += 1
        elif src[k] == "}":
            depth -= 1
            if depth == 0:
                break
        k += 1
    return src[j:k + 1], src[:i].count("\n") + 1

fails, checks = [], 0
def want(cond, label):
    global checks
    checks += 1
    if not cond:
        fails.append(label)

for f in FILES:
    path = os.path.join(ROOT, f)
    raw = io.open(path, encoding="utf-8").read()
    body_all = strip_comments(raw)
    for helper, anchor in ANCHORS.items():
        occ = body_all.count(helper + "(")   # 出现次数：1 处定义 + 6 个调用点
        want(occ == CALLS_PER_HELPER + 1,
             "%s %s OCC=%d expect %d" % (f, helper, occ, CALLS_PER_HELPER + 1))
        body, line = slice_body(body_all, anchor)
        if body is None:
            want(False, "%s %s ANCHOR_NOT_UNIQUE hits=%s" % (f, helper, line))
            continue
        want(body.count("= temp_cache.try_get(cache_key_name)") == 1,
             "%s %s L%d TRYGET=%d expect 1" % (f, helper, line, body.count("= temp_cache.try_get(cache_key_name)")))
        want("catch (" not in body, "%s %s L%d CATCH_LEFT" % (f, helper, line))
        want("= temp_cache.get(" not in body, "%s %s L%d THROWING_GET_LEFT" % (f, helper, line))
        want(body.count("return true;") == 1 and body.count("return false;") == 1,
             "%s %s L%d RETURN_SHAPE" % (f, helper, line))
        tail = "B_BASE::data_reset();" if helper == "get_data_cache" else "B_BASE::record.clear();"
        want(tail in body, "%s %s L%d MISS_CLEAR_MISSING(%s)" % (f, helper, line, tail))
        want(body.count("std::move(*hit)") == 1, "%s %s L%d MOVE_OUT=%d" % (f, helper, line, body.count("std::move(*hit)")))
        want("error_msg" not in body and "iserror" not in body and "unlock_conn" not in body,
             "%s %s L%d TOUCHES_ERROR_OR_CONN" % (f, helper, line))

print("CHECKS=%d" % checks)
print("CALLS_PER_HELPER_EACH=%d" % CALLS_PER_HELPER)
for m in fails:
    print("FAIL " + m)
print("RESULT %s fails=%d" % ("ALLPASS" if not fails else "HASFAIL", len(fails)))
sys.exit(0 if not fails else 1)
