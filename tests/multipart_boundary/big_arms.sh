#!/bin/bash
# 大文件 multipart 的分包/分帧判据台：文件正文跨过中途落盘阈值时，每一种切法都必须与整包逐字节同解。
# 先跑基线（要求 total_fail=0），再跑一条突变臂——把"落盘前先把分隔符前面两个字节留在内存里"关掉，
# 它的红必须逐条等于登记的用例失败数（只比总数不行：总数能被不同的分布凑出来）。
# 锚点命中数漂了就报 ANCHOR_DRIFT 并跳过那条臂，那种红不能归因。
# 末尾另跑刷盘单例台（h1flush 档，同一个 sed_BIGC 突变）。
# 用法: big_arms.sh          # h1 + h2 大文件两臂 + 刷盘台两臂 + 命名/消毒台两臂
#       big_arms.sh h1       # 只跑 h1 大文件两臂
#       big_arms.sh flush    # 只跑刷盘台两臂
#       big_arms.sh name     # 只跑命名唯一性 + filename 消毒台
set -u
ROOT=/Users/hzq/paozhu
HERE="$(cd "$(dirname "$0")" && pwd)"
RUN="$ROOT/temp/multipart_boundary_run"
SUM="$RUN/bigarms_summary.log"
mkdir -p "$RUN" "$RUN/out" "$RUN/h2out"
: > "$SUM"

# 期望：total 后面每个进判据用例的 fail 数。
# D-two-bigfiles（一个请求两个大文件 part）以前只出 READONLY：临时上传名在同一次调用里会重播
# 同一串随机字，两个 part 撞同一个文件，整包参照本身被污染（读数在 12/13 之间摆）。
# make_http_temp_upload_name 换成微秒时间戳 + 进程内自增号之后，这条臂 3 轮实测恒为
# base 0 / nocarry 12，已并回判据。
want_of() {
    case "$1/$2" in
        h1/base)    echo "0 A-legit-bigfile=0 B-legit-smallfile=0 C-lf-only-in-bigfile=0 D-two-bigfiles=0 E-small-two-files=0" ;;
        h2/base)    echo "0 A-legit-bigfile=0 B-legit-smallfile=0 C-lf-only-in-bigfile=0" ;;
        h1/nocarry) echo "27 A-legit-bigfile=12 B-legit-smallfile=0 C-lf-only-in-bigfile=3 D-two-bigfiles=12 E-small-two-files=0" ;;
        h2/nocarry) echo "15 A-legit-bigfile=12 B-legit-smallfile=0 C-lf-only-in-bigfile=3" ;;
        *)          echo "BAD_WANT $1 $2" ;;
    esac
}

TIERS="h1big h2big"
RUNFLUSH=1
RUNNAME=1
if [ "$#" -gt 0 ]; then
    case "$1" in
        h1)    TIERS="h1big" ;;
        h2)    TIERS="h2big" ;;
        flush) TIERS=""; RUNFLUSH=0 ;;
        name)  TIERS=""; RUNFLUSH=0 ;;
        *)     echo "BAD_ARG $1 (want h1|h2|flush|name)"; exit 2 ;;
    esac
    if [ "$1" = "flush" ]; then RUNNAME=0; fi
    if [ "$1" = "name" ]; then RUNFLUSH=0; fi
fi

# 突变标记不许留在工作树里：留着的话基线臂跑的就是被改过的源
for REL in vendor/httpserver/src/http_parse.cpp vendor/httpserver/src/http2_parse.cpp vendor/httpserver/src/func.cpp; do
    N=$(grep -c 'MUT-' "$ROOT/$REL")
    echo "SRC $REL md5=$(md5 -q "$ROOT/$REL") mut_marks=$N" >> "$SUM"
    if [ "$N" != "0" ]; then echo "MUT_RESIDUE $REL"; exit 3; fi
done

FAIL=0
for TIER in $TIERS; do
    case "$TIER" in
        h1big) PLAIN=h1; REL=vendor/httpserver/src/http_parse.cpp ;;
        h2big) PLAIN=h2; REL=vendor/httpserver/src/http2_parse.cpp ;;
    esac
    BASE="$ROOT/$REL"

    for ARM in base nocarry; do
        SRC="$BASE"
        if [ "$ARM" = "nocarry" ]; then
            SRC="$RUN/${TIER}mut_BIGC.cpp"
            cp "$BASE" "$SRC" || { echo "CP_FAIL $TIER $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            sed -f "$HERE/sed_BIGC.txt" "$SRC" > "$SRC.mut" 2>>"$SUM" || { echo "SED_FAIL $TIER $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            CHANGED=$(diff "$SRC" "$SRC.mut" | grep -c '^<')
            if [ "$CHANGED" != "2" ]; then
                echo "ANCHOR_DRIFT $TIER $ARM got=$CHANGED want=2" >> "$SUM"
                rm -f "$SRC" "$SRC.mut"
                FAIL=$((FAIL + 1))
                continue
            fi
            mv "$SRC.mut" "$SRC"
        fi

        bash "$HERE/probe_variant.sh" "$TIER" "$ARM" "$SRC" vendor/httpserver/src/func.cpp >> "$SUM" 2>&1
        BIN="$RUN/mp_${TIER}_${ARM}"
        if [ ! -x "$BIN" ]; then echo "BIN_MISSING $TIER $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; fi
        cd "$ROOT" || exit 9
        "$BIN" > "$RUN/bigrun_${TIER}_${ARM}.log" 2>&1
        echo "RUN_EXIT=$? $TIER $ARM" >> "$SUM"

        # 逐条核对：先总数，再每个用例的 fail
        read -r W_TOT rest <<<"$(want_of "$PLAIN" "$ARM")"
        if [ "$W_TOT" = "BAD_WANT" ]; then echo "BAD_WANT $TIER $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; fi
        GOT_TOT=$(grep -oE 'total_fail=[0-9]+' "$RUN/bigrun_${TIER}_${ARM}.log" | tail -1 | cut -d= -f2)
        if [ "${GOT_TOT:-none}" != "$W_TOT" ]; then
            echo "ARM_RED_MISMATCH $TIER $ARM total=$GOT_TOT want=$W_TOT" >> "$SUM"
            FAIL=$((FAIL + 1))
        fi
        for KV in $rest; do
            NAME=${KV%%=*}
            WANT=${KV##*=}
            GOT=$(grep -E "^SUMMARY $NAME checked=[0-9]+ fail=" "$RUN/bigrun_${TIER}_${ARM}.log" | tail -1 | sed -E 's/.*fail=([0-9]+).*/\1/')
            if [ "${GOT:-none}" != "$WANT" ]; then
                echo "CASE_MISMATCH $TIER $ARM $NAME fail=${GOT:-missing} want=$WANT" >> "$SUM"
                FAIL=$((FAIL + 1))
            fi
        done
        grep -E '^(SUMMARY|READONLY|BIGPROBE|H2BIGPROBE)' "$RUN/bigrun_${TIER}_${ARM}.log" >> "$SUM"
        echo "ARM_DONE $TIER $ARM" >> "$SUM"
    done
done

# --- 刷盘单例台：h1flush 档，同一个 sed_BIGC 突变 ---
# 判据是相对式的（每种切包读数必须等于整包读数），所以每条臂的期望是固定读数：
# base 全绿；nocarry 红恰在 case1 的 3 个切点上。
if [ "$RUNFLUSH" = "1" ]; then
    for ARM in base nocarry; do
        WANT_TOT=0
        WANT_C1=0
        if [ "$ARM" = "nocarry" ]; then
            WANT_TOT=3
            WANT_C1=3
        fi
        SRC="$ROOT/vendor/httpserver/src/http_parse.cpp"
        if [ "$ARM" = "nocarry" ]; then
            SRC="$RUN/h1flushmut_BIGC.cpp"
            cp "$ROOT/vendor/httpserver/src/http_parse.cpp" "$SRC" || { echo "CP_FAIL flush $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            sed -f "$HERE/sed_BIGC.txt" "$SRC" > "$SRC.mut" 2>>"$SUM" || { echo "SED_FAIL flush $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            CHANGED=$(diff "$SRC" "$SRC.mut" | grep -c '^<')
            if [ "$CHANGED" != "2" ]; then
                echo "ANCHOR_DRIFT flush $ARM got=$CHANGED want=2" >> "$SUM"
                rm -f "$SRC" "$SRC.mut"
                FAIL=$((FAIL + 1))
                continue
            fi
            mv "$SRC.mut" "$SRC"
        fi
        bash "$HERE/probe_variant.sh" h1flush "$ARM" "$SRC" vendor/httpserver/src/func.cpp >> "$SUM" 2>&1
        BIN="$RUN/mp_h1flush_$ARM"
        if [ ! -x "$BIN" ]; then echo "BIN_MISSING flush $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; fi
        cd "$ROOT" || exit 9
        "$BIN" > "$RUN/flushrun_$ARM.log" 2>&1
        echo "RUN_EXIT=$? flush $ARM" >> "$SUM"
        GOT_TOT=$(grep -oE '^RESULT fail=[0-9]+' "$RUN/flushrun_$ARM.log" | tail -1 | cut -d= -f2)
        GOT_C1=$(grep -oE '^RESULT case1 fail=[0-9]+' "$RUN/flushrun_$ARM.log" | tail -1 | cut -d= -f2)
        [ "${GOT_TOT:-none}" = "$WANT_TOT" ] || { echo "CASE_MISMATCH flush $ARM total=${GOT_TOT:-missing} want=$WANT_TOT" >> "$SUM"; FAIL=$((FAIL + 1)); }
        [ "${GOT_C1:-none}" = "$WANT_C1" ] || { echo "CASE_MISMATCH flush $ARM case1=${GOT_C1:-missing} want=$WANT_C1" >> "$SUM"; FAIL=$((FAIL + 1)); }
        grep -E '^RESULT' "$RUN/flushrun_$ARM.log" >> "$SUM"
        echo "ARM_DONE flush $ARM" >> "$SUM"
    done
fi

# --- 命名唯一性 + 上传文件名消毒台：name 档（被测源是 func.cpp，解析器现编一份当调用方）---
# 接线门先立住：两个解析器里凡是把 filename 写进 peer->files 的地方，除 rawcontent 那个字面量以外
# 必须全部走 safe_filename()。这条只数现存接线，红在"接线漂移"而不是行为，所以单独一个键名。
if [ "$RUNNAME" = "1" ]; then
    for F in http_parse.cpp http2_parse.cpp; do
        ALL=$(grep -c '\["filename"\] = ' "$ROOT/vendor/httpserver/src/$F")
        SAFE=$(grep -c '\["filename"\] = safe_filename' "$ROOT/vendor/httpserver/src/$F")
        echo "WIRING $F assign=$ALL sanitized=$SAFE" >> "$SUM"
        if [ "$ALL" != "8" ] || [ "$SAFE" != "7" ]; then
            echo "WIRING_DRIFT $F assign=$ALL want=8 sanitized=$SAFE want=7" >> "$SUM"
            FAIL=$((FAIL + 1))
        fi
    done
    for ARM in base collide; do
        WANT_TOT=0
        if [ "$ARM" = "collide" ]; then
            WANT_TOT=5
        fi
        SRC="$ROOT/vendor/httpserver/src/func.cpp"
        if [ "$ARM" = "collide" ]; then
            SRC="$RUN/namemut_NAME.cpp"
            cp "$ROOT/vendor/httpserver/src/func.cpp" "$SRC" || { echo "CP_FAIL name $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            sed -f "$HERE/sed_NAME.txt" "$SRC" > "$SRC.mut" 2>>"$SUM" || { echo "SED_FAIL name $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; }
            CHANGED=$(diff "$SRC" "$SRC.mut" | grep -c '^<')
            if [ "$CHANGED" != "1" ]; then
                echo "ANCHOR_DRIFT name $ARM got=$CHANGED want=1" >> "$SUM"
                rm -f "$SRC" "$SRC.mut"
                FAIL=$((FAIL + 1))
                continue
            fi
            mv "$SRC.mut" "$SRC"
        fi
        bash "$HERE/probe_variant.sh" name "$ARM" "$SRC" vendor/httpserver/src/http_parse.cpp >> "$SUM" 2>&1
        BIN="$RUN/mp_name_$ARM"
        if [ ! -x "$BIN" ]; then echo "BIN_MISSING name $ARM" >> "$SUM"; FAIL=$((FAIL + 1)); continue; fi
        cd "$ROOT" || exit 9
        "$BIN" > "$RUN/namerun_$ARM.log" 2>&1
        echo "RUN_EXIT=$? name $ARM" >> "$SUM"
        GOT_TOT=$(grep -oE '^NAMEPROBE total_fail=[0-9]+' "$RUN/namerun_$ARM.log" | tail -1 | cut -d= -f2)
        [ "${GOT_TOT:-none}" = "$WANT_TOT" ] || { echo "CASE_MISMATCH name $ARM total=${GOT_TOT:-missing} want=$WANT_TOT" >> "$SUM"; FAIL=$((FAIL + 1)); }
        grep -E '^(UNIQ|TABLE|PARSE|SUMMARY|NAMEPROBE)' "$RUN/namerun_$ARM.log" >> "$SUM"
        echo "ARM_DONE name $ARM" >> "$SUM"
    done
fi

echo "BIGARMS_FAIL=$FAIL" >> "$SUM"
echo "ALLDONE" >> "$SUM"
grep -E '^(SRC|RUN_EXIT|ARM_DONE|ARM_RED_MISMATCH|CASE_MISMATCH|ANCHOR_DRIFT|BIN_MISSING|SED_FAIL|CP_FAIL|BAD_WANT|MUT_RESIDUE|WIRING|WIRING_DRIFT|UNIQ|TABLE|PARSE|SUMMARY|NAMEPROBE|BIGARMS_FAIL|ALLDONE|total_fail)' "$SUM"
[ "$FAIL" = "0" ] || exit 1
