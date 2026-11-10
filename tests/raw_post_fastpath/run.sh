#!/bin/bash
# raw POST 内存快路径判据台（HTTP/1 + h2c，离线解析器探针）——基线腿。
#
# 与 multipart_boundary/probe_variant.sh 同一构建法：复用 build/ 里整档 Debug 的框架对象，
# 把 main_dev.cpp.o 换成探针 main，链成 temp/ 下的探针二进制（绝不写 bin/paozhu）。
#
# 与 multipart 那套的差别：本轮改动往 httpparse 加了一个数据成员 raw_spilled，sizeof(httpparse)
# 变了，而 server.cpp 用 make_unique<httpparse> 按 sizeof 分配。所以"只重编 http_parse.o、
# 其余 .o 用旧布局"会读到错位成员、判据假红——这里把 http_parse / http2_parse / server 三枚
# 源都按当前工作树现编一份再换链，使探针与框架用同一份布局，无需先整档 make。
# 前提只剩一条：build/ 是一次 Debug 整档构建（ccflags 取的口径含 -fsanitize=address，
# 且 build/ 里的其余 .o 只提供 httpparse/http2parse 的指针调用面，不新造它们的实例）。
#
# 用法: run.sh            # 跑 h1 + h2 两条腿
#       run.sh h1|h2      # 只跑一条
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/raw_post_fastpath_run"
B="$ROOT/build"
LN="$B/CMakeFiles/paozhu.dir/link.txt"
mkdir -p "$RUN" "$RUN/h1out" "$RUN/h2out"
cd "$ROOT" || exit 9

[ -f "$LN" ] || { echo "NO_LINK_TXT $LN (先在 build/ 跑 cmake+make Debug 整档)"; exit 2; }
[ -f "$B/compile_commands.json" ] || { echo "NO_COMPILE_DB $B/compile_commands.json"; exit 2; }

P1=vendor/httpserver/src/http_parse.cpp
P2=vendor/httpserver/src/http2_parse.cpp
SV=vendor/httpserver/src/server.cpp

fresh() { # src out_name
    o="$RUN/$2.o"
    fs=$(python3 "$HERE/ccflags.py" "$ROOT" "$1")
    case "$fs" in CC_*) echo "CCFAIL $1 $fs"; exit 1;; esac
    if [ ! -f "$o" ] || [ "$ROOT/$1" -nt "$o" ]; then
        /usr/bin/c++ $fs -c "$ROOT/$1" -o "$o" 2>"$RUN/cc_$2.log" || { echo "CC_SRC_FAIL $1"; tail -20 "$RUN/cc_$2.log"; exit 1; }
    fi
}

build_leg() { # tier probe rel
    TIER="$1"; PROBE="$2"; REL="$3"
    fs=$(python3 "$HERE/ccflags.py" "$ROOT" "$REL")
    case "$fs" in CC_*) echo "CCFAIL $REL $fs"; return 1;; esac
    /usr/bin/c++ $fs -c "$PROBE" -o "$RUN/${TIER}_main.o" 2>"$RUN/cc_${TIER}_main.log" || { echo "CC_PROBE_FAIL $TIER"; tail -30 "$RUN/cc_${TIER}_main.log"; return 1; }

    MAINO="CMakeFiles/paozhu.dir/startup/main_dev.cpp.o"
    N_MAIN=$(cat "$LN" | grep -c -- "$MAINO")
    OUTC=$(cat "$LN" | grep -c -- '-o /Users/hzq/paozhu/bin/paozhu')
    if [ "$N_MAIN" != "1" ] || [ "$OUTC" != "1" ]; then echo "ANCHOR_MISS $TIER main=$N_MAIN out=$OUTC"; return 5; fi

    BIN="$RUN/rp_${TIER}"
    L2=$(cat "$LN" \
        | sed -e "s#$MAINO#$RUN/${TIER}_main.o#" \
              -e "s#CMakeFiles/paozhu.dir/$P1.o#$RUN/fresh_h1_parse.o#" \
              -e "s#CMakeFiles/paozhu.dir/$P2.o#$RUN/fresh_h2_parse.o#" \
              -e "s#CMakeFiles/paozhu.dir/$SV.o#$RUN/fresh_server.o#" \
              -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $BIN#")
    printf '%s' "$L2" | grep -q "/bin/paozhu" && { echo "OUT_REPLACEMENT_FAILED $TIER"; return 6; }
    ( cd "$B" && eval "$L2" ) >"$RUN/link_${TIER}.log" 2>&1 || { echo "LINK_FAILED $TIER"; tail -30 "$RUN/link_${TIER}.log"; return 8; }

    ( cd "$ROOT" && "$BIN" ) > "$RUN/${TIER}_run.log" 2>&1
    E=$?
    echo "RUN_EXIT=$E $TIER"
    grep -E "^(FAIL|.*SUMMARY)" "$RUN/${TIER}_run.log"
    return "$E"
}

# 三枚被测/受影响源各现编一份，两条腿共用
fresh "$P1" fresh_h1_parse
fresh "$P2" fresh_h2_parse
fresh "$SV" fresh_server

WANT="${1:-both}"
RC=0
if [ "$WANT" = "both" ] || [ "$WANT" = "h1" ]; then
    build_leg h1 "$HERE/rp_h1_probe.cpp" "$P1" || RC=1
fi
if [ "$WANT" = "both" ] || [ "$WANT" = "h2" ]; then
    build_leg h2 "$HERE/rp_h2_probe.cpp" "$P2" || RC=1
fi
echo "RP_DONE rc=$RC"
exit "$RC"
