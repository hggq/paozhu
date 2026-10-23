#!/bin/bash
# 一把跑完本轮全部臂：base + 三条解析器牙齿臂 + 一条探针判别力臂。
# 每臂的读数落 temp/h1_consume/driver_<臂名>.log，最后一屏给红集合汇总。
# 清理只用 find -delete：zsh/bash 的无匹配通配符会让整条 rm 不执行，旧日志会被当本轮读数。
set -u
ROOT=/Users/hzq/paozhu
HERE="$ROOT/tests/http1_consume"
RUN="$ROOT/temp/h1_consume"

rm -rf "$RUN"
mkdir -p "$RUN/out"

python3 "$HERE/mut.py" "$ROOT" || { echo "MUTGEN_FAILED"; exit 3; }

ARMS="base mut1 mut2 mut3 mutcap"
for K in $ARMS; do
    case "$K" in
        base)   "$HERE/run.sh" base ;;
        mutcap) "$HERE/run.sh" mutcap "" "$RUN/mut_probe_cap.cpp" ;;
        *)      "$HERE/run.sh" "$K" "$RUN/mut_${K#mut}_http_parse.cpp" ;;
    esac
    echo "ARM_DONE $K exit=$?"
done

echo "=== READINGS ==="
for K in $ARMS; do
    L="$RUN/driver_${K}.log"
    N=$(grep -c '^SUMMARY' "$L")
    if [ "$N" != "1" ]; then echo "$K NO_SUMMARY (arm failed; log not this round's reading)"; continue; fi
    echo "$K $(grep '^SUMMARY' "$L" | tr -d '\n') | red: $(grep '^FAIL' "$L" | sed 's/FAIL: \([a-z0-9_]*\).*/\1/' | tr '\n' ' ')"
done
