#!/usr/bin/env python3
# HTTP/2 在线大文件核对：用 bin/http2_multiclient 把 2,097,152 字节的文件正文按 DATA 帧发出去，
# 帧大小对齐到"文件收尾的 \r 是某帧最后一字节、\n 是下一帧首字节"这一刀。
# 判据取自页面回显 + 服务端落盘的真实临时文件（temppath 绝对，直接读盘算 md5）。
# 用法: online_h2_big.py [host:port]      # 默认 127.0.0.1:80
import hashlib
import os
import re
import subprocess
import sys

ROOT = "/Users/hzq/paozhu"
CLIENT = os.path.join(ROOT, "bin", "http2_multiclient")
RUN = os.path.join(ROOT, "temp", "multipart_boundary_run", "online")
TARGET = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:80"
HOST, PORT = TARGET.split(":")
BD = "PZH2BIG"
THRESH = 2097152          # http2_parse.cpp 刷盘阈值（>2097152，且非末帧才刷）


def head_len_prefix(framelen, pad_len):
    """textfield + aa[]x9 固定，textarea 用 pad 调到想要的 head 长度"""
    h = "--%s\r\nContent-Disposition: form-data; name=\"textfield\"\r\n\r\nOK-TEXT\r\n" % BD
    for i in range(9):
        h += "--%s\r\nContent-Disposition: form-data; name=\"aa[]\"\r\n\r\naa-%d\r\n" % (BD, i)
    h += ("--%s\r\nContent-Disposition: form-data; name=\"textarea\"\r\n\r\n%s%s\r\n"
          % (BD, "T" * pad_len, ""))
    h += ("--%s\r\nContent-Disposition: form-data; name=\"fileField\"; filename=\"h2big.bin\"\r\n"
          "Content-Type: application/octet-stream\r\n\r\n" % BD)
    return h


def build_aligned(filelen, framelen):
    """返回 (body, src, 说明)：让 (S+filelen+1) % framelen == 0，即 \r 结尾、\n 起头"""
    for pad in range(0, framelen):
        head = head_len_prefix(framelen, pad)
        if (len(head) + filelen + 1) % framelen == 0:
            src = bytes((i * 31 + (i // 4096)) % 256 for i in range(filelen))
            tail = ("\r\n--%s\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nok\r\n"
                    "--%s--\r\n" % (BD, BD))
            return (head.encode() + src + tail.encode(), src, "pad=%d S=%d seam=%d" % (
                pad, len(head), len(head) + filelen + 1))
    raise RuntimeError("align 失败 framelen=%d" % framelen)


def build_plain(filelen):
    head = head_len_prefix(0, 3)
    src = bytes((i * 31 + (i // 4096)) % 256 for i in range(filelen))
    tail = ("\r\n--%s\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nok\r\n"
            "--%s--\r\n" % (BD, BD))
    return head.encode() + src + tail.encode(), src, "plain"


def run_client(body_path, ctype, framelen):
    path = os.path.join(RUN, "h2_bigbody.bin")
    with open(path, "wb") as f:
        f.write(body_path)
    cmd = [CLIENT, "--host", HOST, "--port", PORT, "--path", "/tfilepost", "--authority", TARGET,
           "--method", "POST", "--content-type", ctype, "--body-file", path,
           "--frame-size", str(framelen), "--dump-body", "1",
           "--conns", "1", "--streams", "1", "--inflight", "1",
           "--deadline", "120000", "--settings-timeout", "8000"]
    r = subprocess.run(cmd, capture_output=True, cwd=ROOT, timeout=600)
    return r.returncode, r.stdout.decode("utf-8", errors="replace")


def judge(name, rc, out, src, note):
    text = out
    sz = re.search(r"fileField:size:(\d+)", text)
    tp = re.search(r"fileField:tempfile:([^<\\\s]+)", text)
    got_size = int(sz.group(1)) if sz else -1
    reds = []
    if "status200=1" not in text:
        reds.append("no-200/无完成读数 rc=%d" % rc)
    if got_size != len(src):
        reds.append("size=%d want=%d 差%+d" % (got_size, len(src), got_size - len(src)))
    if "submit:ok" not in text:
        reds.append("submit 丢失")
    if "textfield:OK-TEXT" not in text:
        reds.append("textfield 丢失")
    md5state = "no-tempfile-path"
    if tp:
        # 判据实例的 temppath 是绝对路径，页面直接回显全路径
        found = tp.group(1)
        if not os.path.isabs(found):
            found = os.path.join(ROOT, found)
        if os.path.exists(found):
            with open(found, "rb") as f:
                data = f.read()
            same = hashlib.md5(data).hexdigest() == hashlib.md5(src).hexdigest()
            md5state = "盘上%d字节 md5=%s" % (len(data), "OK" if same else "不符")
            if not same:
                reds.append("落盘正文与发出的不一致（%s）" % md5state)
        else:
            md5state = "tempfile 不存在:%s" % found
            reds.append(md5state)
    print("%-30s %s | %s | %s" % (name, note, ("RED " + "; ".join(reds)) if reds else "ok", md5state))
    return len(reds)


def main():
    os.makedirs(RUN, exist_ok=True)
    ctype = "multipart/form-data; boundary=" + BD
    total = 0
    # 1) 小文件（远不跨阈值）——普通提交数据的正向核对
    body, src, note = build_plain(40000)
    rc, out = run_client(body, ctype, 16384)
    total += judge("A-small-40K", rc, out, src, note)
    # 2) 大文件、任意帧大小（16384）——跨阈值刷盘但不把缝对齐到收尾 CRLF
    body, src, note = build_plain(2621440)
    rc, out = run_client(body, ctype, 16384)
    total += judge("B-2.5M-frame16384", rc, out, src, "跨阈值/缝不对齐")
    # 3) 大文件、帧缝正好落在收尾的 \r 与 \n 之间（离线判据 A 臂红了的那一刀）
    for F in (4096, 8192):
        body, src, note = build_aligned(THRESH, F)
        rc, out = run_client(body, ctype, F)
        total += judge("C-align2M-seam-cr|lf-f%d" % F, rc, out, src, note + " frame=%d" % F)
    # 4) 同一对齐尺寸但不切缝（frame=16384，缝不在 \r\n 之间）作为对照
    body, src, note = build_plain(THRESH)
    rc, out = run_client(body, ctype, 16384)
    total += judge("E-alignsize-frame16384", rc, out, src, "同尺寸/不对齐缝")
    print("H2_BIG_SUMMARY target=%s red=%d verdict=%s" % (TARGET, total, "PASS" if total == 0 else "FAIL"))
    sys.exit(0 if total == 0 else 1)


main()
