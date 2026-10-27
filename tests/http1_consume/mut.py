#!/usr/bin/env python3
# 牙齿臂生成器：把工作树里的 http_parse.cpp 复制成若干份"故意改坏"的源，供 run.sh 换链。
# 每份只改一处，改动点前后各带一条 MUT-<n> 锚点注释；生成时断言
#   锚点在被复制的产品源里不存在（否则说明上一轮的改动没复原）
#   目标片段在源里恰好命中一次（否则 sed 会改到别处）
# 用法: python3 mut.py <仓库根>     产物落 temp/h1_consume/mut_<n>_http_parse.cpp
import os
import sys

if len(sys.argv) != 2:
    sys.exit("USAGE mut.py <repo_root>")
root = sys.argv[1]
src = os.path.join(root, "vendor/httpserver/src/http_parse.cpp")
out = os.path.join(root, "temp/h1_consume")

# (臂名, 锚点注释, 原文片段, 改坏后的片段)
ARMS = [
    (
        "1",
        "// MUT-1 去掉 body 长度钳制：声明外的字节也被算进本次请求",
        "        else if ((peer->content_length - already) < avail)\n"
        "        {\n"
        "            avail = static_cast<unsigned int>(peer->content_length - already);\n"
        "        }\n",
        "        else if ((peer->content_length - already) < avail)\n"
        "        {\n"
        "            // MUT-1 去掉 body 长度钳制：声明外的字节也被算进本次请求\n"
        "        }\n",
    ),
    (
        "2",
        "// MUT-2 去掉零消费短路：没声明 body 的请求把同包残留当成正文吞掉",
        "    if (peer->content_length == 0 || avail == 0)\n",
        "    // MUT-2 去掉零消费短路：没声明 body 的请求把同包残留当成正文吞掉\n"
        "    if (avail == 0)\n",
    ),
    (
        "3",
        "// MUT-3 认错包尾续接字节：上一包残留的 \\r 配不上本包的 \\n，直接报 40001",
        "        if (readoffset < buffersize && buffer[readoffset] == '\\n')\n",
        "        if (readoffset < buffersize && buffer[readoffset] == '\\r')\n"
        "        // MUT-3 认错包尾续接字节：上一包残留的 \\r 配不上本包的 \\n，直接报 40001\n",
    ),
    (
        "4",
        "// MUT-4 default 分支退回 400：首字母不在表里的方法名不再是未实现，而是格式错误",
        "    default:\n        method_unknown = true;\n    }\n",
        "    default:\n"
        "        // MUT-4 default 分支退回 400：首字母不在表里的方法名不再是未实现，而是格式错误\n"
        "        error  = 40066;\n    }\n",
    ),
    (
        "5",
        "// MUT-5 认不出方法后继续往下解这一条消息：readoffset 跑到缓冲区末尾，isfinish 也被置真",
        "        if (error > 0 || method_unknown)\n",
        "        if (error > 0)  // MUT-5 认不出方法后继续往下解这一条消息\n",
    ),
]

data = open(src, encoding="utf-8").read()
if not os.path.isdir(out):
    sys.exit("NO_OUTDIR %s" % out)
if "MUT-" in data:
    sys.exit("MARKER_LEFT_IN_PRODUCT")

for n, anchor, old, new in ARMS:
    if anchor.strip() in data:
        sys.exit("MARKER_LEFT_IN_PRODUCT %s" % n)
    hits = data.count(old)
    if hits != 1:
        sys.exit("ARM_ANCHOR_COUNT %s got=%d want=1" % (n, hits))
    dst = os.path.join(out, "mut_%s_http_parse.cpp" % n)
    open(dst, "w", encoding="utf-8").write(data.replace(old, new))
    print("ARM_WRITTEN %s %s" % (n, dst))

# 探针侧的判别力臂：不是改产品，而是把"上限判定与自增的先后"颠倒，
# 用来证明组 13/14 那两条读数真的能分辨这两种口径（否则它们只是复述实现）。
probe = os.path.join(root, "tests/http1_consume/h1_consume_probe.cpp")
P_ARMS = [
    (
        "cap",
        "// MUT-CAP 自增挪到判定之前：上限口径从 N 条变成 N-1 条",
        "#ifndef BENCHMARK\n"
        "            if (single_link_count >= CONST_HTTP1_KEEPALIVE_MAX_REQUESTS)\n"
        "            {\n"
        "                closed_reason_ = \"request_cap\";\n"
        "                return;\n"
        "            }\n"
        "#endif\n"
        "            unsigned int consumed = r.consumed;\n"
        "            pre_->clear();\n"
        "            peer_->clear();\n"
        "            single_link_count++;\n",
        "#ifndef BENCHMARK\n"
        "            // MUT-CAP 自增挪到判定之前：上限口径从 N 条变成 N-1 条\n"
        "            single_link_count++;\n"
        "            if (single_link_count >= CONST_HTTP1_KEEPALIVE_MAX_REQUESTS)\n"
        "            {\n"
        "                closed_reason_ = \"request_cap\";\n"
        "                return;\n"
        "            }\n"
        "#endif\n"
        "            unsigned int consumed = r.consumed;\n"
        "            pre_->clear();\n"
        "            peer_->clear();\n",
    ),
]
pdata = open(probe, encoding="utf-8").read()
if "MUT-" in pdata:
    sys.exit("MARKER_LEFT_IN_PROBE")
for n, anchor, old, new in P_ARMS:
    hits = pdata.count(old)
    if hits != 1:
        sys.exit("PROBE_ARM_ANCHOR_COUNT %s got=%d want=1" % (n, hits))
    dst = os.path.join(out, "mut_probe_%s.cpp" % n)
    open(dst, "w", encoding="utf-8").write(pdata.replace(old, new))
    print("PROBE_ARM_WRITTEN %s %s" % (n, dst))
