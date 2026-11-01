#!/bin/sh
# 第二十轮判据台：结构臂 + 三方言编译臂 + 两枚牙齿臂 + 代价读数。
# 源码 tests/orm/ormcache_helper_1101/，产物与日志 temp/ormcache_helper_1101/。
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
HDR="$ROOT/vendor/httpserver/include"
PROBE_DIR="$ROOT/temp/ormprobe1018"
SRC="$ROOT/tests/orm/ormcache_helper_1101"
W="$ROOT/temp/ormcache_helper_1101"
CXX=/usr/bin/c++
if [ ! -d "$HDR" ] || [ ! -f "$PROBE_DIR/prep.py" ] || [ ! -f "$SRC/check.py" ]; then
  printf 'ANCHOR_MISSING hdr=%s probe=%s src=%s\n' "$HDR" "$PROBE_DIR/prep.py" "$SRC/check.py"
  exit 8
fi
mkdir -p "$W/mut_reach" "$W/mut_throw" "$W/mutcheck"

say() { printf '%s\n' "$1"; }

compile_probe() {   # $1=前置 include 目录 $2=探针名 $3=日志
  ( cd "$PROBE_DIR" && $CXX -std=c++20 -w -fsyntax-only -I"$1" -I"$HDR" $(cat flags.txt) "$2.cpp" ) > "$3" 2>&1
  cc=$?
  say "  PROBE $2 EXIT=$cc DIAG=$(grep -c 'error:\|warning:' "$3")"
  return $cc
}

say "=== 1. S1 结构臂（真身三模板，按函数锚切片）==="
$CXX --version > /dev/null 2>&1
say "CHECKS_RUN:"
python3 "$SRC/check.py" "$HDR" > "$W/check_real.log" 2>&1
say "CHECK_EXIT=$?  # 期望 0"
cat "$W/check_real.log"

say "=== 2. S2 编译臂（模板重生成 prepped 后三方言 -fsyntax-only）==="
( cd "$PROBE_DIR" && python3 prep.py ) > "$W/prep.log" 2>&1
say "PREP_EXIT=$?"
for t in mysqlorm postgresqlorm sqliteorm; do
  say "PREPPED_${t}_CHANGED=$(grep -c '= temp_cache.try_get(cache_key_name)' "$PROBE_DIR/prepped/$t.hpp")  # 期望 2：两枚 helper 各 1，prepped 真吃到新本体"
done
for t in mysql pg lite; do
  compile_probe "$PROBE_DIR/prepped" "probe_$t" "$W/probe_$t.log"
  say "S2_${t}_EXIT=$?"
done

say "=== 3. S3 可达性臂（副本把 try_get 改名 -> probe_mysql 必须编译失败）==="
cp "$PROBE_DIR/prepped/mysqlorm.hpp" "$W/mut_reach/mysqlorm.hpp"
cp "$PROBE_DIR/prepped/postgresqlorm.hpp" "$PROBE_DIR/prepped/sqliteorm.hpp" "$W/mut_reach/" 2>/dev/null
python3 "$SRC/mutate_reach.py" "$W/mut_reach/mysqlorm.hpp"
say "MUT3_EXIT=$?  # 期望 0（锚命中恰 1）"
say "MUT3_LANDED=$(grep -c 'try_get_broken' "$W/mut_reach/mysqlorm.hpp")  # 期望 1：突变确实落盘"
say "MUT3_KEPT_TRYGET=$(grep -c '= temp_cache.try_get(cache_key_name)' "$W/mut_reach/mysqlorm.hpp")  # 期望 1：只动 data 那枚，record 那枚仍在"
say "MUT3_PREPPED_INTACT=$(grep -c 'try_get_broken' "$PROBE_DIR/prepped/mysqlorm.hpp")  # 期望 0：prepped 未被污染"
say "MUT3_VENDOR_INTACT=$(grep -c 'try_get_broken' "$HDR/mysqlorm.hpp")  # 期望 0：真身模板未被污染"
rm -f "$W/probe_mut3.log"
compile_probe "$W/mut_reach" "probe_mysql" "$W/probe_mut3.log"
say "MUT3_COMPILE_EXIT=$?  # 期望非 0"
LBL=$(grep -n 'try_get_broken' "$W/mut_reach/mysqlorm.hpp" | head -1 | cut -d: -f1)
say "MUT3_ERR_LINENO=$LBL  # 改名那一行的行号，诊断归因用它"
say "MUT3_DIAG_AT_ERRLINE=$(grep -c "mysqlorm.hpp:$LBL:" "$W/probe_mut3.log")  # 期望 >=1：红正落在那一行=这枚 helper 体内（S2 判别力的证明）"
say "MUT3_DIAG_NAMES_CACHE=$(grep -c 'model_meta_cache<' "$W/probe_mut3.log")  # 旁证：诊断点名缓存模板本体，说明该成员真被实例化"
grep 'error:' "$W/probe_mut3.log" | head -3 | sed 's/^/  /'

say "=== 4. S4 回退臂（副本把 record 那枚改回 try/get/catch -> 结构臂必须恰红那一片）==="
for f in mysqlorm.hpp postgresqlorm.hpp sqliteorm.hpp; do cp "$HDR/$f" "$W/mutcheck/$f"; done
python3 "$SRC/mutate_throw.py" "$W/mutcheck/mysqlorm.hpp"
say "MUT4_EXIT=$?  # 期望 0（锚命中恰 1；=2 表示这条臂根本没落盘）"
say "MUT4_LANDED=$(grep -c 'catch (const std::exception &e)' "$W/mutcheck/mysqlorm.hpp")  # 期望：比真身多 1"
say "MUT4_REAL_CATCH=$(grep -c 'catch (const std::exception &e)' "$HDR/mysqlorm.hpp")  # 真身基准数，供上条对照"
python3 "$SRC/check.py" "$W/mutcheck" > "$W/check_mut4.log" 2>&1
say "MUT4_CHECK_EXIT=$?  # 期望 1"
say "MUT4_RED_SET:"
grep '^FAIL' "$W/check_mut4.log" | sed 's/^/  /'

say "=== 5. S6 代价读数（冷 miss：抛异常 vs optional）==="
$CXX -std=c++20 -O2 -I"$HDR" -o "$W/bench" "$SRC/bench.cpp" 2> "$W/bench_build.err"
say "BENCH_CC_EXIT=$?"
if [ -x "$W/bench" ]; then
  "$W/bench" > "$W/bench.log" 2>&1
  say "BENCH_RUN_EXIT=$?  # 期望 0（N 次全 caught、0 次 hits）"
  cat "$W/bench.log"
else
  say "BENCH_MISSING"
  tail -5 "$W/bench_build.err"
fi

say "=== SUMMARY ==="
say "EXPECT_check='CHECKS=48 RESULT ALLPASS fails=0 + CHECK_EXIT=0'"
say "EXPECT_S2='三方言各 EXIT=0 DIAG=0，三份 prepped 各 CHANGED=2'"
say "EXPECT_S3='MUT3_EXIT=0 MUT3_LANDED=1 MUT3_KEPT_TRYGET=1 MUT3_PREPPED_INTACT=0 MUT3_VENDOR_INTACT=0 MUT3_COMPILE_EXIT!=0 MUT3_DIAG_AT_ERRLINE>=1'"
say "EXPECT_S4='MUT4_EXIT=0 恰红 4 条，全在 mysqlorm 的 get_record_cache 那一片：TRYGET=0 / CATCH_LEFT / THROWING_GET_LEFT / MOVE_OUT=0；pg 与 lite、以及 mysqlorm 的 get_data_cache 一片不动'"
say "EXPECT_S6='只报 ns/次与倍数，不做通过/失败判定'"
