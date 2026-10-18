#!/usr/bin/env python3
# 从工作树里已移植完成的 vendor/httpserver/src/http2_parse.cpp 出发，每次只关掉一条移植，
# 生成突变臂底本（写到 temp/multipart_boundary_run/h2mut_<arm>.cpp）。
# 每条替换都断言命中次数——少一次就说明锚点漂了，那条臂的读数不能归因到这条移植。
import sys

ROOT = "/Users/hzq/paozhu"
SRC = ROOT + "/vendor/httpserver/src/http2_parse.cpp"
RUN = ROOT + "/temp/multipart_boundary_run"

PREV_TAIL = """        if (temp_post_data.field_item.size() < back)
        {
            return -1;
"""

RUNOUT_APPEND = """                temp_post_data.pre_content.append(
                    temp_post_data.content.substr(temp_post_data.field_offset, pos_m - temp_post_data.field_offset));
"""

ARMS = {
    # 主扫描的 "-- 前面必须是 CRLF" 判定关掉（保留剥字节的门）
    "mu1": [("if (dash_lf == 0)", "if (dash_lf == 99)", 1)],
    # 主扫描剥掉两个字节改成无条件剥（保留判定）
    "mu2": [("if (dash_lf == 1 && dash_cr == 1 && temp_post_data.field_item.size() > 1)",
             "if (temp_post_data.field_item.size() > 1)", 1)],
    # 三条跨帧支路的 CRLF 判定关掉（保留剥字节的门）
    "mu3": [("if (!isnotmatch && tail_is(1, '\\n') == 0)", "if (!isnotmatch && tail_is(1, '\\n') == 99)", 3)],
    # 三条跨帧支路剥字节改成无条件剥（保留判定）
    "mu4": [("if (tail_is(1, '\\n') == 1 && tail_is(2, '\\r') == 1 && temp_post_data.field_item.size() > 1)",
             "if (temp_post_data.field_item.size() > 1)", 3)],
    # 帧尾停在边界文本中间时，不再把已对上的字节留在 pre_content（字节被丢掉）
    "mu5": [(RUNOUT_APPEND, "                temp_post_data.pre_content.append(\n"
                            "                    temp_post_data.content.substr(temp_post_data.field_offset, 0));\n", 3)],
    # 嗅探分支不再要求 boundary 非空（没有 boundary 也按 multipart 解）
    "mu6": [("if (!temp_post_data.boundary.empty())", "if (true)", 1)],
    # case 2 的"无 boundary 硬失败"闸门关掉
    "mu7": [("if (temp_post_data.boundary.empty())\n            {\n                error = 40099;",
             "if (temp_post_data.boundary.size() == 9999)\n            {\n                error = 40099;", 1)],
    # 三态取不到（-1）改成"明确不是换行"（0）：跨帧时把真边界判成数据
    "mu8": [(PREV_TAIL, PREV_TAIL.replace("return -1;", "return 0;"), 2)],
}

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("USAGE h2mut.py <arm>")
        sys.exit(2)
    arm = sys.argv[1]
    if arm not in ARMS:
        print("UNKNOWN_ARM " + arm)
        sys.exit(2)
    text = open(SRC, "r", encoding="utf-8").read()
    for old, new, want in ARMS[arm]:
        got = text.count(old)
        if got != want:
            print("ANCHOR_COUNT arm=%s want=%d got=%d old=%r" % (arm, want, got, old[:60]))
            sys.exit(3)
        text = text.replace(old, new)
    out = RUN + "/h2mut_%s.cpp" % arm
    open(out, "w", encoding="utf-8").write(text)
    print("MUT_OK %s -> %s" % (arm, out))
