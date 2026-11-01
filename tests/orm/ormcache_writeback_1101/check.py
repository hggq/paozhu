#!/usr/bin/env python3
# S1 结构臂 / S4-S5 牙齿臂的读数器：按函数锚切体 + 花括号配平，不吃全文件计数。
# 判"文本 record 写回闸"这一轮的落码形状：快照在归还连接之前、闸门带 read_is_clean、
# 而 meta 面（effect_num>0）、预编译面（!iserror）与 fetch_row 面（size()>0）一字未动。
# 用法：check.py [模板目录]   缺省 = 工作树的 vendor/httpserver/include
import io, os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/Users/hzq/paozhu/vendor/httpserver/include"
FILES = ["mysqlorm.hpp", "postgresqlorm.hpp", "sqliteorm.hpp"]
SNAP = "bool read_is_clean = select_conn->error_msg.empty() && !select_conn->isclose;"
GATE = "if (iscache && read_is_clean)"
ANCHORS = {
    "fetch": "        unsigned int fetch()\n",
    "async_fetch": "        asio::awaitable<unsigned int> async_fetch()\n",
    "fetch_append": "        M_MODEL &fetch_append()\n",
    "async_fetch_append": "        asio::awaitable<unsigned int> async_fetch_append()\n",
}
SITES_PER_FILE = 4
EXPECT_META = 4      # 4 处 meta 写回（effect_num>0），本轮刻意不动
EXPECT_DATA_SAVE = 4  # save_data_cache(exptime)
EXPECT_PREP_ERR = 2   # exec_fetch 面 if (iscache && !iserror)
EXPECT_PREP_ONE = 2   # exec_one 面 if (iscache && !iserror && effect_num > 0)
EXPECT_ROWGUARD = 1   # fetch_row 面 temprecord.size() > 0


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
    want(body_all.count(SNAP) == SITES_PER_FILE,
         "%s FILE_SNAP=%d expect %d" % (f, body_all.count(SNAP), SITES_PER_FILE))
    want(body_all.count(GATE) == SITES_PER_FILE,
         "%s FILE_GATE=%d expect %d" % (f, body_all.count(GATE), SITES_PER_FILE))
    want(body_all.count("if (iscache && effect_num > 0)") == EXPECT_META,
         "%s FILE_META_GATE=%d expect %d" % (f, body_all.count("if (iscache && effect_num > 0)"), EXPECT_META))
    want(body_all.count("save_data_cache(exptime);") == EXPECT_DATA_SAVE,
         "%s FILE_META_SAVE=%d expect %d" % (f, body_all.count("save_data_cache(exptime);"), EXPECT_DATA_SAVE))
    want(body_all.count("if (iscache && !iserror)") == EXPECT_PREP_ERR,
         "%s FILE_PREP_FETCH=%d expect %d" % (f, body_all.count("if (iscache && !iserror)"), EXPECT_PREP_ERR))
    want(body_all.count("if (iscache && !iserror && effect_num > 0)") == EXPECT_PREP_ONE,
         "%s FILE_PREP_ONE=%d expect %d" % (f, body_all.count("if (iscache && !iserror && effect_num > 0)"), EXPECT_PREP_ONE))
    want(body_all.count("if (temprecord.size() > 0)") == EXPECT_ROWGUARD,
         "%s FILE_ROW_GUARD=%d expect %d" % (f, body_all.count("if (temprecord.size() > 0)"), EXPECT_ROWGUARD))
    want(len(re.findall(r'if \(iscache\)\s*\{\s*save_cache\(exptime\)', body_all)) == 0,
         "%s FILE_BARE_GATE_LEFT=%d expect 0"
         % (f, len(re.findall(r'if \(iscache\)\s*\{\s*save_cache\(exptime\)', body_all))))

    for fn, anchor in ANCHORS.items():
        body, line = slice_body(body_all, anchor)
        if body is None:
            want(False, "%s %s ANCHOR_NOT_UNIQUE hits=%s" % (f, fn, line))
            continue
        want(body.count(SNAP) == 1, "%s %s L%d SNAP=%d expect 1" % (f, fn, line, body.count(SNAP)))
        m = re.search(r'bool read_is_clean = ([^;]*);', body)
        expr = m.group(1) if m else ""
        want("error_msg.empty()" in expr and "isclose" in expr,
             "%s %s L%d EVIDENCE(%r)" % (f, fn, line, expr))
        want(body.count(GATE) == 1, "%s %s L%d GATE=%d expect 1" % (f, fn, line, body.count(GATE)))
        if body.count(SNAP) != 1 or body.count(GATE) != 1:
            continue
        s = body.index(SNAP)
        g = body.index(GATE)
        want(s < g, "%s %s L%d SNAP_AFTER_GATE" % (f, fn, line))
        b = body.rfind("std::move(select_conn)", 0, g)
        want(b != -1 and s < b, "%s %s L%d SNAP_NOT_BEFORE_MOVEBACK(b=%d s=%d)" % (f, fn, line, b, s))
        after = body[g + len(GATE):g + len(GATE) + 220]
        want(re.match(r'\s*\{\s*save_cache\(exptime\);\s*exptime = 0;\s*iscache = false;\s*\}', after) is not None,
             "%s %s L%d BLOCK_BODY(%r)" % (f, fn, line, after[:90]))
        want("                if (iscache)\n" not in body, "%s %s L%d BARE_GATE_LEFT" % (f, fn, line))

print("CHECKS=%d" % checks)
print("SITES_PER_FILE=%d FILES=%d" % (SITES_PER_FILE, len(FILES)))
for m in fails:
    print("FAIL " + m)
print("RESULT %s fails=%d" % ("ALLPASS" if not fails else "HASFAIL", len(fails)))
sys.exit(0 if not fails else 1)
