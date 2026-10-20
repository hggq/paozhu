#!/bin/bash
# 离线枚举台的一臂：用 build/ 里的 Debug 整档对象，把某个解析器源文件换成指定版本、
# 再把 main 换成分包/分帧探针，链成 temp/ 下的探针二进制（绝不写 bin/paozhu）。
# 用法: probe_variant.sh <h1|h2|h1big|h2big|h1flush|name> <臂名> <被测源文件绝对路径> [额外换链源,逗号分隔]
# 例: probe_variant.sh h1big base "$ROOT/vendor/httpserver/src/http_parse.cpp" vendor/httpserver/src/func.cpp
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
elif [ "$TIER" = "h1big" ]; then
    REL=vendor/httpserver/src/http_parse.cpp
    PROBE="$HERE/mp_big_probe.cpp"
elif [ "$TIER" = "h2big" ]; then
    REL=vendor/httpserver/src/http2_parse.cpp
    PROBE="$HERE/h2_big_probe.cpp"
elif [ "$TIER" = "h1flush" ]; then
    REL=vendor/httpserver/src/http_parse.cpp
    PROBE="$HERE/mp_flush_probe.cpp"
elif [ "$TIER" = "name" ]; then
    # 被测对象是命名与文件名消毒（都在 func.cpp 里），解析器只作为调用方现编一份换进去
    REL=vendor/httpserver/src/func.cpp
    PROBE="$HERE/mp_name_probe.cpp"
else
    echo "BAD_TIER $TIER (want h1|h2|h1big|h2big|h1flush|name)"; exit 2
fi
if [ ! -f "$SRC" ]; then echo "NO_SRC $SRC"; exit 2; fi
if [ ! -f "$PROBE" ]; then echo "NO_PROBE $PROBE"; exit 2; fi
EXTRA="${4:-}"
# func.cpp 永远现编一份换进去：构建目录里那枚 .o 是上次整档编译的（12:26），
# 之后往 func.cpp 加的共享 helper（safe_filename、临时件命名）不会自动出现在里面，
# 而解析器 .o 是现编的、会引用这些符号 ⇒ 不换就直接 undefined symbol。
if [ "$REL" != "vendor/httpserver/src/func.cpp" ]; then
    case ",$EXTRA," in
        *",vendor/httpserver/src/func.cpp,"*) ;;
        *) EXTRA="vendor/httpserver/src/func.cpp${EXTRA:+,$EXTRA}" ;;
    esac
fi

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

# 第 4 个参数：逗号分隔的仓库相对路径，把这些源也现编一份换掉构建目录里的同名 .o。
# 用来验 func.cpp 这类"解析器之外、但被解析器调用"的改动（临时上传命名就在那儿）。
EXTRA_SED=""
for REL2 in $(printf '%s' "$EXTRA" | tr ',' ' '); do
    [ -z "$REL2" ] && continue
    F2=$(python3 "$HERE/ccflags.py" "$ROOT" "$REL2")
    case "$F2" in
        CC_*) echo "CCFLAGS_FAIL $REL2 $F2"; exit 3 ;;
    esac
    O2="$RUN/extra_$(printf '%s' "$REL2" | tr '/.' '__').o"
    if [ ! -f "$O2" ] || [ "$ROOT/$REL2" -nt "$O2" ]; then
        /usr/bin/c++ $F2 -c "$ROOT/$REL2" -o "$O2" >> "$LOG" 2>&1
        E4=$?
        echo "CC_EXIT=$E4 $REL2" >> "$LOG"
        if [ "$E4" != "0" ]; then echo "COMPILE_FAILED $TIER $NAME extra $REL2"; tail -40 "$LOG"; exit 4; fi
        echo "EXTRA_BUILT $REL2 -> $O2" >> "$LOG"
    else
        echo "EXTRA_CACHED $REL2 -> $O2" >> "$LOG"
    fi
    C2=$(printf '%s' "$LN" | grep -c -- "CMakeFiles/paozhu.dir/$REL2.o")
    if [ "$C2" != "1" ]; then echo "EXTRA_ANCHOR_MISS $REL2 got=$C2 want=1"; exit 5; fi
    EXTRA_SED="$EXTRA_SED -e s#CMakeFiles/paozhu.dir/$REL2.o#$O2#"
    echo "EXTRA_ARM $REL2 -> $O2" >> "$LOG"
done

BIN="$RUN/mp_${TIER}_${NAME}"
LN2=$(printf '%s' "$LN" | sed \
  -e "s#CMakeFiles/paozhu.dir/startup/main_dev.cpp.o#$RUN/mp_${TIER}_${NAME}_main.o#" \
  -e "s#CMakeFiles/paozhu.dir/$REL.o#$RUN/${TIER}_${NAME}_parse.o#" \
  $EXTRA_SED \
  -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $BIN#")
if printf '%s' "$LN2" | grep -q -- '/bin/paozhu'; then echo "OUT_REPLACEMENT_FAILED $TIER $NAME"; exit 6; fi

cd "$B" || exit 7
eval "$LN2" >> "$LOG" 2>&1
E3=$?
echo "LINK_EXIT=$E3" >> "$LOG"
if [ "$E3" != "0" ]; then echo "LINK_FAILED $TIER $NAME"; tail -40 "$LOG"; exit 8; fi
stat -f "%Sm %N" -t "%m-%d %H:%M:%S" "$BIN" >> "$LOG" 2>&1
echo "PROBE_ARM_OK $TIER $NAME $BIN"
