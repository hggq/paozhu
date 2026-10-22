#!/usr/bin/env python3
# 落盘后的结构核对 + 单 TU 语法门（只 -fsyntax-only，不产出 .o、不碰 bin/paozhu）
import json, os, re, subprocess, sys

# ROOT 由脚本位置往上三层推出（探针目录在 tests/orm/<name>/，脚本自身那一层先剥掉），换机器不用改
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
SRC = ROOT + "/vendor/httpserver/src/mysql_conn.cpp"
text = open(SRC, encoding="utf-8").read()
lines = text.splitlines()

def count(pat):
    return len(re.findall(pat, text))

checks = [
    ("C-1 文本路径新终结判据", count(r"status == 0xFE && pkt_len < 0xFFFFFFu"), 2),
    ("C-2 大行错误文案(4 预编译+2 文本)", count(r"result row over 16MB \(0xFFFFFF split\)"), 6),
    ("C-3 文本版排空分支 skip_end", count(r"size_t skip_end = consumed \+ total_pkt_len;"), 2),
    ("C-6 payload too large 站点", count(r"COM_STMT_EXECUTE: payload too large"), 2),
]

def linestab(pat):
    return [i + 1 for i, ln in enumerate(lines) if re.search(pat, ln)]

def owner(lineno):
    """回溯最近的 col-0 函数定义行，给出归属函数名"""
    for i in range(lineno - 1, -1, -1):
        ln = lines[i]
        if ln and not ln[0].isspace() and "mysql_conn_base::" in ln:
            m = re.search(r"mysql_conn_base::(\w+)", ln)
            return m.group(1) if m else "?"
    return "?"

def owned(pat):
    return [(l, owner(l)) for l in linestab(pat)]

def phase_of(lineno):
    """回溯最近的 case phase::XXX: 标签，给出该判据所在相位"""
    for i in range(lineno - 1, -1, -1):
        m = re.match(r"\s*case phase::(\w+):", lines[i])
        if m:
            return m.group(1)
        if lines[i] and not lines[i][0].isspace() and "mysql_conn_base::" in lines[i]:
            return "<out>"
    return "?"

def owned_ph(pat):
    return [(l, owner(l), phase_of(l)) for l in linestab(pat)]

tfunc = {"fetch_directly_impl", "async_fetch_directly_impl"}
new_sites = owned_ph(r"status == 0xFE && pkt_len < 0xFFFFFFu")
bare_sites = owned_ph(r"if \(status == 0xFE\)\s*$")
bare_text = [s for s in bare_sites if s[1] in tfunc]
handoff_sites = owned_ph(r"if \(pkt_len == 0xFFFFFFu && expected_cols > 0")
for l, f, p in handoff_sites:
    window = "\n".join(lines[l:l + 5])
    ok = "current_phase = phase::ROWS;" in window and "consumed" not in window
    print(("PASS " if ok else "FAIL ") + "C-11 COLUMNS 交还 ROWS 且不消费 %s:%s/%s" % (l, f, p))
for pat, name in ((r"if \(!read_more\(\)\)", "R-1 sync 补读点"),
                  (r"unsigned int k = co_await async_read_loop\(\);", "R-2 async 内联补读"),
                  (r"result row over 16MB", "R-3 大行护栏归属")):
    print("READ %s -> %s" % (name, owned_ph(pat)))
print("READ R-4 新终结判据(行,函数,相位) -> %s" % new_sites)
print("READ R-5 裸 0xFE 判据全仓 -> %s" % bare_sites)
print("READ R-6 COLUMNS 交还点(行,函数,相位) -> %s" % handoff_sites)

ok4 = (len(new_sites) == 2 and all(f in tfunc and p == "ROWS" for _, f, p in new_sites)
       and len(bare_text) == 2 and all(p == "COLUMNS" for _, f, p in bare_text)
       and len(handoff_sites) == 2
       and all(f in tfunc and p == "COLUMNS" for _, f, p in handoff_sites))
print(("PASS " if ok4 else "FAIL ") + "C-4 新判据只在文本两函数的 ROWS 相位；COLUMNS 各有交还点、残留裸判据也只在 COLUMNS new=%s bare_text=%s handoff=%s" % (new_sites, bare_text, handoff_sites))

checks = [c for c in checks if c[0] != "C-1 文本路径新终结判据"]

# C-7: 每个 payload too large 站点其后 3 行内必须出现 error_code = 20
bad = []
for i, ln in enumerate(lines):
    if "COM_STMT_EXECUTE: payload too large" in ln:
        window = "\n".join(lines[i:i + 4])
        if "error_code = 20;" not in window:
            bad.append(i + 1)
checks.append(("C-7 payload 拒绝已置 error_code=20", 0 if not bad else 1, 0))
if bad:
    print("  C-7 未置位的行号:", bad)

for name, got, want in checks:
    print(("PASS " if got == want else "FAIL ") + name + " got=%d want=%d" % (got, want))

# C-8: 文本路径行解析本体仍在（未被排空分支吞掉）
for anchor in ("unsigned int row_start = consumed + 4;", "std::vector<char *> &name_ptrs = name_ptrs_cache;"):
    print(("PASS " if text.count(anchor) >= 2 else "FAIL ") + "C-8 行解析锚点保留: " + anchor[:40] + " x%d" % text.count(anchor))

# C-9: 三处文案
docs = [("AGENTS.md", "SET autocommit=0"), ("AGENTS_CN.md", "SET autocommit=0"),
        ("vendor/httpserver/include/mysql_conn.h", "SET autocommit=0")]
doc_fail = 0
for f, key in docs:
    d = open(ROOT + "/" + f, encoding="utf-8").read()
    n = d.count(key)
    doc_fail += (n == 0)
    print(("PASS " if n else "FAIL ") + "C-10 %s 含 %s x%d" % (f, key, n))

# 语法门
cc = json.load(open(ROOT + "/build/compile_commands.json"))
entry = next((e for e in cc if e["file"].endswith("vendor/httpserver/src/mysql_conn.cpp")), None)
if entry is None:
    print("FAIL 语法门: compile_commands.json 无 mysql_conn.cpp 条目")
    sys.exit(2)
cmd = entry.get("command")
if cmd is None:
    cmd = " ".join(entry["arguments"])
cmd = re.sub(r"\s-o\s+\S+", "", cmd)
cmd = cmd.replace(" -c ", " -fsyntax-only -c ")
print("语法门: dir=%s" % entry["directory"])
r = subprocess.run(cmd, shell=True, cwd=entry["directory"], capture_output=True, text=True)
out = (r.stdout + r.stderr)
os.makedirs(ROOT + "/temp/mysql_real_1022", exist_ok=True)
open(ROOT + "/temp/mysql_real_1022/syntax_mysql_conn.log", "w", encoding="utf-8").write(cmd + "\n----\n" + out)
print("语法门 rc=%d 输出行数=%d" % (r.returncode, len(out.splitlines())))
if r.returncode != 0:
    print(out[-4000:])
sys.exit(0 if r.returncode == 0 and doc_fail == 0 else 1)
