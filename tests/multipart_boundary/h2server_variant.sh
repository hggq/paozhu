#!/bin/bash
# 用 build/ 里的 Debug 整档对象，只把 vendor/httpserver/src/http2_parse.cpp 这一个对象换成指定源文件重链，
# 产物落 temp/multipart_boundary_run/srv_<name>，绝不写 bin/paozhu（三档共用同一个 bin，链接会打死在跑的实例）。
# 用法: h2server_variant.sh <http2_parse.cpp 源文件绝对路径> <name>
# 判据前置：除这一个文件外，build/ 之后不能再有别的已编译头改动，否则换出来的对象和其余 .o 不同源。
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
B="$ROOT/build"
SRC="$1"
NAME="$2"
LOG="$RUN/build_srv_$NAME.log"
REL=vendor/httpserver/src/http2_parse.cpp
mkdir -p "$RUN"
: > "$LOG"

if [ ! -f "$SRC" ]; then echo "NO_SRC $SRC"; exit 2; fi

FLAGS=$(python3 "$HERE/ccflags.py" "$ROOT" "$REL")
case "$FLAGS" in
    CC_*) echo "CCFLAGS_FAIL $FLAGS"; exit 3 ;;
esac
echo "FLAGS_OK len=${#FLAGS}" >> "$LOG"

/usr/bin/c++ $FLAGS -c "$SRC" -o "$RUN/srv_${NAME}_http2_parse.o" >> "$LOG" 2>&1
E1=$?
echo "CC_EXIT=$E1 $SRC" >> "$LOG"
if [ "$E1" != "0" ]; then echo "COMPILE_FAILED $NAME"; tail -40 "$LOG"; exit 4; fi

LN=$(cat "$B/CMakeFiles/paozhu.dir/link.txt")
N_PARSE=$(printf '%s' "$LN" | grep -c -- "CMakeFiles/paozhu.dir/$REL.o")
N_OUT=$(printf '%s' "$LN" | grep -c -- '-o /Users/hzq/paozhu/bin/paozhu')
echo "ANCHORS parse=$N_PARSE out=$N_OUT" >> "$LOG"
if [ "$N_PARSE" != "1" ] || [ "$N_OUT" != "1" ]; then echo "ANCHOR_MISS $NAME"; exit 5; fi

LN2=$(printf '%s' "$LN" | sed \
  -e "s#CMakeFiles/paozhu.dir/$REL.o#$RUN/srv_${NAME}_http2_parse.o#" \
  -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $RUN/srv_$NAME#")
if printf '%s' "$LN2" | grep -q -- '/bin/paozhu'; then echo "OUT_REPLACEMENT_FAILED $NAME"; exit 6; fi

cd "$B" || exit 7
eval "$LN2" >> "$LOG" 2>&1
E3=$?
echo "LINK_EXIT=$E3" >> "$LOG"
if [ "$E3" != "0" ]; then echo "LINK_FAILED $NAME"; tail -40 "$LOG"; exit 8; fi
stat -f "%Sm %N" -t "%m-%d %H:%M:%S" "$RUN/srv_$NAME" >> "$LOG" 2>&1
echo "SRV_ARM_OK $NAME $RUN/srv_$NAME"
