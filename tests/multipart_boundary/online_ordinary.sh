#!/bin/bash
# 普通提交数据的在线核对：urlencoded / json（合法与坏）/ octet-xml / 小 multipart /
# Content-Type 里 boundary 的各种写法（P5b 改的是 getcontenttype 收尾分支）。
# 用法: online_ordinary.sh [host:port]      # 默认 127.0.0.1:80
set -u
ROOT=/Users/hzq/paozhu
TARGET="${1:-127.0.0.1:80}"
RUN="$ROOT/temp/multipart_boundary_run/online"
mkdir -p "$RUN"
BD="PZORD1"
RED=0
N=0

# $1 名字 $2 路径 $3 Content-Type $4 数据文件 $5 期望正文子串（可空） $6 期望状态（默认 200）
arm() {
    local name="$1" path="$2" ctype="$3" data="$4" want="${5:-}" wantst="${6:-200}"
    local code body
    code=$(curl -s -o "$RUN/ord_body.tmp" -w '%{http_code}' -X POST \
        -H "Host: $TARGET" -H "Content-Type: $ctype" --data-binary "@$data" \
        --max-time 30 "http://$TARGET$path" 2>/dev/null)
    body=$(cat "$RUN/ord_body.tmp" 2>/dev/null)
    N=$((N + 1))
    local msg=""
    [ "$code" != "$wantst" ] && msg="$msg status=$code want=$wantst;"
    if [ -n "$want" ] && ! printf '%s' "$body" | grep -qF -- "$want"; then
        msg="$msg 缺期望读数[$want];"
    fi
    if [ -n "$msg" ]; then
        RED=$((RED + 1))
        printf '%-34s RED %s\n' "$name" "$msg"
    else
        printf '%-34s ok   st=%s bytes=%s\n' "$name" "$code" "$(printf '%s' "$body" | wc -c | tr -d ' ')"
    fi
}

# --- urlencoded ---
printf 'textfield2=V1&cc%%5B22%%5D%%5B%%5D=x&cc%%5Baabbcc%%5D%%5B%%5D=y&textarea2=T2&submit2=ok' > "$RUN/ord_url.bin"
arm "urlencoded-普通表单" /tformpost 'application/x-www-form-urlencoded' "$RUN/ord_url.bin" 'submit2:ok'

# --- json 合法 / 坏 ---
printf '{"firstName":"Ada","lastName":"L"    }' > "$RUN/ord_json.bin"
arm "json-合法" /tjsonpost 'application/json' "$RUN/ord_json.bin" 'firstName:Ada'
printf '{"firstName":"Ada",' > "$RUN/ord_json_bad.bin"
# 坏 json 只记读数不断言状态码（那是 F-5 那一层的事，本臂只核对"别把连接搞挂"）
arm "json-坏尾(只读数)" /tjsonpost 'application/json' "$RUN/ord_json_bad.bin" '' 400

# --- xml / octet-stream ---
printf '<root><a>1</a></root>' > "$RUN/ord_xml.bin"
arm "xml-rawcontent" /txmlupload 'application/xml' "$RUN/ord_xml.bin" 'rawcontent:<root>'
# octet-stream 走 case 5，正文落 files["rawcontent"]["tempfile"]，那页只打 rawcontent 字符串（空），
# 所以这一臂只能核对"解析没报错、页面正常出"，尺寸不在读数面。
arm "octet-rawcontent" /txmlupload 'application/octet-stream' "$RUN/ord_xml.bin" 'rawcontent:'

# --- 小 multipart（远不跨阈值） ---
mkmp() {
    # $1=Content-Type  $2=输出文件
    {
        printf -- '--%s\r\nContent-Disposition: form-data; name="textfield"\r\n\r\nOK-TEXT\r\n' "$BD"
        for i in 0 1 2 3 4 5 6 7 8; do
            printf -- '--%s\r\nContent-Disposition: form-data; name="aa[]"\r\n\r\naa-%s\r\n' "$BD" "$i"
        done
        printf -- '--%s\r\nContent-Disposition: form-data; name="textarea"\r\n\r\nPFX\r\n' "$BD"
        printf -- '--%s\r\nContent-Disposition: form-data; name="fileField"; filename="s.bin"\r\nContent-Type: application/octet-stream\r\n\r\nSMALL-DATA-1234\r\n' "$BD"
        printf -- '--%s\r\nContent-Disposition: form-data; name="submit"\r\n\r\nok\r\n' "$BD"
        printf -- '--%s--\r\n' "$BD"
    } > "$2"
}
mkmp '' "$RUN/ord_mp.bin"
CT_STD="multipart/form-data; boundary=$BD"
arm "multipart-小文件-标准写法" /tfilepost "$CT_STD" "$RUN/ord_mp.bin" 'fileField:size:15'
# 上一条若红了，把正文留下看字段
grep -qF 'submit:ok' "$RUN/ord_body.tmp" || cp "$RUN/ord_body.tmp" "$RUN/ord_mp_firstbody.txt"

printf 'multipart/form-data; boundary="%s"' "$BD" > "$RUN/ct_q.bin"
arm 'multipart-boundy带引号' /tfilepost "$(cat "$RUN/ct_q.bin")" "$RUN/ord_mp.bin" 'fileField:size:15'
printf 'multipart/form-data; boundary=%s ' "$BD" > "$RUN/ct_sp.bin"
arm 'multipart-boundy后带空格' /tfilepost "$(cat "$RUN/ct_sp.bin")" "$RUN/ord_mp.bin" 'fileField:size:15'
printf 'multipart/form-data; boundary=%s; charset=utf-8' "$BD" > "$RUN/ct_par.bin"
arm 'multipart-boundy后接参数' /tfilepost "$(cat "$RUN/ct_par.bin")" "$RUN/ord_mp.bin" 'fileField:size:15'
printf 'multipart/form-data;charset=utf-8; boundary=%s' "$BD" > "$RUN/ct_mid.bin"
arm 'multipart-boundy在末位' /tfilepost "$(cat "$RUN/ct_mid.bin")" "$RUN/ord_mp.bin" 'fileField:size:15'
printf 'multipart/form-data; boundary=' > "$RUN/ct_empty.bin"
arm 'multipart-boundy空值(应拒)' /tfilepost "$(cat "$RUN/ct_empty.bin")" "$RUN/ord_mp.bin" '' 400
printf 'multipart/form-data' > "$RUN/ct_none.bin"
arm 'multipart-无boundary参数(应拒)' /tfilepost "$(cat "$RUN/ct_none.bin")" "$RUN/ord_mp.bin" '' 400
echo "ORDINARY_SUMMARY target=$TARGET arms=$N red=$RED verdict=$( [ "$RED" = 0 ] && echo PASS || echo FAIL )"
[ "$RED" = 0 ] || exit 1
