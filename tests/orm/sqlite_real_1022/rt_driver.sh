#!/bin/bash
# 在新构建(整档重编 + lite 重生成)上复跑三套产物、三条牙齿臂与两条压测基线。
# 规则：同一时刻只允许一个探针进程(夹具表与 db 文件共享会打脏读数)⇒全部串行。
# 每臂四道门：驱动退出码 / 日志非空 / SUMMARY 行存在 / 链接集摘要与本轮其它臂一致。
set -u
# 脚本与探针源码在 tests/orm/<name>/，产物与日志一律落 temp/<name>/；ROOT 由脚本位置往上三层推出
SRC=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SRC/../../.." && pwd)
OUT=$ROOT/temp/$(basename "$SRC")
LOG=$OUT/rt_driver.log
mkdir -p "$OUT"
cd "$ROOT" || { echo "DRIVER ABORT cd" > "$LOG"; exit 2; }
: > "$LOG"

say() { echo "$@" >> "$LOG"; }

# 链接集摘要：link.txt 上除被换掉的那几枚之外的全部对象，按内容取 md5 后再取一次 md5。
# 用来证明本轮 base/fix/mut 各臂链的是同一批构建产物；驱动/池对象被单列记录。
link_digest() {
    ( cd "$ROOT/build/tests" && tr ' ' '\n' < CMakeFiles/test_sqlite_stress.dir/link.txt \
      | grep '\.o$' \
      | grep -v -e 'sqlite_conn.cpp.o$' -e 'orm_conn_pool.cpp.o$' -e 'test_sqlite_stress.cpp.o$' \
      | while read -r o; do [ -f "$o" ] && md5 -q "$o"; done | md5 -q ) 2>/dev/null
}

arm() {
    # $1=名字 $2=可执行 $3=族列表
    local name=$1 bin=$2 fams=$3 tag
    tag=$(echo "$fams" | tr ',' '_')
    local f="$OUT/rt_${name}_${tag}.log"
    "$bin" "$fams" > "$f" 2>&1
    local rc=$?
    local sz=$(wc -c < "$f")
    local sum=$(grep '^SUMMARY' "$f" | tail -1)
    say "ARM $name fams=$fams rc=$rc bytes=$sz $sum log=$f"
    if [ "$sz" -lt 100 ] || [ -z "$sum" ]; then
        say "GATE_FAIL $name 日志过小或无 SUMMARY"
    fi
}

say "== 产品源码血统 =="
say "driver $(md5 -q vendor/httpserver/src/sqlite_conn.cpp)"
say "header $(md5 -q vendor/httpserver/include/sqlite_conn.h)"
say "bin_paozhu $(stat -f '%Sm %z' -t '%F %T' bin/paozhu)"

# 有别的编译器在跑就先停：混着构建拿到的对象集不可比
if pgrep -x clang++ > /dev/null; then
    say "DRIVER ABORT 检测到并发编译(clang++)，等清场后重跑"
    exit 3
fi

say "== 链接集摘要(base 构建前) =="
say "DIGEST_pre $(link_digest)"

say "---- 一、base ----"
python3 "$SRC/build_probe.py" base > "$OUT/rt_build_base.log" 2>&1
say "BUILD base rc=$? $(tail -1 "$OUT/rt_build_base.log")"
say "DIGEST_base $(link_digest)"
arm base "$OUT/sqlite_real_probe" A,B
arm base "$OUT/sqlite_real_probe" C,E,R,F
arm base "$OUT/sqlite_real_probe" D
arm base "$OUT/sqlite_real_probe" G

say "---- 二、fix ----"
python3 "$SRC/make_fix.py" > "$OUT/rt_make_fix.log" 2>&1
say "MAKE_FIX rc=$? $(tail -1 "$OUT/rt_make_fix.log")"
python3 "$SRC/build_probe.py" fix > "$OUT/rt_build_fix.log" 2>&1
say "BUILD fix rc=$? $(grep '^HEADER_PATCHED' "$OUT/rt_build_fix.log" | head -1) $(tail -1 "$OUT/rt_build_fix.log")"
say "DIGEST_fix $(link_digest)"
arm fix "$OUT/sqlite_real_probe_fix" A,B
arm fix "$OUT/sqlite_real_probe_fix" C,E,R,F
arm fix "$OUT/sqlite_real_probe_fix" D
arm fix "$OUT/sqlite_real_probe_fix" G

say "---- 三、牙齿臂 MUT-R / MUT-B / MUT-T ----"
for M in MUT-R MUT-B MUT-T; do
    python3 "$SRC/make_mut.py" "$M" > "$OUT/rt_make_mut_${M}.log" 2>&1
    say "MAKE_MUT $M rc=$? $(tail -1 "$OUT/rt_make_mut_${M}.log")"
    python3 "$SRC/build_probe.py" mut > "$OUT/rt_build_mut_${M}.log" 2>&1
    say "BUILD mut($M) rc=$? $(tail -1 "$OUT/rt_build_mut_${M}.log")"
    say "DIGEST_mut_${M} $(link_digest)"
    case "$M" in
        MUT-R) arm mutR "$OUT/sqlite_real_probe_mut" E ;;
        MUT-B) arm mutB "$OUT/sqlite_real_probe_mut" A,B ;;
        MUT-T) arm mutT "$OUT/sqlite_real_probe_mut" C ;;
    esac
done

say "---- 四、既有压测基线(当前 bin/ 上的两条) ----"
say "stress_bin_sync $(stat -f '%Sm %z' -t '%F %T' bin/test_sqlite_stress)"
say "stress_bin_mt   $(stat -f '%Sm %z' -t '%F %T' bin/test_sqlite_stress_mt)"
./bin/test_sqlite_stress    > "$OUT/rt_stress_sync.log" 2>&1
say "STRESS sync rc=$? $(grep '\[TOTAL\]' "$OUT/rt_stress_sync.log" | tail -1)"
./bin/test_sqlite_stress_mt > "$OUT/rt_stress_mt.log" 2>&1
say "STRESS mt rc=$? $(grep '\[TOTAL\]' "$OUT/rt_stress_mt.log" | tail -1)"

say "== 链接集摘要(收尾) =="
say "DIGEST_end $(link_digest)"
say "DRIVER DONE"
