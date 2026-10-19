#!/bin/bash
# HTTP/1 侧离线分包枚举：底本就是工作树里的 vendor/httpserver/src/http_parse.cpp，
# 先跑基线（要求 abs_fail=0 且 total_fail=0），再逐条把某一个修复关掉跑突变臂。
# 每条臂的 sed 锚点命中数必须等于登记的期望值——漂了就报 ANCHOR_DRIFT 并跳过，
# 因为锚点没落地的臂会和上一条臂红集合逐字相同，那种"绿"或"红"都不能归因。
# 红集合为空的臂要单独看：多半是这条臂和原实现同语义（结构性防御），不是判据坏了。
# 用法: h1_arms.sh                 # 基线 + 全部六臂
#       h1_arms.sh M1 M5           # 基线 + 点名臂
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
REL=vendor/httpserver/src/http_parse.cpp
BASE="$ROOT/$REL"
SUM="$RUN/h1arms_summary.log"
mkdir -p "$RUN"
: > "$SUM"

PAIRS_ALL=("M1 1" "M2 1" "M3 3" "M4 1" "M5 1" "M6 3" "MB 1" "MC 1")
if [ "$#" -gt 0 ]; then
    PAIRS=()
    for want in "$@"; do
        for pair in "${PAIRS_ALL[@]}"; do
            set -- $pair
            [ "$1" = "$want" ] && PAIRS+=("$pair")
        done
    done
else
    PAIRS=("${PAIRS_ALL[@]}")
fi

if [ ! -f "$BASE" ]; then echo "NO_BASE $BASE"; exit 2; fi
echo "BASE_MD5=$(md5 -q "$BASE")" >> "$SUM"

# --- 基线：工作树原样 ---
bash "$HERE/probe_variant.sh" h1 base "$BASE" >> "$SUM" 2>&1
if [ ! -x "$RUN/mp_h1_base" ]; then echo "BASE_BIN_MISSING"; exit 4; fi
cd "$ROOT" || exit 9
"$RUN/mp_h1_base" > "$RUN/h1run_base.log" 2>&1
echo "BASE_RUN_EXIT=$?" >> "$SUM"
grep -E '^PROBE |^ABSFAIL' "$RUN/h1run_base.log" >> "$SUM"
if ! grep -q 'abs_fail=0 total_fail=0' "$RUN/h1run_base.log"; then
    echo "BASELINE_RED 基线不干净，突变臂的红无法逐条归因" >> "$SUM"
fi

for pair in "${PAIRS[@]}"; do
    set -- $pair
    V="$1"
    EXPECT="$2"
    echo "=== ARM $V expect_hits=$EXPECT ===" >> "$SUM"
    SRC="$RUN/h1mut_${V}.cpp"
    cp "$BASE" "$SRC" || { echo "CP_FAIL $V" >> "$SUM"; continue; }
    sed -f "$HERE/sed_${V}.txt" "$SRC" > "$SRC.mut" 2>>"$SUM" || { echo "SED_FAIL $V" >> "$SUM"; continue; }
    CHANGED=$(diff "$SRC" "$SRC.mut" | grep -c '^<')
    if [ "$CHANGED" != "$EXPECT" ]; then
        echo "ANCHOR_DRIFT $V got=$CHANGED want=$EXPECT" >> "$SUM"
        rm -f "$SRC" "$SRC.mut"
        continue
    fi
    mv "$SRC.mut" "$SRC" || { echo "MV_FAIL $V" >> "$SUM"; continue; }
    bash "$HERE/probe_variant.sh" h1 "$V" "$SRC" >> "$SUM" 2>&1
    if [ ! -x "$RUN/mp_h1_$V" ]; then echo "BIN_MISSING $V" >> "$SUM"; continue; fi
    "$RUN/mp_h1_$V" > "$RUN/h1run_$V.log" 2>&1
    echo "RUN_EXIT=$? $V" >> "$SUM"
    grep -E '^PROBE |^ABSFAIL' "$RUN/h1run_$V.log" >> "$SUM"
    grep -oE '^FAIL [A-Za-z0-9_-]+' "$RUN/h1run_$V.log" | sort -u >> "$SUM"
done
echo "ALLDONE" >> "$SUM"
grep -E '^(BASE_MD5|BASE_RUN_EXIT|BASELINE_RED|=== ARM|RUN_EXIT|PROBE |ABSFAIL|ANCHOR_DRIFT|BIN_MISSING|SED_FAIL|ALLDONE)' "$SUM"
