#!/bin/bash
# raw POST 内存快路径判据台的"牙齿臂"：每条把一处修复关掉，必须让对应判式变红。
# 判式若某臂下仍全绿，说明那条臂与原实现同语义（没牙），不是判据通过。
#
# 基线三枚源现编一次（http_parse / http2_parse / server），每条臂只重编被改的那枚解析器源，
# 其余沿用基线 fresh 对象；产物只落 temp/，绝不写 bin/paozhu。
# 依赖与 run.sh 相同的整档 Debug（ccflags 需要 compile_commands.json 里的 -fsanitize=address）。
#
# 用法: arms.sh          # 跑全部四臂
#       arms.sh h1:LE    # 只跑点名臂（leg:ARM）
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/raw_post_fastpath_run"
B="$ROOT/build"; LN="$B/CMakeFiles/paozhu.dir/link.txt"
mkdir -p "$RUN"
cd "$ROOT" || exit 9

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

MAIN() { # tier probe rel
    o="$RUN/$1_main.o"
    if [ ! -f "$o" ] || [ "$2" -nt "$o" ]; then
        fs=$(python3 "$HERE/ccflags.py" "$ROOT" "$3")
        /usr/bin/c++ $fs -c "$2" -o "$o" 2>"$RUN/cc_$1_main.log" || { echo "CC_PROBE_FAIL $1"; tail -20 "$RUN/cc_$1_main.log"; exit 1; }
    fi
}

LINKRUN() { # tier  parse_o  -> echo run.log 路径; parse_o 为空则用基线 fresh
    tier=$1; po=$2
    [ -z "$po" ] && po="$RUN/fresh_${tier}_parse.o"
    L2=$(cat "$LN" \
        | sed -e "s#CMakeFiles/paozhu.dir/startup/main_dev.cpp.o#$RUN/${tier}_main.o#" \
              -e "s#CMakeFiles/paozhu.dir/vendor/httpserver/src/http_parse.cpp.o#$( [ "$tier" = h1 ] && echo "$po" || echo "$RUN/fresh_h1_parse.o" )#" \
              -e "s#CMakeFiles/paozhu.dir/vendor/httpserver/src/http2_parse.cpp.o#$( [ "$tier" = h2 ] && echo "$po" || echo "$RUN/fresh_h2_parse.o" )#" \
              -e "s#CMakeFiles/paozhu.dir/vendor/httpserver/src/server.cpp.o#$RUN/fresh_server.o#" \
              -e "s#-o /Users/hzq/paozhu/bin/paozhu#-o $RUN/arm_${tier}_$(basename "$po" .o)#")
    printf '%s' "$L2" | grep -q "/bin/paozhu" && { echo "OUTFAIL $tier"; return 1; }
    ( cd "$B" && eval "$L2" ) >"$RUN/link_${tier}_$(basename "$po" .o).log" 2>&1 || { echo "LINK_FAIL $tier"; tail -20 "$RUN/link_${tier}_$(basename "$po" .o).log"; return 1; }
    ( cd "$ROOT" && "$RUN/arm_${tier}_$(basename "$po" .o)" ) > "$RUN/armrun_${tier}_$(basename "$po" .o).log" 2>&1
    echo "$RUN/armrun_${tier}_$(basename "$po" .o).log"
}

# --- 基线 fresh ---
fresh "$P1" fresh_h1_parse
fresh "$P2" fresh_h2_parse
fresh "$SV" fresh_server
MAIN h1 "$HERE/rp_h1_probe.cpp" "$P1"
MAIN h2 "$HERE/rp_h2_probe.cpp" "$P2"

ARMS_ALL=("h1:LE C2-at-cap-mem" "h1:FLUSH C4-prefix-flush-spill" "h2:LE H2-at-cap-mem" "h2:FLUSH2 H5-prefix-flush-spill")
if [ "$#" -gt 0 ]; then
    SEL=(); for w in "$@"; do for a in "${ARMS_ALL[@]}"; do [ "${a%% *}" = "$w" ] && SEL+=("$a"); done; done
    ARMS=("${SEL[@]}")
else
    ARMS=("${ARMS_ALL[@]}")
fi

OVERALL=0
for a in "${ARMS[@]}"; do
    set -- $a
    TIER=${1%%:*}; ARM=${1##*:}; WANT=$2
    if [ "$TIER" = h1 ]; then SRC="$P1"; SED="sed_${ARM}.txt"; else SRC="$P2"; SED="sed_${ARM}.txt"; fi
    MUT="$RUN/mut_${TIER}_${ARM}.cpp"
    cp "$ROOT/$SRC" "$MUT"
    sed -f "$HERE/$SED" "$MUT" > "$MUT.x" || { echo "SED_FAIL $ARM"; OVERALL=1; continue; }
    CHG=$(diff "$MUT" "$MUT.x" | grep -c '^<')
    if [ "$CHG" != "1" ]; then echo "ANCHOR_DRIFT $TIER:$ARM got=$CHG want=1"; OVERALL=1; rm -f "$MUT" "$MUT.x"; continue; fi
    mv "$MUT.x" "$MUT"
    PO="$RUN/mut_${TIER}_${ARM}.o"
    fs=$(python3 "$HERE/ccflags.py" "$ROOT" "$SRC")
    /usr/bin/c++ $fs -c "$MUT" -o "$PO" 2>"$RUN/cc_mut_${TIER}_${ARM}.log" || { echo "CC_MUT_FAIL $ARM"; tail -20 "$RUN/cc_mut_${TIER}_${ARM}.log"; OVERALL=1; continue; }
    LOG=$(LINKRUN "$TIER" "$PO" | tail -1)
    if [ ! -f "$LOG" ]; then echo "LINKRUN_FAIL $TIER:$ARM"; OVERALL=1; continue; fi
    RED=$(grep -c '^FAIL' "$LOG")
    if [ "$RED" -ge 1 ] && grep -q "^FAIL $WANT" "$LOG"; then
        echo "TEETH_OK $TIER:$ARM 期望红=$WANT 红数=$RED"
    else
        echo "NO_TEETH $TIER:$ARM 期望 $WANT 变红，实际 FAIL=$RED"; grep '^FAIL' "$LOG"; OVERALL=1
    fi
    rm -f "$MUT" "$MUT.x"
done
echo "ARMS_DONE overall=$OVERALL"
exit "$OVERALL"
