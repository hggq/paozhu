#!/usr/bin/env python3
# 在线大文件上传判据：对着正在跑的实例（默认 127.0.0.1:80）真发一次 multipart POST，
# 把 body 按选定的下标切成两次 write（中间 sleep），让服务端"看见"的包缝落在指定位置。
# 判据：回显的 fileField:size 必须等于源文件字节数，落盘临时文件的 md5 必须等于源文件 md5，
#       且后续字段（submit）仍在。
# 用法: python3 online_big_upload.py [host:port]
import hashlib
import os
import re
import socket
import sys
import time

ROOT = "/Users/hzq/paozhu"
RUN = os.path.join(ROOT, "temp", "multipart_boundary_run", "online")
BD = "PZMPBIG7"
TARGET = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:80"
HOST, PORT = TARGET.split(":")
PORT = int(PORT)

HDR_PART = 'Content-Disposition: form-data; name="fileField"; filename="big.bin"\r\nContent-Type: application/octet-stream\r\n\r\n'


def build(size, split_mode):
    """返回 (body, 源文件bytes, 切点下标 or None)
    字段集必须凑齐 /tfilepost 那页读的键（textfield、aa[]×9、textarea、fileField、submit），
    否则控制器在取 aa[8] 时抛异常，页面后半段的文件读数根本打不出来。"""
    src = bytes((i * 31 + (i // 4096)) % 256 for i in range(size))
    head = "--" + BD + "\r\nContent-Disposition: form-data; name=\"textfield\"\r\n\r\nOK-TEXT\r\n"
    for i in range(9):
        head += "--" + BD + "\r\nContent-Disposition: form-data; name=\"aa[]\"\r\n\r\naa-%d\r\n" % i
    head += ("--" + BD + "\r\nContent-Disposition: form-data; name=\"textarea\"\r\n\r\n"
             "PFX--" + BD[-6:] + "TAIL\r\n")
    head += "--" + BD + "\r\n" + HDR_PART
    tail = "\r\n--" + BD + "\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nok\r\n--" + BD + "--\r\n"
    body = head.encode() + src + tail.encode()
    cut = None
    if split_mode == "cr_lf":
        # 文件 part 收尾的 \r\n 里，\r 落在前一个包末尾（\n 是下一包首字节）
        cut = len(head) + len(src) + 1
    elif split_mode == "before_crlf":
        cut = len(head) + len(src)
    elif split_mode == "mid":
        cut = len(head) + size // 2
    return body, src, cut


def post(body, cut):
    req = ("POST /tfilepost HTTP/1.1\r\nHost: %s\r\nContent-Type: multipart/form-data; boundary=%s\r\n"
           "Content-Length: %d\r\nConnection: close\r\n\r\n") % (HOST, BD, len(body))
    s = socket.create_connection((HOST, PORT), timeout=30)
    s.sendall(req.encode() + body[:cut if cut is not None else len(body)])
    if cut is not None and cut < len(body):
        time.sleep(0.35)
        s.sendall(body[cut:])
    chunks = []
    while True:
        d = s.recv(65536)
        if not d:
            break
        chunks.append(d)
    s.close()
    return b"".join(chunks)


def judge(name, size, split_mode, src, resp):
    text = resp.decode("utf-8", errors="replace")
    m = re.search(r"fileField:size:(\d+)", text)
    got_size = int(m.group(1)) if m else -1
    t = re.search(r"fileField:tempfile:([^<]*)", text)
    tp = t.group(1).strip() if t else ""
    md5 = "no-tempfile"
    if tp and os.path.exists(tp):
        with open(tp, "rb") as f:
            md5 = hashlib.md5(f.read()).hexdigest()
    want_md5 = hashlib.md5(src).hexdigest()
    reds = []
    if "hello world" not in text:
        reds.append("no-200-page")
    if got_size != size:
        reds.append("size=%d want=%d (差 %+d)" % (got_size, size, got_size - size))
    if md5 != want_md5:
        reds.append("md5=%s want=%s" % (md5, want_md5))
    if "submit:ok" not in text:
        reds.append("submit 丢失")
    if "textfield:OK-TEXT" not in text:
        reds.append("textfield 丢失")
    print("%-28s bytes=%d got_size=%d md5_ok=%s %s" % (
        name, size, got_size, "Y" if md5 == want_md5 else "N",
        "PASS" if not reds else "FAIL " + "; ".join(reds)))
    return 0 if not reds else 1


def main():
    os.makedirs(RUN, exist_ok=True)
    total_red = 0
    cases = [
        ("A-small-40K-nosplit", 40000, None),
        ("B-1M-mid", 1024 * 1024, "mid"),
        ("C-3.6M-nosplit", 3_600_000, None),
        ("D-3.6M-mid", 3_600_000, "mid"),
        ("E-3.6M-cut-before-crlf", 3_600_000, "before_crlf"),
        ("F-3.6M-cut-cr|lf", 3_600_000, "cr_lf"),
        ("G-8M-cut-cr|lf", 8_000_000, "cr_lf"),
        ("H-8M-nosplit", 8_000_000, None),
        # 阈值对齐三臂：文件正好 3145728 字节时，field_item 累计到收尾的 \r 那一刻
        # 才跨过 >3145728（刷盘检查在每个包处理完之后做一次），于是 \r 和正文一起被写进文件、
        # field_item 清空；下一个包只剩以 \n 开头的分隔符，前缀 \r 已经看不见。
        # 3145730 那臂作为对照：跨过阈值发生在正文最后两字节，\r 留在 field_item 里。
        ("I-align3145728-cr|lf", 3145728, "cr_lf"),
        ("J-align3145728-nosplit", 3145728, None),
        ("K-align3145730-cr|lf", 3145730, "cr_lf"),
    ]
    print("TARGET=%s boundary=%s" % (TARGET, BD))
    for name, size, mode in cases:
        body, src, cut = build(size, mode)
        resp = post(body, cut)
        total_red += judge(name, size, mode, src, resp)
    print("ONLINE_BIG_SUMMARY red=%d verdict=%s" % (total_red, "PASS" if total_red == 0 else "FAIL"))
    sys.exit(0 if total_red == 0 else 1)


main()
