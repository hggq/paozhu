#!/bin/bash
# 离线枚举台的一臂：用 build/ 里的 Debug 整档对象，把某个解析器源文件换成指定版本、
# 再把 main 换成分包/分帧探针，链成 temp/ 下的探针二进制（绝不写 bin/paozhu）。
# 用法: probe_variant.sh <h1|h2> <臂名> <解析器源文件绝对路径>
# 探针二进制落 temp/multipart_boundary_run/mp_<tier>_<name>，编译与换链日志落 build_<tier>_<name>.log。
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
B="$ROOT/build"
TIER="$1"
NAME="$2"
SRC="$3"
LOG="$RUN/build_${TIER}_${NAME}.log"
# 探针把上传正文落成临时文件再读回比对，落点目录必须先存在，否则文件判据全红
mkdir -p "$RUN" "$RUN/out" "$RUN/h2out"
: > "$LOG"

if [ "$TIER" = "h1" ]; then
    REL=vendor/httpserver/src/http_parse.cpp
    PROBE="$HERE/mp_probe.cpp"
elif [ "$TIER" = "h2" ]; then
    REL=vendor/httpserver/src/http2_parse.cpp
    PROBE="$HERE/h2_probe.cpp"
else
    echo "BAD_TIER $TIER (want h1|h2)"; exit 2
fi
if [ ! -f "$SRC" ]; then echo "NO_SRC $SRC"; exit 2; fi
if [ ! -f "$PROBE" ]; then echo "NO_PROBE $PROBE"; exit 2; fi

FLAGS=$(python3 "$HERE/ccflags.py" "$ROOT" "$REL")
case "$FLAGS" in
    CC_*) echo "CCFLAGS_FAIL $FLAGS"; exit 3 ;;
esac
echo "FLAGS_OK len=${#FLAGS}" >> "$LOG"

/usr/bin/c++ $FLAGS -c "$SRC" -o "$RUN/${TIER}_${NAME}_parse.o" >> "$LOG" 2>&1
E1=$?
echo "CC_EXIT=$E1 $SRC" >> "$LOG"
/usr/bin/c++ $FLAGS -c "$PROBE" -o "$RUN/mp_${TIER}_${NAME}_main.o" >> "$LOG" 2>&1
E2=$?
echo "CC_EXIT=$E2 $PROBE" >> "$LOG"
if [ "$E1" != "0" ] || [ "$E2" != "0" ]; then echo "COMPILE_FAILED $TIER $NAME"; tail -40 "$LOG"; exit 4; fi

LN=$(cat "$B/CMakeFiles/paozhu.dir/link.txt")
N_MAIN=$(printf '%s' "$LN" | grep -c -- 'CMakeFiles/paozhu.dir/startup/main_dev.cpp.o')
N_PARSE=$(printf '%s' "$LN" | grep -c -- "CMakeFiles/paozhu.dir/$REL.o")
N_OUT=$(printf '%s' "$LN" | grep -c -- '-o /Users/hzq/paozhu/bin/paozhu')
echo "ANCHORS main=$N_MAIN parse=$N_PARSE out=$N_OUT" >> "$LOG"
if [ "$N_MAIN" != "1" ] || [ "$N_PARSE" != "1" ] || [ "$N_OUT" != "1" ]; then echo "ANCHOR_MISS $TIER $NAME"; exit 5; fi

BIN="$RUN/mp_${TIER}_${NAME}"
LN2=$(printf '%s' "$LN" | sed \
  -e "s#CMakeFiles/paozhu.dir/startup/main_dev.cpp.o#$RUN/mp_${TIER}_${NAME}_main.o#" \
  -e "s#CMakeFiles/paozhu.dir/$REL.o#$RUN/${TIER}_${NAME}_parse.o#" \
  -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $BIN#")
if printf '%s' "$LN2" | grep -q -- '/bin/paozhu'; then echo "OUT_REPLACEMENT_FAILED $TIER $NAME"; exit 6; fi

cd "$B" || exit 7
eval "$LN2" >> "$LOG" 2>&1
E3=$?
echo "LINK_EXIT=$E3" >> "$LOG"
if [ "$E3" != "0" ]; then echo "LINK_FAILED $TIER $NAME"; tail -40 "$LOG"; exit 8; fi
stat -f "%Sm %N" -t "%m-%d %H:%M:%S" "$BIN" >> "$LOG" 2>&1
echo "PROBE_ARM_OK $TIER $NAME $BIN"
