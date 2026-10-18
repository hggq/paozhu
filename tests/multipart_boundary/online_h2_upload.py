#!/usr/bin/env python3
# h2 multipart 在线回归：用 bin/http2_multiclient 把夹具正文按不同 DATA 帧大小发出去，
# 读回 /tfilepost 的响应，逐字段判"发出去的和读回来的是不是同一份"。
# 用法: python3 online_h2_upload.py [host:port]     # 默认 127.0.0.1:18890
# 前置: 服务端已在跑（判据实例形状见仓库 temp/h2c_run，httpport=18890）；
#       客户端已编好（缺失时本脚本自己按 startup/http2_multiclient.cpp 头部那行编）。
# 帧大小列表是判据的一部分：h2 的 content 就是单个 DATA 帧正文，所以每个切点都是跨帧边界，
# 小帧值专门用来把分隔符、字段名切成跨帧，抓的是"跨帧丢字节/切错字段"这一类缺陷。
import hashlib
import ast
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
RUN = os.path.join(ROOT, "temp", "multipart_boundary_run")
CLIENT = os.path.join(ROOT, "bin", "http2_multiclient")
BODY = os.path.join(RUN, "h2_body.bin")
FRAME_SIZES = [16384, 997, 64, 7]
PATH = "/tfilepost"
# 与客户端 body_dump_cap 同源；响应体超过它就只判得见前缀，缺字段记"读数不足"而不是红。
DUMP_CAP = 131072

os.makedirs(RUN, exist_ok=True)


def sh(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def fixture():
    r = sh([sys.executable, os.path.join(HERE, "mp_fixture.py"), BODY], cwd=ROOT)
    if r.returncode != 0:
        print("FIXTURE_FAIL rc=%d %s" % (r.returncode, r.stderr[-400:]))
        sys.exit(2)
    kv = {}
    for line in r.stdout.splitlines():
        if "=" in line:
            k, v = line.split("=", 1)
            kv[k] = v
    print("FIXTURE bytes=%s md5=%s" % (kv["BODY_BYTES"], kv["EXPECT_file_md5"]))
    return kv


def stamp(p):
    return time.strftime("%m-%d %H:%M:%S", time.localtime(os.path.getmtime(p)))


def build_client():
    src = os.path.join(ROOT, "startup", "http2_multiclient.cpp")
    if os.path.exists(CLIENT) and os.path.getmtime(src) <= os.path.getmtime(CLIENT):
        print("CLIENT_USE %s mtime=%s" % (CLIENT, stamp(CLIENT)))
        return True
    r = sh(["/usr/bin/c++", "-std=c++20", "-O0", "-g", "-Wall", "-DASIO_STANDALONE",
            "-Iasio", "startup/http2_multiclient.cpp", "-o", "bin/http2_multiclient", "-pthread"], cwd=ROOT)
    if r.returncode != 0:
        print("CLIENT_BUILD_FAIL\n" + r.stderr[-1500:])
        return False
    print("CLIENT_REBUILT %s mtime=%s" % (CLIENT, stamp(CLIENT)))
    return True


def run_case(base, ctype, fs):
    cmd = [CLIENT, "--host", base.split(":")[0], "--port", base.split(":")[1],
           "--path", PATH, "--authority", base, "--method", "POST",
           "--content-type", ctype, "--body-file", BODY,
           "--frame-size", str(fs), "--dump-body", "1",
           "--conns", "1", "--streams", "1", "--inflight", "1",
           "--deadline", "60000", "--settings-timeout", "8000"]
    r = subprocess.run(cmd, capture_output=True, cwd=ROOT, timeout=300)
    # 响应体里可能夹着非 UTF-8 字节（正文被误当字段值回显时就会），
    # 判据只用 ASCII 标记，所以按替换解码读，不让解码失败冒充成红。
    return r.returncode, r.stdout.decode("utf-8", errors="replace")


def tempfile_md5(body_text):
    m = re.search(r"fileField:tempfile:([^<]*)</p>", body_text)
    if not m:
        return None, "no-tempfile-marker"
    p = m.group(1).strip()
    if not p or not os.path.exists(p):
        return None, "tempfile-recycled:" + p
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest(), p


def esc(v):
    # 客户端打 BODY-DUMP 时把非可打印字节转义成 \xHH，所以按原始值拼的标记要先做同样转义。
    return "".join(c if 0x20 <= ord(c) < 0x7F else "\\x%02x" % ord(c) for c in v)


def judge(fs, rc, out, exp):
    reads = []
    reds = []
    dump = None
    truncated = False
    for line in out.splitlines():
        if line.startswith("BODY-DUMP "):
            i = line.find(" body=")
            dump = line[i + 6:] if i >= 0 else ""
            nbytes = int(re.search(r" bytes=(\d+)", line).group(1))
            total = int(re.search(r" total=(\d+)", line).group(1))
            reads.append("dump bytes=%d total=%d" % (nbytes, total))
            truncated = nbytes >= DUMP_CAP
            if truncated:
                reads.append("TRUNCATED:响应体截到 %d 字节，缺字段属读数不足不是红" % DUMP_CAP)
    if "VERDICT PASS" not in out:
        reds.append("client 未判干净(VERDICT) rc=%d" % rc)
    if "status200=1" not in out:
        reds.append("no-200")
    if dump is None:
        reds.append("no-body-dump")
        return reads, reds
    checks = [
        ("textfield", "textfield:" + esc(ast.literal_eval(exp["EXPECT_textfield"])) + "</p>"),
        ("aa0", "aa[]:" + esc(ast.literal_eval(exp["EXPECT_aa0"])) + "</p>"),
        ("aa8", "aa[8]:" + esc(ast.literal_eval(exp["EXPECT_aa8"])) + "</p>"),
        ("submit", "submit:" + esc(ast.literal_eval(exp["EXPECT_submit"])) + "</p>"),
        ("textarea", "textarea:" + esc(ast.literal_eval(exp["EXPECT_textarea"])) + "</p>"),
        ("decoy-json", 'A\\nB--' + exp["BOUNDARY"] + 'C\\rD'),
        ("file-name", "fileField:filename:" + esc(ast.literal_eval(exp["EXPECT_filename"])) + "</p>"),
        ("file-size", "fileField:size:" + exp["EXPECT_fileField_size"] + "</p>"),
    ]
    for name, needle in checks:
        if needle in dump:
            reads.append("ok %s" % name)
        elif truncated:
            reads.append("inconclusive %s" % name)
        else:
            reds.append("field-mismatch %s" % name)
    md5, info = tempfile_md5(dump)
    if md5 is None:
        reads.append("file-md5 判不了(%s)" % info)
    elif md5 != exp["EXPECT_file_md5"]:
        reds.append("file-md5 %s != %s" % (md5, exp["EXPECT_file_md5"]))
    else:
        reads.append("ok file-md5")
    return reads, reds


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:18890"
    if not build_client():
        sys.exit(2)
    exp = fixture()
    ctype = exp["CONTENT_TYPE"]
    total_red = 0
    for fs in FRAME_SIZES:
        try:
            rc, out = run_case(base, ctype, fs)
        except subprocess.TimeoutExpired:
            print("FRAME=%d TIMEOUT rc=-" % fs)
            total_red += 1
            continue
        reads, reds = judge(fs, rc, out, exp)
        total_red += len(reds)
        print("FRAME=%d rc=%d %s" % (fs, rc, " | ".join(reads)))
        for t in reds:
            print("  RED %s" % t)
        with open(os.path.join(RUN, "online_%d.log" % fs), "w") as f:
            f.write(out)
    print("ONLINE_SUMMARY target=%s path=%s frames=%s red=%d verdict=%s" % (
        base, PATH, FRAME_SIZES, total_red, "PASS" if total_red == 0 else "FAIL"))
    sys.exit(0 if total_red == 0 else 1)


main()
