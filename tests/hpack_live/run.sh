#!/bin/sh
# HPACK 编码器动态表 + HTTP/2 流式发送在线协议判据（tests/hpack_live/）。
# 探针: test_hpack_live（锁内联路径动态表同步）+ test_stream_live（内容环:
# 流式正文完整/分块上限/窗口停等恢复/中途 RST 死流丢弃/流式与内联交错）。
# 用法: tests/hpack_live/run.sh <paozhu 二进制> [port]
#
# 二进制必须是 Debug(+BENCHMARK) 档：Release 档入口是 main_daemon，会 chdir /tmp
# 回落线上 conf 抢 80/443，不能旁路起。Debug 档入口 main_dev 认 CWD 下的 conf/。
# 例: cmake --build build_debug_hpack --target paozhu   # 产物落共享 bin/paozhu
#     tests/hpack_live/run.sh bin/paozhu 18895
#     make -C build paozhu                               # 测完恢复 Release 链接
#
# 隔离台: temp/hpack_live_run/（conf 副本改端口，证书/orm/acme 软链回仓库，
# models/view/controller/www/libs/common/plugins 软链，temp/log/module 真目录）。
# 产物与日志: temp/hpack_live/（探针二进制、server.log、pid）。
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
BIN=${1:?usage: run.sh <debug paozhu binary> [port]}
PORT=${2:-18895}
SPORT=$((PORT + 1))
case "$BIN" in /*) ;; *) BIN="$ROOT/$BIN" ;; esac
[ -x "$BIN" ] || { echo "not executable: $BIN" >&2; exit 2; }

OUT="$ROOT/temp/hpack_live"
RUN="$ROOT/temp/hpack_live_run"
mkdir -p "$OUT" "$RUN/temp" "$RUN/log" "$RUN/module"

# 探针直编：链接面只有 http2_huffman.cpp（解码用），不依赖框架目标
/usr/bin/c++ -std=c++20 -DASIO_STANDALONE \
  -I"$ROOT/vendor/httpserver/include" -I"$ROOT/common" -I"$ROOT/libs/types" -I"$ROOT" \
  "$HERE/test_hpack_live.cpp" "$ROOT/vendor/httpserver/src/http2_huffman.cpp" \
  -o "$OUT/test_hpack_live"
/usr/bin/c++ -std=c++20 -DASIO_STANDALONE \
  -I"$ROOT/vendor/httpserver/include" -I"$ROOT/common" -I"$ROOT/libs/types" -I"$ROOT" \
  "$HERE/test_stream_live.cpp" "$ROOT/vendor/httpserver/src/http2_huffman.cpp" \
  -o "$OUT/test_stream_live"

# 隔离 conf：首次从仓库 conf 复制，端口换旁路；证书/orm/acme 软链回仓库（私钥不落副本）
if [ ! -f "$RUN/conf/server.conf" ]; then
  cp -R "$ROOT/conf" "$RUN/conf"
  for f in "$RUN/conf/"*.key "$RUN/conf/"*.pem "$RUN/conf/"*.crt "$RUN/conf/orm.conf" "$RUN/conf/acme.conf"; do
    [ -e "$f" ] || continue
    b=$(basename "$f")
    rm "$f"
    ln -s "$ROOT/conf/$b" "$f"
  done
fi
sed -i '' -E \
  -e "s/^(httpport[[:space:]]*=[[:space:]]*)[0-9]+/\1$PORT/" \
  -e "s/^(httpsport[[:space:]]*=[[:space:]]*)[0-9]+/\1$SPORT/" \
  "$RUN/conf/server.conf"
for d in models view controller www libs common plugins docs; do
  ln -sfn "$ROOT/$d" "$RUN/$d"
done

# 起实例（CWD 入口），等端口就绪。exec 让 $! 就是服务进程本身（Debug 档不 fork），
# 收尾 kill 才打得中；若误用 Release 档，main_daemon 会 chdir /tmp 抢 80/443，
# 端口等不到即退出——起之前先确认二进制是 Debug 档（体积明显大于 Release）。
(cd "$RUN" && exec "$BIN" > "$OUT/server.log" 2>&1) &
echo $! > "$OUT/server.pid"
PID=$(cat "$OUT/server.pid")
up=0
for _ in $(seq 1 40); do
  if nc -z -G 1 127.0.0.1 "$PORT" 2>/dev/null; then up=1; break; fi
  sleep 0.5
done
if [ "$up" != 1 ]; then
  echo "server did not come up on $PORT; log tail:" >&2
  tail -20 "$OUT/server.log" >&2
  kill "$PID" 2>/dev/null || true
  exit 3
fi

rc=0
"$OUT/test_hpack_live" "$PORT" || rc=1
"$OUT/test_stream_live" "$PORT" "$ROOT/docs/webpdf_tutorial.pdf" || rc=1

kill "$PID" 2>/dev/null || true
for _ in $(seq 1 20); do
  nc -z -G 1 127.0.0.1 "$PORT" 2>/dev/null || break
  sleep 0.5
done
echo "live probe exit=$rc (server pid $PID stopped)"
exit $rc