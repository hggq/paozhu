#!/bin/sh
# 过期归框架这一轮的判据台：真身 7 条 + 两枚头文件突变臂（各红各的）+ 结构锚点。
# 源码在 tests/orm/ormcache_ttl_1031/，产物与日志一律落 temp/ormcache_ttl_1031/。
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
HDR="$ROOT/vendor/httpserver/include/orm_cache.hpp"
SRC="$ROOT/tests/orm/ormcache_ttl_1031/probe.cpp"
W="$ROOT/temp/ormcache_ttl_1031"
if [ ! -f "$HDR" ] || [ ! -f "$SRC" ]; then
  printf 'ANCHOR_MISSING hdr=%s src=%s root=%s\n' "$HDR" "$SRC" "$ROOT"
  exit 8
fi
CXX=/usr/bin/c++
mkdir -p "$W/mut1" "$W/mut2"

say() { printf '%s\n' "$1"; }

run_probe() {
  # $1=头文件目录 $2=产物名 $3=日志
  "$CXX" -std=c++20 -w -I "$1" -I "$ROOT/vendor/httpserver/include" -o "$2" "$SRC" 2>> "$W/build.err"
  cc=$?
  say "CC_EXIT=$cc"
  if [ $cc -ne 0 ] || [ ! -x "$2" ]; then
    say "BUILD_MISSING $2"
    : > "$3"
    return 9
  fi
  "$2" > "$3" 2>&1
  say "PROBE_EXIT=$?"
  return 0
}

say "=== 1. 结构锚点（真身头文件）==="
say "deadline_calls=$(grep -c 'orm_cache_deadline(e' "$HDR")  # 期望 3：两枚 save + update"
say "deadline_def=$(grep -c 'inline unsigned int orm_cache_deadline' "$HDR")  # 期望 1"
say "default_ttl_macro=$(grep -c 'define ORM_CACHE_DEFAULT_TTL 120' "$HDR")  # 期望 1：19 轮把默认档从 60 升到 120"
say "old_default_60=$(grep -c 'define ORM_CACHE_DEFAULT_TTL 60' "$HDR")  # 期望 0：旧值不留残留"
say "max_ttl_macro=$(grep -c 'define ORM_CACHE_MAX_TTL 3600' "$HDR")  # 期望 1"
say "exptime_zero_write=$(grep -c 'temp.exptime = 0;' "$HDR")  # 期望 0：永久条目整条来源已断"
say "zero_sentinel_read=$(grep -c 'exptime == 0\|exptime != 0' "$HDR")  # 期望 0：==0 哨兵读写全删"
say "expnum_ifelse=$(grep -c 'expnum != 0' "$HDR")  # 期望 0"

say "=== 2. 真身探针：7 条全绿 ==="
run_probe "$ROOT/vendor/httpserver/include" "$W/probe" "$W/probe.log"
say "REAL_FAILS=$(grep -c '^FAIL' "$W/probe.log")  # 期望 0"
say "REAL_PASSES=$(grep -c '^PASS' "$W/probe.log")  # 期望 7"

say "=== 3. MUT-d0：默认档退回永久（臂必须恰红 A1/A4/A6）==="
sed 's/return orm_timeid() + ORM_CACHE_DEFAULT_TTL;/return 0;/' "$HDR" > "$W/mut1/orm_cache.hpp"
say "MUT1_LANDED=$(grep -c 'return 0;' "$W/mut1/orm_cache.hpp")  # 期望 1：突变确实落盘"
say "MUT1_KEPT_DEFAULT=$(grep -c 'define ORM_CACHE_DEFAULT_TTL' "$W/mut1/orm_cache.hpp")  # 期望 1：只改算式不改旋钮"
"$CXX" -std=c++20 -w -I "$W/mut1" -I "$ROOT/vendor/httpserver/include" -M "$SRC" 2>/dev/null > "$W/mut1.deps"
say "MUT1_INCLUDES_COPY=$(grep -c 'mut1/orm_cache.hpp' "$W/mut1.deps")  # 期望 1：这枚 TU 真吃到副本"
run_probe "$W/mut1" "$W/probe_mut1" "$W/probe_mut1.log"
say "MUT1_FAILS=$(grep -c '^FAIL' "$W/probe_mut1.log")  # 期望 3"
say "MUT1_RED_SET:"
grep '^FAIL' "$W/probe_mut1.log"

say "=== 4. MUT-noclamp：撤上界钳制（臂必须恰红 A2）==="
say "CLAMP_LINES_REAL=$(grep -c 'expnum = ORM_CACHE_MAX_TTL;' "$HDR")  # 期望 1：突变点唯一"
sed '/^[[:space:]]*expnum = ORM_CACHE_MAX_TTL;$/d' "$HDR" > "$W/mut2/orm_cache.hpp"
say "MUT2_LANDED=$(grep -c 'expnum = ORM_CACHE_MAX_TTL;' "$W/mut2/orm_cache.hpp")  # 期望 0：钳制赋值已删"
"$CXX" -std=c++20 -w -I "$W/mut2" -I "$ROOT/vendor/httpserver/include" -M "$SRC" 2>/dev/null > "$W/mut2.deps"
say "MUT2_INCLUDES_COPY=$(grep -c 'mut2/orm_cache.hpp' "$W/mut2.deps")  # 期望 1"
run_probe "$W/mut2" "$W/probe_mut2" "$W/probe_mut2.log"
say "MUT2_FAILS=$(grep -c '^FAIL' "$W/probe_mut2.log")  # 期望 1"
say "MUT2_RED_SET:"
grep '^FAIL' "$W/probe_mut2.log"

say "=== SUMMARY ==="
say "EXPECT_real='7 PASS / 0 FAIL'"
say "EXPECT_mut1='FAIL 恰 3 条: A1_default_not_permanent + A4_update_zero_expires + A6_default_entry_hits'"
say "EXPECT_mut2='FAIL 恰 1 条: A2_ttl_clamped'"
say "EXPECT_struct='deadline_calls=3 deadline_def=1 默认档 120=1 旧值 60=0 上界 3600=1 三枚 0 界=0/0/0'"
