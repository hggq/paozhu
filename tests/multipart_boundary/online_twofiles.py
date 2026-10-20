#!/usr/bin/env python3
# 在线核对：一个表单里两个文件 part（普通大小，不跨刷盘阈值）。
# 判据：页面打出的 fileField:tempfile 里必须是第一个 part 的正文（md5 对得上），
#       且 fileField:size 等于第一个 part 的字节数。
# 用法: online_twofiles.py [host:port]
import hashlib
import os
import re
import socket
import sys

TARGET = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:80"
HOST, PORT = TARGET.split(":")
BD = "PZTWOF"
SRV_CWD = "/Users/hzq/paozhu"


def build():
    f1 = bytes((i * 7 + 1) % 256 for i in range(15))
    f2 = bytes((i * 3 + 99) % 256 for i in range(200))

    def part(name, fn, data):
        return ("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
                "Content-Type: application/octet-stream\r\n\r\n" % (BD, name, fn)).encode() + data + b"\r\n"

    body = (b"--" + BD.encode() +
            b"\r\nContent-Disposition: form-data; name=\"textfield\"\r\n\r\nOK-TEXT\r\n")
    for i in range(9):
        body += ("--%s\r\nContent-Disposition: form-data; name=\"aa[]\"\r\n\r\naa-%d\r\n" % (BD, i)).encode()
    body += ("--%s\r\nContent-Disposition: form-data; name=\"textarea\"\r\n\r\nPFX\r\n" % BD).encode()
    body += part("fileField", "a.bin", f1) + part("fileField2", "b.bin", f2)
    body += ("--%s\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nok\r\n--%s--\r\n" % (BD, BD)).encode()
    return body, f1, f2


def post(body):
    req = ("POST /tfilepost HTTP/1.1\r\nHost: %s\r\nContent-Type: multipart/form-data; boundary=%s\r\n"
           "Content-Length: %d\r\nConnection: close\r\n\r\n") % (HOST, BD, len(body))
    s = socket.create_connection((HOST, int(PORT)), timeout=20)
    s.sendall(req.encode() + body)
    out = []
    while True:
        d = s.recv(65536)
        if not d:
            break
        out.append(d)
    s.close()
    return b"".join(out).decode("utf-8", errors="replace")


def main():
    body, f1, f2 = build()
    red = 0
    for k in range(3):
        text = post(body)
        m = re.search(r"fileField:tempfile:([^<\s]+)", text)
        sz = re.search(r"fileField:size:(\d+)", text)
        got_size = int(sz.group(1)) if sz else -1
        if not m:
            print("RUN%d RED 页面上没有 fileField:tempfile" % k)
            red += 1
            continue
        path = m.group(1)
        if not os.path.isabs(path):
            path = os.path.join(SRV_CWD, path)
        if not os.path.exists(path):
            print("RUN%d RED tempfile 不存在 %s" % (k, path))
            red += 1
            continue
        with open(path, "rb") as f:
            data = f.read()
        ok_size = (got_size == len(f1))
        ok_first = (data == f1)
        note = ""
        if not ok_first:
            note = "文件正文成了第二个 part（%d 字节, md5=%s）→ 两个 part 撞在同一个临时文件名" % (
                len(data), hashlib.md5(data).hexdigest()[:8])
        print("RUN%d fileField:size=%d want=%d | 盘上 %d 字节 | 是第一个 part 正文=%s %s" % (
            k, got_size, len(f1), len(data), "Y" if ok_first else "N", note))
        if not (ok_size and ok_first):
            red += 1
    print("TWOFILES_SUMMARY target=%s red=%d verdict=%s" % (TARGET, red, "PASS" if red == 0 else "FAIL"))
    sys.exit(0 if red == 0 else 1)


main()
