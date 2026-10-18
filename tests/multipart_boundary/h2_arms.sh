#!/bin/bash
# HTTP/2 侧离线分帧枚举：底本是工作树 vendor/httpserver/src/http2_parse.cpp。
# 先跑基线（要求 abs_fail=0 且 total_fail=0），再逐条跑八臂突变（关掉一条移植看红是否恰好落在那一条）。
# 用法: h2_arms.sh                  # 基线 + 全部八臂
#       h2_arms.sh mu1 mu8          # 基线 + 点名臂
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
SUM="$RUN/h2arms_summary.log"
BASE="$ROOT/vendor/httpserver/src/http2_parse.cpp"
mkdir -p "$RUN"
: > "$SUM"

ARMS_ALL="mu1 mu2 mu3 mu4 mu5 mu6 mu7 mu8"
if [ "$#" -gt 0 ]; then ARMS="$*"; else ARMS="$ARMS_ALL"; fi

if [ ! -f "$BASE" ]; then echo "NO_BASE $BASE"; exit 2; fi
echo "BASE_MD5=$(md5 -q "$BASE")" >> "$SUM"

bash "$HERE/probe_variant.sh" h2 base "$BASE" >> "$SUM" 2>&1
if [ ! -x "$RUN/mp_h2_base" ]; then echo "BASE_BIN_MISSING"; exit 4; fi
cd "$ROOT" || exit 9
"$RUN/mp_h2_base" > "$RUN/h2run_base.log" 2>&1
echo "BASE_RUN_EXIT=$?" >> "$SUM"
grep -E '^H2PROBE |^ABSFAIL' "$RUN/h2run_base.log" >> "$SUM"
if ! grep -q 'abs_fail=0 total_fail=0' "$RUN/h2run_base.log"; then
    echo "BASELINE_RED 基线不干净，突变臂的红无法逐条归因" >> "$SUM"
fi

for A in $ARMS; do
    echo "=== ARM $A ===" >> "$SUM"
    python3 "$HERE/h2mut.py" "$A" >> "$SUM" 2>&1
    MUT="$RUN/h2mut_$A.cpp"
    if [ ! -f "$MUT" ]; then echo "GEN_MISSING $A" >> "$SUM"; continue; fi
    if cmp -s "$MUT" "$BASE"; then echo "GEN_NO_DIFF $A" >> "$SUM"; continue; fi
    bash "$HERE/probe_variant.sh" h2 "$A" "$MUT" >> "$SUM" 2>&1
    if [ ! -x "$RUN/mp_h2_$A" ]; then echo "BIN_MISSING $A" >> "$SUM"; continue; fi
    "$RUN/mp_h2_$A" > "$RUN/h2run_$A.log" 2>&1
    echo "RUN_EXIT=$? $A" >> "$SUM"
    grep -E '^H2PROBE |^ABSFAIL' "$RUN/h2run_$A.log" >> "$SUM"
    grep -oE '^FAIL [A-Za-z0-9_-]+' "$RUN/h2run_$A.log" | sort -u >> "$SUM"
done
echo "ALLDONE" >> "$SUM"
grep -E '^(BASE_MD5|BASE_RUN_EXIT|BASELINE_RED|=== ARM|MUT_OK|RUN_EXIT|H2PROBE |ABSFAIL|ANCHOR_COUNT|GEN_MISSING|GEN_NO_DIFF|BIN_MISSING|ALLDONE)' "$SUM"
