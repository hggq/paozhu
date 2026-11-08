#!/bin/bash
# HTTP/1 POST chunked 38 组回归的一键入口。
#
# 复用 run.sh 的整档对象换链（探针二进制落 temp/h1_consume/，不写 bin/paozhu），
# 再对 driver 日志做判据：RUN_EXIT=0、恰好 38 组、零 FAIL、SUMMARY groups=38 failures=0。
#
# 用法：bash tests/http1_consume/run_chunked_regression.sh
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/h1_consume"
NAME="chunkedreg"
LOG="$RUN/driver_${NAME}.log"

"$HERE/run.sh" "$NAME" "" "$HERE/chunked_regression_probe.cpp" >/dev/null || {
    echo "ARM_FAILED $NAME (run.sh 中途失败，详见 temp/h1_consume/build_${NAME}.log)"
    exit 1
}

if ! grep -q '^RUN_EXIT=0' "$LOG"; then
    echo "RUN_EXIT_NOT_ZERO $NAME"
    tail -5 "$LOG"
    exit 1
fi

SUMMARY=$(grep '^SUMMARY ' "$LOG" || true)
if [ -z "$SUMMARY" ]; then
    echo "NO_SUMMARY $NAME"
    exit 1
fi

PASS_N=$(grep -c '^PASS: ' "$LOG" || true)
FAIL_N=$(grep -c '^FAIL: ' "$LOG" || true)

echo "$SUMMARY"
echo "PASS_LINES=$PASS_N FAIL_LINES=$FAIL_N"
if [ "$FAIL_N" != "0" ]; then
    echo "red: $(grep '^FAIL: ' "$LOG" | sed 's/^FAIL: \([a-z0-9_]*\).*/\1/' | tr '\n' ' ')"
fi

if [ "$PASS_N" != "38" ] || [ "$FAIL_N" != "0" ] ||
   ! printf '%s' "$SUMMARY" | grep -q 'groups=38 failures=0'; then
    echo "CHUNKED_REGRESSION_FAILED $NAME"
    exit 1
fi

echo "CHUNKED_REGRESSION_OK $NAME"
