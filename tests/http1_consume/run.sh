#!/bin/bash
# HTTP/1 消费口径判据台：用 build/ 里的整档对象，把 httpparse 的源换成指定版本、
# 再把 main 换成本目录的探针，链成 temp/ 下的探针二进制（绝不写 bin/paozhu）。
#
# 解析器符号闭包是整个产品（nm -u 上 httppeer/http_domain/router/serverconfig 都有未定义符），
# 所以不能用 tests/CMakeLists.txt 的 glob 目标，只能沿用 tests/multipart_boundary 的换链做法。
#
# 用法: run.sh [臂名] [被测源文件绝对路径] [探针源文件绝对路径]
#   run.sh                                   # base 臂：产品里的 http_parse.cpp + 本目录探针
#   run.sh mut_clamp /path/改过的http_parse.cpp     # 解析器牙齿臂
#   run.sh mut_cap "" /path/改过的探针.cpp            # 判据判别力臂（第 2 个参数留空 = 用产品解析器）
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/h1_consume"
B="$ROOT/build"
NAME="${1:-base}"
SRC="${2:-}"
[ -z "$SRC" ] && SRC="$ROOT/vendor/httpserver/src/http_parse.cpp"
PROBE="${3:-}"
[ -z "$PROBE" ] && PROBE="$HERE/h1_consume_probe.cpp"
REL="vendor/httpserver/src/http_parse.cpp"
LOG="$RUN/build_${NAME}.log"
BIN="$RUN/h1_consume_${NAME}"
DRIVERLOG="$RUN/driver_${NAME}.log"

mkdir -p "$RUN" "$RUN/out"
: > "$LOG"
# 臂中途失败时不许留下"看着像本轮"的旧读数，所以一开头就抹掉上一轮日志
: > "$DRIVERLOG"

if [ ! -f "$SRC" ]; then echo "NO_SRC $SRC"; exit 2; fi
if [ ! -f "$PROBE" ]; then echo "NO_PROBE $PROBE"; exit 2; fi
# 构建前断残留：牙齿臂的锚点只允许出现在 temp/ 的副本里，产品源必须干净
if grep -q 'MUT-' "$ROOT/vendor/httpserver/src/http_parse.cpp"; then echo "MARKER_LEFT_IN_PRODUCT"; exit 2; fi
if [ "$PROBE" = "$HERE/h1_consume_probe.cpp" ] && grep -q 'MUT-' "$PROBE"; then echo "MARKER_LEFT_IN_PROBE"; exit 2; fi
echo "SRC_MD5=$(md5 -q "$SRC") SRC_MTIME=$(stat -f %Sm -t '%m-%d %H:%M:%S' "$SRC")" >> "$LOG"
echo "PROBE_MD5=$(md5 -q "$PROBE") PROBE=$PROBE" >> "$LOG"

# 血统读数：换链用的每枚源，工作树是否比 build/ 里那枚 .o 新（新就必须现编）
for RELX in "$REL" vendor/httpserver/src/func.cpp vendor/httpserver/src/httppeer.cpp; do
    OX="$B/CMakeFiles/paozhu.dir/$RELX.o"
    if [ -f "$OX" ]; then
        if [ "$ROOT/$RELX" -nt "$OX" ]; then echo "LINEAGE_STALE $RELX" >> "$LOG"; else echo "LINEAGE_FRESH $RELX" >> "$LOG"; fi
    else
        echo "LINEAGE_MISSING $RELX" >> "$LOG"
    fi
done

FLAGS=$(python3 "$ROOT/tests/multipart_boundary/ccflags.py" "$ROOT" "$REL")
case "$FLAGS" in
    CC_*) echo "CCFLAGS_FAIL $FLAGS"; exit 3 ;;
esac
echo "FLAGS_OK len=${#FLAGS}" >> "$LOG"

/usr/bin/c++ $FLAGS -c "$SRC" -o "$RUN/${NAME}_parse.o" >> "$LOG" 2>&1
E1=$?
echo "CC_EXIT=$E1 $SRC" >> "$LOG"
/usr/bin/c++ $FLAGS -DH1_ARM_NAME="\"$NAME\"" -c "$PROBE" -o "$RUN/${NAME}_main.o" >> "$LOG" 2>&1
E2=$?
echo "CC_EXIT=$E2 $PROBE" >> "$LOG"
if [ "$E1" != "0" ] || [ "$E2" != "0" ]; then echo "COMPILE_FAILED $NAME"; exit 4; fi
echo "OBJ_MD5 parse=$(md5 -q "$RUN/${NAME}_parse.o") main=$(md5 -q "$RUN/${NAME}_main.o")" >> "$LOG"

LN=$(cat "$B/CMakeFiles/paozhu.dir/link.txt")
N_MAIN=$(printf '%s' "$LN" | grep -c -- 'CMakeFiles/paozhu.dir/startup/main_dev.cpp.o')
N_PARSE=$(printf '%s' "$LN" | grep -c -- "CMakeFiles/paozhu.dir/$REL.o")
N_OUT=$(printf '%s' "$LN" | grep -c -- '-o /Users/hzq/paozhu/bin/paozhu')
echo "ANCHORS main=$N_MAIN parse=$N_PARSE out=$N_OUT" >> "$LOG"
if [ "$N_MAIN" != "1" ] || [ "$N_PARSE" != "1" ] || [ "$N_OUT" != "1" ]; then echo "ANCHOR_MISS $NAME"; exit 5; fi

# 被测源之外，还要把工作树里改过的 httppeer.cpp/func.cpp 现编换进去：
# build/ 那两枚 .o 是上一轮整档编译的，可能早于它们各自的源改动，
# 而解析器 .o 是现编的，会吃到新旧不一致的 peer/clear 语义。
EXTRA_SED=""
for REL2 in vendor/httpserver/src/func.cpp vendor/httpserver/src/httppeer.cpp; do
    O2="$RUN/extra_$(printf '%s' "$REL2" | tr '/.' '__').o"
    if [ ! -f "$O2" ] || [ "$ROOT/$REL2" -nt "$O2" ]; then
        F2=$(python3 "$ROOT/tests/multipart_boundary/ccflags.py" "$ROOT" "$REL2")
        case "$F2" in
            CC_*) echo "CCFLAGS_FAIL $REL2 $F2"; exit 3 ;;
        esac
        /usr/bin/c++ $F2 -c "$ROOT/$REL2" -o "$O2" >> "$LOG" 2>&1
        E4=$?
        echo "CC_EXIT=$E4 $REL2" >> "$LOG"
        if [ "$E4" != "0" ]; then echo "COMPILE_FAILED $NAME extra $REL2"; exit 4; fi
        echo "EXTRA_BUILT $REL2 -> $O2" >> "$LOG"
    else
        echo "EXTRA_CACHED $REL2 -> $O2" >> "$LOG"
    fi
    C2=$(printf '%s' "$LN" | grep -c -- "CMakeFiles/paozhu.dir/$REL2.o")
    if [ "$C2" != "1" ]; then echo "EXTRA_ANCHOR_MISS $REL2 got=$C2 want=1"; exit 5; fi
    EXTRA_SED="$EXTRA_SED -e s#CMakeFiles/paozhu.dir/$REL2.o#$O2#"
done

LN2=$(printf '%s' "$LN" | sed \
  -e "s#CMakeFiles/paozhu.dir/startup/main_dev.cpp.o#$RUN/${NAME}_main.o#" \
  -e "s#CMakeFiles/paozhu.dir/$REL.o#$RUN/${NAME}_parse.o#" \
  $EXTRA_SED \
  -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $BIN#")
if printf '%s' "$LN2" | grep -q -- '/bin/paozhu'; then echo "OUT_REPLACEMENT_FAILED $NAME"; exit 6; fi

cd "$B" || exit 7
eval "$LN2" >> "$LOG" 2>&1
E3=$?
echo "LINK_EXIT=$E3" >> "$LOG"
if [ "$E3" != "0" ]; then echo "LINK_FAILED $NAME"; exit 8; fi
stat -f "%Sm %N" -t "%m-%d %H:%M:%S" "$BIN" >> "$LOG" 2>&1
echo "PROBE_ARM_OK $NAME $BIN" >> "$LOG"

# 判据台必须在仓库根跑：探针的 temp_path 是相对路径
# 先把上一次（或上一臂）的日志抹掉：臂中途失败时不许留下"看着像本轮"的旧读数
: > "$DRIVERLOG"
cd "$ROOT" || exit 9
"$BIN" > "$DRIVERLOG" 2>&1
E5=$?
echo "RUN_EXIT=$E5" >> "$DRIVERLOG"
echo "PROBE_RUN_DONE $NAME exit=$E5 log=$DRIVERLOG"
