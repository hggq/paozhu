#!/usr/bin/env python3
# HTTP/2 在线"刷盘缝"判据：用 bin/http2_multiclient 直连，每个 DATA 帧就是服务端一次解析，
# 帧长由 --frame-size 定死（16384），所以"某一帧正好停在文件 part 收尾的 \r 之后"
# 可以按构造算出来——不再靠 nghttp2 自己决定帧边界（那是 online_h2_big_curl.py 判不了这条的原因）。
#
# 构造：设某条流的正文（不含 HTTP/1 头）里，文件正文从偏移 S=len(head) 开始，正文长 L，
# 则文件 part 的收尾 CRLF 占偏移 S+L、S+L+1，闭合分隔符从 S+L+2 开始。
# 刷盘检查在每帧处理完之后做，file_field 的 field_item 在第 k 帧末 = k*F - S，
# 第一次跨过 2097152 的帧号 kf = floor((2097152+S)/F)+1。
#   A 臂取 L = kf*F - S - 1  → 第 kf 帧正好以 \r 结尾并刷盘，下一帧只剩以 \n 开头的分隔符
#   B 臂取 L = kf*F - S - 2  → 第 kf 帧以 \n 结尾并刷盘，下一帧首字节就是 '-'（CRLF 整体不可见）
#   C 臂取 L = kf*F - S - 3  → 下一帧开头是 \r\n--，两个字节约在本帧内可见（不该出问题）
#   D 臂 L 远小于阈值        → 全程不刷盘，作对照
# 判据：页面 fileField:size 必须等于 L、md5 必须等于源文件 md5、submit/textfield/aa[] 都还在。
# 用法: online_h2_seam.py [host:port] [path]
import hashlib
import os
import re
import subprocess
import sys

ROOT = "/Users/hzq/paozhu"
RUN = os.path.join(ROOT, "temp", "multipart_boundary_run", "online")
BIN = os.path.join(ROOT, "bin", "http2_multiclient")
BD = "PZH2SEAM"
TARGET = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:80"
PATH = sys.argv[2] if len(sys.argv) > 2 else "/tfilepost"
HOST, PORT = TARGET.split(":")
F = 16384
H2_FLUSH = 2097152

TAIL = ("\r\n--%s\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nok\r\n--%s--\r\n" % (BD, BD))


def head():
    h = "--%s\r\nContent-Disposition: form-data; name=\"textfield\"\r\n\r\nOK-TEXT\r\n" % BD
    for i in range(9):
        h += "--%s\r\nContent-Disposition: form-data; name=\"aa[]\"\r\n\r\naa-%d\r\n" % (BD, i)
    h += ("--%s\r\nContent-Disposition: form-data; name=\"fileField\"; filename=\"h2seam.bin\"\r\n"
          "Content-Type: application/octet-stream\r\n\r\n" % BD)
    return h


def make_body(L):
    hh = head()
    src = bytes((i * 31 + (i // 4096)) % 256 for i in range(L))
    return hh.encode() + src + TAIL.encode(), src, len(hh)


def unescape(s):
    out = []
    i = 0
    while i < len(s):
        if s[i] == "\\" and i + 4 <= len(s) and s[i + 1] == "x":
            out.append(chr(int(s[i + 2:i + 4], 16)))
            i += 4
        else:
            out.append(s[i])
            i += 1
    return "".join(out)


def run_client(body):
    p = os.path.join(RUN, "h2seam_body.bin")
    with open(p, "wb") as f:
        f.write(body)
    cmd = [BIN, "--host", HOST, "--port", PORT, "--path", PATH, "--authority", TARGET,
           "--method", "POST", "--content-type", "multipart/form-data; boundary=" + BD,
           "--body-file", p, "--frame-size", str(F), "--dump-body", "1",
           "--conns", "1", "--streams", "1", "--inflight", "1", "--deadline", "120000"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=200)
    return r.stdout + r.stderr


def judge(name, L, src, out):
    reds = []
    dumps = [unescape(m.group(1)) for m in re.finditer(r"BODY-DUMP \S+ sid=\d+ bytes=\d+ total=\d+ body=(.*)", out)]
    conn = re.search(r"CONN id=\d+ .*", out)
    page = dumps[0] if dumps else ""
    if not conn:
        reds.append("no-CONN-line")
    else:
        for key in ("completed", "status200"):
            v = re.search(key + r"=(\d+)", conn.group(0))
            if not v or int(v.group(1)) != 1:
                reds.append("%s!=1" % key)
        if "body_undelivered" in conn.group(0):
            reds.append("body_undelivered:" + re.search(r"body_undelivered=(\d+)", conn.group(0)).group(1))
        for key in ("rst", "stuck", "late"):
            v = re.search(key + r"=(\d+)", conn.group(0))
            if v and int(v.group(1)) != 0:
                reds.append("%s=%s" % (key, v.group(1)))
    m = re.search(r"fileField:size:(\d+)", page)
    got = int(m.group(1)) if m else -1
    if got != L:
        reds.append("size=%d want=%d delta=%d" % (got, L, got - L))
    t = re.search(r"fileField:tempfile:([^\x00-\x1f<]*)", page)
    tp = t.group(1).strip() if t else ""
    if not tp:
        reds.append("no-tempfile-path")
    else:
        abs_tp = tp if os.path.isabs(tp) else os.path.join(ROOT, tp)
        if not os.path.exists(abs_tp):
            reds.append("tempfile-missing:" + tp)
        else:
            with open(abs_tp, "rb") as f:
                data = f.read()
            if hashlib.md5(data).hexdigest() != hashlib.md5(src).hexdigest():
                reds.append("md5差(%d字节,%r)" % (len(data), data[-24:]))
            os.remove(abs_tp)
    if "submit:ok" not in page:
        reds.append("submit 丢失")
    if "textfield:OK-TEXT" not in page:
        reds.append("textfield 丢失")
    print("%-18s %s L=%d size=%s %s" % (name, "RED " if reds else "ok  ", L, got, "; ".join(reds)))
    return 0 if not reds else 1


def main():
    S = len(head())
    kf = (H2_FLUSH + S) // F + 1
    print("SEAM S=%d F=%d kf=%d 跨阈值帧末偏移=%d" % (S, F, kf, kf * F))
    arms = [
        ("A-缝在cr之后", kf * F - S - 1),
        ("B-缝在lf之后", kf * F - S - 2),
        ("C-缝在crlf之后", kf * F - S - 3),
        ("D-未跨阈值", 40000),
    ]
    red = 0
    for name, L in arms:
        body, src, _ = make_body(L)
        out = run_client(body)
        with open(os.path.join(RUN, "h2seam_%s.log" % name.split("-")[0]), "w") as f:
            f.write(out)
        red += judge(name, L, src, out)
    print("H2_SEAM_SUMMARY target=%s arms=%d red=%d verdict=%s"
          % (TARGET, len(arms), red, "PASS" if red == 0 else "FAIL"))
    return 1 if red else 0


sys.exit(main())
