#!/usr/bin/env bash
# HTTP 集成验证：testformpost.cpp 的 4 个入口 + testpost.html 的两个表单
#
# 前置条件:
#   1. 服务已启动（bin/paozhu），且已注册以下路由:
#        /tformpost   application/x-www-form-urlencoded
#        /tfilepost   multipart/form-data
#        /tjsonpost   application/json
#        /txmlupload  application/xml
#   2. curl、python3 可用
#
# 用法:
#   BASE_URL=http://127.0.0.1:8080 tests/postform/post_form_curl.sh
#   默认 BASE_URL=http://127.0.0.1  （conf/server.conf 的 httpport=80）
#
# 退出码: 0 = 全部通过; 1 = 有失败
set -u

BASE_URL="${BASE_URL:-http://127.0.0.1}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0

check_contains() {
    local name="$1" out="$2" expect="$3"
    if printf '%s' "$out" | grep -qF -- "$expect"; then
        echo "[PASS] $name"
        pass=$((pass + 1))
    else
        echo "[FAIL] $name (missing: $expect)"
        printf '%s\n' "$out" | head -20 | sed 's/^/    /'
        fail=$((fail + 1))
    fi
}

check_regex() {
    local name="$1" out="$2" re="$3"
    if printf '%s' "$out" | grep -qE -- "$re"; then
        echo "[PASS] $name"
        pass=$((pass + 1))
    else
        echo "[FAIL] $name (regex: $re)"
        printf '%s\n' "$out" | head -20 | sed 's/^/    /'
        fail=$((fail + 1))
    fi
}

req() { curl -sS -m 20 "$@"; }

echo "base: $BASE_URL"

# ---------------------------------------------------------------- V1/V10
# form2 (testpost.html:56) → /tformpost
#   textfield2 / cc[22][] / cc[aabbcc][] / textarea2 / submit2
out=$(req -X POST "$BASE_URL/tformpost" \
    --data-urlencode 'textfield2=T2' \
    --data-urlencode 'cc[22][]=A' \
    --data-urlencode 'cc[aabbcc][]=B' \
    --data-urlencode 'textarea2=TA' \
    --data-urlencode 'submit2=S')

check_contains v1-textfield2 "$out" 'textfield2:T2'
check_contains v1-cc22 "$out" 'cc[22][]:A'
check_contains v1-cc-aabbcc "$out" 'cc[aabbcc][]:B'
check_contains v1-textarea2 "$out" 'textarea2:TA'
check_contains v1-submit2 "$out" 'submit2:S'

# V10: 值里的双引号/反斜杠必须原样回显，不引发 500
out=$(req -X POST "$BASE_URL/tformpost" --data-urlencode 'textfield2=a"b\c')
check_contains v10-value "$out" 'textfield2:a"b\c'

# ---------------------------------------------------------------- V11
# 重复标量字段: 期望 last-wins
out=$(req -X POST "$BASE_URL/tformpost" -d 'textfield2=A&textfield2=B')
check_contains v11-dup-scalar "$out" 'textfield2:B'

# 重复数组字段: cc[22][] 两次 -> 追加
out=$(req -X POST "$BASE_URL/tformpost" -d 'textfield2=T&cc[22][]=A&cc[22][]=B')
check_contains v11-array-append "$out" 'cc[22][]:A'

# ---------------------------------------------------------------- V5/V6
# form1 (testpost.html:26) → /tfilepost
printf 'post form upload fixture\n' >"$TMP/upload.txt"
out=$(req -X POST "$BASE_URL/tfilepost" \
    -F 'textfield=TF' \
    -F 'aa[]=A1' \
    -F 'aa[]=A2' \
    -F 'aa[8]=A8' \
    -F 'textarea=TA' \
    -F "fileField=@$TMP/upload.txt" \
    -F 'submit=S')

check_contains v5-textfield "$out" 'textfield:TF'
check_contains v5-aa0 "$out" 'aa[]:A1'
check_contains v5-aa1 "$out" 'aa[]:A2'
check_contains v5-aa8 "$out" 'aa[8]:A8'
check_contains v5-textarea "$out" 'textarea:TA'

for f in name filename tempfile size error; do
    check_regex "v6-file-$f" "$out" "fileField:$f:[^<]+"
done

# to_json 输出里应包含 post 的 key（testformpost.cpp:48）
check_regex v5-tojson "$out" '"(textfield|aa|textarea)"'
# aa[] + aa[8] 的解析结果是 OBJECT + 数字文本 key, 不补空位:
#   {"aa":{"0":"A1","1":"A2","8":"A8"}}
check_contains v5-tojson-object "$out" '"aa":{'
check_contains v5-tojson-aa0 "$out" '"0":"A1"'
check_contains v5-tojson-aa1 "$out" '"1":"A2"'
check_contains v5-tojson-aa8 "$out" '"8":"A8"'

# 等价性: /tformpost 也接受 aa[] 与 aa[8], 数字下标与文本 key 命中同一成员
out=$(req -X POST "$BASE_URL/tformpost" \
    -d 'textfield2=T&cc[22][]=A&aa[]=A1&aa[]=A2&aa[8]=A8')
check_contains v5-urlencoded-ok "$out" 'textfield2:T'
code=$(req -o /dev/null -w '%{http_code}' -X POST "$BASE_URL/tformpost" \
    -d 'aa[]=A1&aa[]=A2&aa[8]=A8')
case "$code" in
    5*) echo "[FAIL] v5-urlencoded-sparse (HTTP $code)"; fail=$((fail + 1)) ;;
    *)  echo "[PASS] v5-urlencoded-sparse (HTTP $code)"; pass=$((pass + 1)) ;;
esac

# ---------------------------------------------------------------- V7
# /tjsonpost (testformpost.cpp:78)
out=$(req -X POST "$BASE_URL/tjsonpost" \
    -H 'Content-Type: application/json' \
    -d '{"firstName":"Fred","lastName":"Flintstone"}')
check_contains v7-first "$out" 'firstName:Fred'
check_contains v7-last "$out" 'lastName:Flintstone'

# 畸形 JSON: 不应 5xx（isjson=false, json 已清空）
code=$(req -o /dev/null -w '%{http_code}' -X POST "$BASE_URL/tjsonpost" \
    -H 'Content-Type: application/json' -d '{"a":1}xyz')
case "$code" in
    5*) echo "[FAIL] v7-malformed (HTTP $code)"; fail=$((fail + 1)) ;;
    *)  echo "[PASS] v7-malformed (HTTP $code)"; pass=$((pass + 1)) ;;
esac

# ---------------------------------------------------------------- V12
# 1MB 字段值: 不应 5xx
python3 - "$TMP/big.txt" <<'PY'
import sys
with open(sys.argv[1], 'w') as f:
    f.write('x' * (1024 * 1024))
PY
code=$(req -o /dev/null -w '%{http_code}' -X POST "$BASE_URL/tformpost" \
    --data-urlencode "textfield2@$TMP/big.txt")
case "$code" in
    5*) echo "[FAIL] v12-1mb (HTTP $code)"; fail=$((fail + 1)) ;;
    *)  echo "[PASS] v12-1mb (HTTP $code)"; pass=$((pass + 1)) ;;
esac

# 130 层嵌套字段名: 不应 5xx
deep_name=$(python3 -c 'print("a" + "[b]" * 130)')
code=$(req -o /dev/null -w '%{http_code}' -X POST "$BASE_URL/tformpost" \
    --data-urlencode "$deep_name=1")
case "$code" in
    5*) echo "[FAIL] v12-deep-key (HTTP $code)"; fail=$((fail + 1)) ;;
    *)  echo "[PASS] v12-deep-key (HTTP $code)"; pass=$((pass + 1)) ;;
esac

echo
echo "pass=$pass fail=$fail"
echo "提示: V2/V3/V4/V8/V9 由 tests/postform/run_probe.sh 与 cmake --build build -j8 覆盖"
[ "$fail" -eq 0 ]
