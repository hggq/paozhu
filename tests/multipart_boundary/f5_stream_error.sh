#!/bin/bash
# h2 上一条流里的正文坏了，只该打死那一条流，不该把整条连接一起带走。
# 用法: f5_stream_error.sh [host:port]        # 默认 127.0.0.1:18890
# 前置: 服务端在跑（判据实例形状见 temp/h2c_run，httpport=18890）；客户端 bin/http2_multiclient 已编好。
# 判据两半，缺一不可：
#   坏 JSON 臂 goaway=0 且 rst=3 stuck=0 —— 连接活着，只有三条流各自被 RST；
#   合法 JSON 臂 completed=3 且 status200=3 —— 正向读数下界，否则"任何 POST 都被 RST"也能骗过第一半。
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
CLIENT="$ROOT/bin/http2_multiclient"
TARGET="${1:-127.0.0.1:18890}"
HOST="${TARGET%%:*}"
PORT="${TARGET##*:}"
STREAMS=3
mkdir -p "$RUN"

if [ ! -x "$CLIENT" ]; then echo "NO_CLIENT $CLIENT"; exit 2; fi

printf '{"a":1,' > "$RUN/h2_badjson.bin"
printf '{"a":1}' > "$RUN/h2_goodjson.bin"

run_one() {
    local tag="$1" body="$2"
    "$CLIENT" --host "$HOST" --port "$PORT" --path /tjsonpost --authority "$TARGET" \
        --method POST --content-type 'application/json' --body-file "$body" \
        --streams "$STREAMS" --conns 1 --inflight 1 --dump-body 1 \
        --deadline 30000 --settings-timeout 5000 > "$RUN/f5_$tag.log" 2>&1
}

run_one bad "$RUN/h2_badjson.bin"
run_one good "$RUN/h2_goodjson.bin"

BAD_GOAWAY=$(grep -c '^GOAWAY' "$RUN/f5_bad.log")
BAD_CONN=$(grep '^CONN ' "$RUN/f5_bad.log" | head -1)
GOOD_CONN=$(grep '^CONN ' "$RUN/f5_good.log" | head -1)
BAD_RST=$(printf '%s' "$BAD_CONN" | sed -nE 's/.* rst=([0-9]+).*/\1/p')
GOOD_DONE=$(printf '%s' "$GOOD_CONN" | sed -nE 's/.* completed=([0-9]+).*/\1/p')
GOOD_200=$(printf '%s' "$GOOD_CONN" | sed -nE 's/.* status200=([0-9]+).*/\1/p')

RED=0
if [ "$BAD_GOAWAY" != "0" ]; then echo "  RED bad-arm GOAWAY=$BAD_GOAWAY（整条连接被打断）"; RED=$((RED + 1)); fi
if [ "$BAD_RST" != "$STREAMS" ]; then echo "  RED bad-arm rst=${BAD_RST:-?} want=$STREAMS"; RED=$((RED + 1)); fi
if [ "$GOOD_DONE" != "$STREAMS" ] || [ "$GOOD_200" != "$STREAMS" ]; then
    echo "  RED good-arm completed=${GOOD_DONE:-?} status200=${GOOD_200:-?} want=$STREAMS"; RED=$((RED + 1));
fi
echo "BAD $BAD_CONN"
echo "GOOD $GOOD_CONN"
echo "F5_SUMMARY target=$TARGET bad_goaway=$BAD_GOAWAY bad_rst=${BAD_RST:-?} good_completed=${GOOD_DONE:-?} good_200=${GOOD_200:-?} red=$RED verdict=$( [ "$RED" = 0 ] && echo PASS || echo FAIL )"
[ "$RED" = 0 ] || exit 1
