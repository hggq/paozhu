#!/usr/bin/env python3
# Land the "read_is_clean" write-back gate at the 4 text-path record write-back
# sites of each dialect template. Every site must match the exact expected shape;
# any file whose site count is not 4 exits non-zero without writing.
import sys

SNAPSHOT = [
    '                // 写回只认"这次读干净"：error_msg 与 isclose 是三个驱动共有的凭证，',
    '                // error_code 只有 mysql 和 pg 有，这段三个模板要逐字节一致所以不用它；',
    '                // 三个驱动都在语句入口清零 error_msg，所以不会残留成永不写回。',
    '                bool read_is_clean = select_conn->error_msg.empty() && !select_conn->isclose;',
]

BACK_PREFIX = "                    conn_obj->back_"
BACK_SUFFIX = "_select_conn(std::move(select_conn));"
OLD_GATE = "                if (iscache)"
NEW_GATE = "                if (iscache && read_is_clean)"
SAVE_CALL = "                    save_cache(exptime);"


def patch(path):
    with open(path, "r", encoding="utf-8") as fp:
        lines = fp.read().split("\n")

    sites = []
    for i, line in enumerate(lines):
        if line == SAVE_CALL:
            sites.append(i)

    out = list(lines)
    marks = []
    for i in sites:
        if out[i - 2] != OLD_GATE and out[i - 2] != NEW_GATE:
            sys.stderr.write("SHAPE_FAIL %s L%d: gate line is %r\n" % (path, i, out[i - 2]))
            return 2
        if out[i - 1] != "                {":
            sys.stderr.write("SHAPE_FAIL %s L%d: brace line is %r\n" % (path, i, out[i - 1]))
            return 2
        end = i - 3
        if out[end] == "":
            end -= 1
        if out[end] != "                }":
            sys.stderr.write("SHAPE_FAIL %s L%d: return-block close is %r\n" % (path, i, out[end]))
            return 2
        back = end - 1
        if not (out[back].startswith(BACK_PREFIX) and out[back].endswith(BACK_SUFFIX)):
            sys.stderr.write("SHAPE_FAIL %s L%d: back_conn line is %r\n" % (path, i, out[back]))
            return 2
        if out[back - 1] != "                {" or out[back - 2] != "                if (!islock_conn)":
            sys.stderr.write("SHAPE_FAIL %s L%d: islock_conn block is %r %r\n"
                             % (path, i, out[back - 2], out[back - 1]))
            return 2
        marks.append((back - 2, i, out[i - 2] == NEW_GATE))

    already = sum(1 for m in marks if m[2])
    if already == len(marks) and len(marks) == 4:
        sys.stderr.write("ALREADY_LANDED %s (%d sites)\n" % (path, len(marks)))
        return 3
    if len(marks) != 4 or already != 0:
        sys.stderr.write("SITE_COUNT_FAIL %s sites=%d already_gated=%d\n" % (path, len(marks), already))
        return 1

    for gate_at, save_at, _ in sorted(marks, reverse=True):
        out[save_at - 2] = NEW_GATE
        out[gate_at:gate_at] = SNAPSHOT

    with open(path, "w", encoding="utf-8") as fp:
        fp.write("\n".join(out))
    sys.stderr.write("PATCHED %s sites=%d lines_added=%d\n" % (path, len(marks), 4 * len(marks)))
    return 0


if __name__ == "__main__":
    rc = 0
    for p in sys.argv[1:]:
        r = patch(p)
        rc = rc or r
    sys.exit(rc)
