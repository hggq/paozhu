#!/usr/bin/env bash
# 编译并运行 POST 表单 obj_val 探针（对应 docx/request_h_post_form_validation_2026-10-27.md）
#
# 用法:
#   tests/postform/run_probe.sh            # ASan 编译并运行
#   tests/postform/run_probe.sh --no-asan  # 不带 ASan
#
# 退出码: 0 = 没有非预期失败; 1 = 有非预期失败或编译失败
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRC="$ROOT/tests/test_post_form_probe.cpp"
BIN_DIR="${BIN_DIR:-$ROOT/build/postform}"
mkdir -p "$BIN_DIR"

CXX="${CXX:-clang++}"
SAN="-fsanitize=address"
if [ "${1:-}" = "--no-asan" ]; then
    SAN=""
fi

echo "== compile =="
# shellcheck disable=SC2086
"$CXX" -std=c++20 -g $SAN -I "$ROOT/vendor/httpserver/include" \
    "$ROOT/vendor/httpserver/src/request.cpp" "$SRC" -o "$BIN_DIR/probe" || exit 1

# v3 已修复: NIL 上数字下标与 ["<n>"] 同路, 会自动建对象.
# v8 是契约项不是缺陷: 定稿方案 B 用 std::vector, operator[] 返回的引用在下一次
# 插入/删除后失效, 跨插入持有属于违约用法, 这里固定记为 expected-fail.
expected_fail() {
    case "$1" in
        v8) return 0 ;;
        *) return 1 ;;
    esac
}

pass=0
fail=0
known=0
for c in v1 v2 v3 v4 v5 v8 v10 v11 v12; do
    log="$BIN_DIR/$c.log"
    "$BIN_DIR/probe" "$c" >"$log" 2>&1
    rc=$?
    if [ "$rc" -eq 0 ]; then
        echo "[PASS] $c"
        pass=$((pass + 1))
    elif expected_fail "$c"; then
        echo "[EXPECTED-FAIL] $c (契约项: 跨插入持有引用, 见验证文档 V8)"
        known=$((known + 1))
        sed -n '1,8p' "$log" | sed 's/^/    /'
    else
        echo "[FAIL] $c"
        fail=$((fail + 1))
        sed -n '1,12p' "$log" | sed 's/^/    /'
    fi
done

echo
echo "pass=$pass fail=$fail expected-fail=$known"
[ "$fail" -eq 0 ]
