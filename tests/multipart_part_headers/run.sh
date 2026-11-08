#!/bin/bash
# tests/multipart_part_headers/run.sh
#
# 复现 multipart part 头解析的四个既有问题（只读探针，不改产品代码、不起服务、不写 bin/paozhu）。
#
# 做法：用 build/ 里 Debug(ASAN) 整档对象做底座，把 http_parse.cpp 与 http2_parse.cpp
# 现编两份换进链接，再把 main 换成 part_header_probe.cpp，链到 temp/ 下单独运行。
#
# 用法：
#   ./tests/multipart_part_headers/run.sh
#
# 前置：
#   - 仓库根存在 build/CMakeFiles/paozhu.dir/link.txt 与 build/compile_commands.json（Debug + ASAN 整档）
#   - 首次会自动现编两个解析器与探针，耗时相当于一次增量编译
#
# 输出：
#   逐条 [PASS]/[FAIL]（失败附期望 vs 实际 + 前 48 字节 hexdump），末尾 SUMMARY total=N pass=X fail=Y
# 退出码：
#   有用例失败 → 1；构建/链接/锚点问题 → 2..8（stderr 有对应标记）
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$ROOT/tests/multipart_part_headers"
B="$ROOT/build"
RUN="$ROOT/temp/multipart_part_headers"
BIN="$RUN/part_header_probe"
OUT="$RUN/out"
LOG="$RUN/build.log"

mkdir -p "$RUN" "$OUT"
rm -f "$RUN"/*.o "$BIN"
rm -f "$OUT"/* 2>/dev/null
: > "$LOG"

LINKF="$B/CMakeFiles/paozhu.dir/link.txt"
CFG="$B/compile_commands.json"
[ -f "$LINKF" ] || { echo "NO_LINK_TXT $LINKF" >&2; exit 2; }
[ -f "$CFG" ]   || { echo "NO_COMPILE_COMMANDS $CFG" >&2; exit 2; }
[ -f "$HERE/part_header_probe.cpp" ] || { echo "NO_PROBE_SRC" >&2; exit 2; }

FLAGS_H1="$(python3 "$ROOT/tests/multipart_boundary/ccflags.py" "$ROOT" vendor/httpserver/src/http_parse.cpp)"
case "$FLAGS_H1" in CC_*) echo "CCFLAGS_FAIL h1 $FLAGS_H1" >&2; exit 3 ;; esac
FLAGS_H2="$(python3 "$ROOT/tests/multipart_boundary/ccflags.py" "$ROOT" vendor/httpserver/src/http2_parse.cpp)"
case "$FLAGS_H2" in CC_*) echo "CCFLAGS_FAIL h2 $FLAGS_H2" >&2; exit 3 ;; esac
echo "FLAGS_OK h1=${#FLAGS_H1} h2=${#FLAGS_H2}" >> "$LOG"

# 两侧解析器现编（确保测的是工作区当前代码，而不是 build/ 里的旧对象）
/usr/bin/c++ $FLAGS_H1 -c "$ROOT/vendor/httpserver/src/http_parse.cpp" -o "$RUN/http_parse.o" >> "$LOG" 2>&1
E1=$?
/usr/bin/c++ $FLAGS_H2 -c "$ROOT/vendor/httpserver/src/http2_parse.cpp" -o "$RUN/http2_parse.o" >> "$LOG" 2>&1
E2=$?
if [ "$E1" != "0" ] || [ "$E2" != "0" ]; then
    echo "COMPILE_FAILED parser (h1=$E1 h2=$E2)" >&2
    tail -40 "$LOG" >&2
    exit 4
fi

# 探针现编（用 h1 的编译口径；两者 include 路径一致）
/usr/bin/c++ $FLAGS_H1 -c "$HERE/part_header_probe.cpp" -o "$RUN/probe_main.o" >> "$LOG" 2>&1
E3=$?
if [ "$E3" != "0" ]; then
    echo "COMPILE_FAILED probe=$E3" >&2
    tail -40 "$LOG" >&2
    exit 4
fi

LN="$(cat "$LINKF")"
N_MAIN=$(printf '%s' "$LN" | grep -c -- 'CMakeFiles/paozhu.dir/startup/main_dev.cpp.o')
N_H1=$(printf '%s' "$LN" | grep -c -- 'CMakeFiles/paozhu.dir/vendor/httpserver/src/http_parse.cpp.o')
N_H2=$(printf '%s' "$LN" | grep -c -- 'CMakeFiles/paozhu.dir/vendor/httpserver/src/http2_parse.cpp.o')
N_OUT=$(printf '%s' "$LN" | grep -c -- '-o /Users/hzq/paozhu/bin/paozhu')
echo "ANCHORS main=$N_MAIN h1=$N_H1 h2=$N_H2 out=$N_OUT" >> "$LOG"
if [ "$N_MAIN" != "1" ] || [ "$N_H1" != "1" ] || [ "$N_H2" != "1" ] || [ "$N_OUT" != "1" ]; then
    echo "ANCHOR_MISS main=$N_MAIN h1=$N_H1 h2=$N_H2 out=$N_OUT" >&2
    exit 5
fi

LN2=$(printf '%s' "$LN" | sed \
  -e "s#CMakeFiles/paozhu.dir/startup/main_dev.cpp.o#$RUN/probe_main.o#" \
  -e "s#CMakeFiles/paozhu.dir/vendor/httpserver/src/http_parse.cpp.o#$RUN/http_parse.o#" \
  -e "s#CMakeFiles/paozhu.dir/vendor/httpserver/src/http2_parse.cpp.o#$RUN/http2_parse.o#" \
  -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $BIN#")
if printf '%s' "$LN2" | grep -q -- '/bin/paozhu'; then
    echo "OUT_REPLACEMENT_FAILED" >&2
    exit 6
fi

cd "$B" || exit 7
eval "$LN2" >> "$LOG" 2>&1
E4=$?
echo "LINK_EXIT=$E4" >> "$LOG"
if [ "$E4" != "0" ]; then
    echo "LINK_FAILED" >&2
    tail -40 "$LOG" >&2
    exit 8
fi

cd "$ROOT" || exit 7
ASAN_OPTIONS=detect_leaks=0:halt_on_error=0 "$BIN" "$OUT"
RC=$?
rm -f "$OUT"/* 2>/dev/null
exit "$RC"
