#!/bin/sh
# 第二十一章判据台：ORM 文本 record 写回闸（read_is_clean）。
# 源码 tests/orm/ormcache_writeback_1101/，产物与日志 temp/ormcache_writeback_1101/。
# 臂序：S1 结构 -> S2 三方言编译 -> S3 可达性(改名) -> S4 牙齿(快照恒真) -> S5 漏改(退回一处)
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
HDR="$ROOT/vendor/httpserver/include"
PROBE_DIR="$ROOT/temp/ormprobe1018"
SRC="$ROOT/tests/orm/ormcache_writeback_1101"
W="$ROOT/temp/ormcache_writeback_1101"
CXX=/usr/bin/c++
if [ ! -d "$HDR" ] || [ ! -f "$PROBE_DIR/prep.py" ] || [ ! -f "$SRC/check.py" ]; then
  printf 'ANCHOR_MISSING hdr=%s probe=%s src=%s\n' "$HDR" "$PROBE_DIR/prep.py" "$SRC/check.py"
  exit 8
fi
mkdir -p "$W/mut_snap" "$W/mut_revert" "$W/reach_mysql" "$W/reach_pg" "$W/reach_lite"

say() { printf '%s\n' "$1"; }

compile_probe() {   # $1=前置 include 目录 $2=方言 $3=日志
  ( cd "$PROBE_DIR" && $CXX -std=c++20 -w -fsyntax-only -I"$1" -I"$HDR" -I"$SRC" $(cat flags.txt) "$SRC/probe_$2.cpp" ) > "$3" 2>&1
  cc=$?
  say "  PROBE probe_$2 EXIT=$cc DIAG=$(grep -c 'error:' "$3")"
  return $cc
}

say "=== 1. S1 结构臂（真身三模板，按函数锚切片）==="
python3 "$SRC/check.py" "$HDR" > "$W/check_real.log" 2>&1
say "CHECK_EXIT=$?  # 期望 0"
cat "$W/check_real.log"

say "=== 2. S2 编译臂（模板重生成 prepped 后三方言 -fsyntax-only）==="
( cd "$PROBE_DIR" && python3 prep.py ) > "$W/prep.log" 2>&1
say "PREP_EXIT=$?"
for t in mysqlorm postgresqlorm sqliteorm; do
  say "PREPPED_${t}_SNAP=$(grep -c 'bool read_is_clean = select_conn->error_msg.empty()' "$PROBE_DIR/prepped/$t.hpp")  # 期望 4：prepped 真吃到新闸门"
done
for t in mysql pg lite; do
  compile_probe "$PROBE_DIR/prepped" "$t" "$W/probe_$t.log"
  say "S2_${t}_EXIT=$?"
done

say "=== 3. S3 可达性臂（副本把 4 处快照声明改名 -> 该方言探针必须编译失败且红在 4 行）==="
for pair in "mysql mysqlorm" "pg postgresqlorm" "lite sqliteorm"; do
  set -- $pair
  d=$1; hf=$2
  cp "$PROBE_DIR/prepped/$hf.hpp" "$W/reach_$d/$hf.hpp"
  python3 "$SRC/mutate_decl_broken.py" "$W/reach_$d/$hf.hpp" > "$W/reach_$d/mut.log" 2>&1
  me=$?
  say "MUT3_${d}_EXIT=$me  # 期望 0（声明命中恰 4）"
  landed=$(grep -c 'read_is_clean_broken' "$W/reach_$d/$hf.hpp")
  say "MUT3_${d}_LANDED=$landed  # 期望 4：突变确实落盘"
  ppintact=$(grep -c 'read_is_clean_broken' "$PROBE_DIR/prepped/$hf.hpp")
  say "MUT3_${d}_PREPPED_INTACT=$ppintact  # 期望 0：prepped 未被污染"
  vdintact=$(grep -c 'read_is_clean_broken' "$HDR/$hf.hpp")
  say "MUT3_${d}_VENDOR_INTACT=$vdintact  # 期望 0：真身未被污染"
  rm -f "$W/probe_reach_$d.log"
  compile_probe "$W/reach_$d" "$d" "$W/probe_reach_$d.log"
  cce=$?
  say "MUT3_${d}_COMPILE_EXIT=$cce  # 期望非 0"
  und=$(grep -c "undeclared identifier 'read_is_clean'" "$W/probe_reach_$d.log")
  say "MUT3_${d}_UNDECLARED=$und  # 期望 >=4：4 道闸门各报一次"
  # 突变只改写行内文本，不增删行，所以副本行号与闸门行号可直接对齐
  grep -n 'if (iscache && read_is_clean)' "$W/reach_$d/$hf.hpp" | cut -d: -f1 | sort -u > "$W/reach_$d/gates.txt"
  grep -o "$hf\.hpp:[0-9]*:" "$W/probe_reach_$d.log" | cut -d: -f2 | sort -u > "$W/reach_$d/errlines.txt"
  gcount=$(wc -l < "$W/reach_$d/gates.txt")
  ecount=$(wc -l < "$W/reach_$d/errlines.txt")
  ongate=$(comm -12 "$W/reach_$d/gates.txt" "$W/reach_$d/errlines.txt" | wc -l)
  say "MUT3_${d}_GATE_LINES=$gcount ERR_LINES=$ecount ERR_ON_GATE=$ongate  # 期望 4 / >=4 / 4：红正落在那 4 道闸门=探针真实例化了这 4 个点"
done

say "=== 4. S4 牙齿臂（副本把 4 处快照换成恒真 -> 结构臂必须恰红 27 条）==="
for f in mysqlorm.hpp postgresqlorm.hpp sqliteorm.hpp; do cp "$HDR/$f" "$W/mut_snap/$f"; done
python3 "$SRC/mutate_snapshot_true.py" "$W/mut_snap/mysqlorm.hpp" > "$W/mut_snap/mut1.log" 2>&1
say "MUT4_1_EXIT=$?"
python3 "$SRC/mutate_snapshot_true.py" "$W/mut_snap/postgresqlorm.hpp" > /dev/null 2>&1
say "MUT4_2_EXIT=$?"
python3 "$SRC/mutate_snapshot_true.py" "$W/mut_snap/sqliteorm.hpp" > /dev/null 2>&1
say "MUT4_3_EXIT=$?  # 三条都期望 0"
say "MUT4_LANDED=$(grep -c 'bool read_is_clean = true; // MUT-' "$W/mut_snap/mysqlorm.hpp" "$W/mut_snap/postgresqlorm.hpp" "$W/mut_snap/sqliteorm.hpp" | paste -sd, -)  # 期望每个文件 4：四处快照全被改写"
say "MUT4_SNAP_GONE=$(grep -c 'bool read_is_clean = select_conn' "$W/mut_snap"/*.hpp | paste -sd, -)  # 期望每个文件 0"
python3 "$SRC/check.py" "$W/mut_snap" > "$W/check_mut4.log" 2>&1
say "MUT4_CHECK_EXIT=$?  # 期望 1"
say "MUT4_FAILS=$(grep -c '^FAIL ' "$W/check_mut4.log")  # 期望 27 = 3 文件 x (1 FILE_SNAP + 4 SNAP + 4 EVIDENCE)"
grep '^FAIL ' "$W/check_mut4.log" | awk '{print $2, $3, $5}' | sort | uniq -c > "$W/mut4_kinds.txt" 2>/dev/null
say "MUT4_RED_KINDS:"
sed 's/^/  /' "$W/mut4_kinds.txt"

say "=== 5. S5 漏改臂（副本只把 async_fetch 那一处闸门退回裸 iscache -> 结构臂恰红 3 条）==="
for f in mysqlorm.hpp postgresqlorm.hpp sqliteorm.hpp; do cp "$HDR/$f" "$W/mut_revert/$f"; done
python3 "$SRC/mutate_revert_one.py" "$W/mut_revert/mysqlorm.hpp" 1 > "$W/mut_revert/mut.log" 2>&1
say "MUT5_EXIT=$?  # 期望 0（快照之后 20 行内命中恰 1 道闸门）"
say "MUT5_LANDED=$(grep -c '^                if (iscache) // MUT-$' "$W/mut_revert/mysqlorm.hpp")  # 期望 1"
say "MUT5_GATE_LEFT=$(grep -c 'if (iscache && read_is_clean)' "$W/mut_revert/mysqlorm.hpp")  # 期望 3"
say "MUT5_OTHER_FILES_INTACT=$(grep -c 'MUT-' "$W/mut_revert/postgresqlorm.hpp" "$W/mut_revert/sqliteorm.hpp" | paste -sd, -)  # 期望每个 0"
python3 "$SRC/check.py" "$W/mut_revert" > "$W/check_mut5.log" 2>&1
say "MUT5_CHECK_EXIT=$?  # 期望 1"
say "MUT5_FAILS=$(grep -c '^FAIL ' "$W/check_mut5.log")  # 期望 3"
grep '^FAIL ' "$W/check_mut5.log" | sed 's/^/  /'

say "=== 6. 真身复原核对（判据台只动副本）==="
say "REAL_MUT_RESIDUE=$(grep -c 'MUT-' "$HDR/mysqlorm.hpp" "$HDR/postgresqlorm.hpp" "$HDR/sqliteorm.hpp" | paste -sd, -)  # 期望每个 0"
say "REAL_MD5:"
md5 "$HDR/mysqlorm.hpp" "$HDR/postgresqlorm.hpp" "$HDR/sqliteorm.hpp" | sed 's/^/  /'
