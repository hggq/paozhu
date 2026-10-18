#!/usr/bin/env python3
# multipart 正文夹具：把这份 body 用 h2 POST 发出去，字段值原样应能从响应里读回。
# 用法: python3 mp_fixture.py <输出文件>
# 打印期望读数（KEY=VALUE），在线判据脚本报据此比对。
#
# 夹具里埋的靶：
#   1) 值里带裸 \n（没有 \r）和单独的 \r，看边界判定会不会把值截断；
#   2) 值里带 "--BOUND" 字面量但前面不是 CRLF，这是数据不是分隔符，naive 解析会在这里切错；
#   3) 值里带 "--" + boundary 的后半段，考察"部分匹配"会不会被当成边界；
#   4) 40000 字节的文件正文，配合小 --frame-size 让 DATA 帧边界落在正文中间（h2 的 content 就是单帧正文）。
#
# 字段名/个数照 /tfilepost 那页读的键来配（textfield、aa[]×9、textarea、fileField 带 filename、submit），
# 否则控制器在取 aa[8] 时抛异常，页面后半段的读数根本打不出来。
import hashlib
import sys

BD = "PZMPBOUNDARY7"
OUT = sys.argv[1] if len(sys.argv) > 1 else "./temp/mp_h2_body.bin"

TEXTFIELD = "OK-TEXT"
# 值里既有裸 LF 也有裸 CR，还有一段 "--BOUND" 前面不是 CRLF
DECOY = "A\nB--" + BD + "C\rD"
TEXTAREA = "PFX--" + BD[-6:] + "TAIL" + "\n" + "Q\r\nR"
FILENAME = "pzprobe.bin"
FILEBYTES = bytes((1 + (i * 7 + (i // 4093) * 11)) % 256 for i in range(40000))
SUBMIT = "ok"
AA = ["aa-%d" % i for i in range(9)]


def part(name, extra_headers, body, filename=None):
    h = "--" + BD + "\r\n"
    if filename is None:
        h += 'Content-Disposition: form-data; name="' + name + '"\r\n'
    else:
        h += 'Content-Disposition: form-data; name="' + name + '"; filename="' + filename + '"\r\n'
    for k, v in extra_headers:
        h += k + ": " + v + "\r\n"
    h += "\r\n"
    return h.encode() + body + b"\r\n"


body = bytearray()
# 首字节是 '-'，h2 侧靠这个嗅出 multipart；boundary 从 Content-Type 头取
body += part("textfield", [], TEXTFIELD.encode())
for v in AA:
    body += part("aa[]", [], v.encode())
body += part("decoy", [], DECOY.encode())
body += part("fileField", [("Content-Type", "application/octet-stream")], FILEBYTES, filename=FILENAME)
body += part("textarea", [], TEXTAREA.encode())
body += part("submit", [], SUBMIT.encode())
body += ("--" + BD + "--\r\n").encode()

with open(OUT, "wb") as f:
    f.write(bytes(body))

print("BODY_FILE=" + OUT)
print("BODY_BYTES=%d" % len(body))
print("BOUNDARY=" + BD)
print("CONTENT_TYPE=multipart/form-data; boundary=" + BD)
print("EXPECT_textfield=%r" % TEXTFIELD)
print("EXPECT_decoy=%r" % DECOY)
print("EXPECT_textarea=%r" % TEXTAREA)
print("EXPECT_submit=%r" % SUBMIT)
print("EXPECT_aa0=%r" % AA[0])
print("EXPECT_aa8=%r" % AA[8])
print("EXPECT_filename=%r" % FILENAME)
print("EXPECT_fileField_size=%d" % len(FILEBYTES))
print("EXPECT_file_md5=%s" % hashlib.md5(FILEBYTES).hexdigest())
