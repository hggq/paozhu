#!/usr/bin/env python3
"""
Multipart file part upload probe (2026-11-07)

One tiny multipart POST to /tfilepost carrying a 16-byte file part. Two arms,
each with its own fixed expectation:

  normal   temppath is writable.
           PASS = HTTP 200, fileField:size == 16, and the tempfile really is
                  on disk with 16 bytes and the matching md5.

  no-temp  temppath is NOT writable (missing dir / read-only fs), so the
           parser cannot open the upload tempfile.
           PASS = HTTP 400 carrying "Error Code: 40226", and no fileField
                  entry in the body at all — the request never reaches
                  business.

The normal arm is the positive lower bound: it only stays green while
writable uploads still work, so it catches a refusal that fires too wide.

Pre-registered exit codes: 0 = PASS, 1 = FAIL, 2 = no response reached us
(connection reset before the status line) or a bad command line.

USAGE:
  python3 mp_size0_repro.py [host:port] [normal|no-temp]
  default: 127.0.0.1:80 normal

  no-temp needs its own instance. Do not edit conf/server.conf in place — the
  repo copy is the live one; run a second paozhu from a scratch working
  directory with a copied conf whose temppath points somewhere unwritable.

HISTORY:
  commit edde414dd dropped the else branch after the fopen check in
  post_multipart_itemcontent() — a failed fopen still published a
  fileField:size:0 with a tempfile path that was never created. The 40226
  branch is what the no-temp arm now asserts.
"""
import sys, os, hashlib, re, urllib.request, urllib.error

WANT_SIZE = 16          # fixed bound, not measured from the fixture below
FIXED_CODE = 40226      # h1 fopen-failure code on the multipart upload path
FILE_CONTENT = b"SMALL-DATA-12345"
assert len(FILE_CONTENT) == WANT_SIZE, "fixture drifted away from WANT_SIZE"
WANT_MD5 = hashlib.md5(FILE_CONTENT).hexdigest()

# -------- helpers --------
def build_multipart(boundary: bytes, file_content: bytes) -> bytes:
    """Build a minimal multipart body: textfield + one file part + submit."""
    CRLF = b"\r\n"
    bd = b"--" + boundary
    parts = []
    # textfield (non-file, baseline sanity)
    parts += [bd, b'Content-Disposition: form-data; name="textfield"',
              b"", b"OK-TEXT"]
    # file part (THE one that should write to temp)
    parts += [bd,
              b'Content-Disposition: form-data; name="fileField"; filename="s.bin"',
              b"Content-Type: application/octet-stream",
              b"", file_content]
    # submit
    parts += [bd, b'Content-Disposition: form-data; name="submit"',
              b"", b"ok"]
    parts += [bd + b"--", b""]
    return CRLF.join(parts)

def http_post(url: str, body: bytes, boundary: bytes, timeout=10):
    """Return (status, body_text); a 4xx response is a reading, not a send failure."""
    req = urllib.request.Request(url, data=body, method="POST")
    req.add_header("Content-Type", f"multipart/form-data; boundary={boundary.decode()}")
    req.add_header("Content-Length", str(len(body)))
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return resp.status, resp.read().decode("utf-8", errors="replace")
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode("utf-8", errors="replace")


def grab(resp: str, tag: str):
    """Read "<tag>: <value>" from the page; the value stops at '<' or whitespace."""
    m = re.search(rf'{re.escape(tag)}:\s*([^<\s]+)', resp)
    return m.group(1) if m else None


def resolve_on_disk(tempfile_path: str):
    """Return (path, size, md5) for the reported tempfile; size None when missing."""
    abs_path = tempfile_path
    if abs_path.startswith("./"):
        abs_path = os.path.abspath(abs_path)
    elif not os.path.isabs(abs_path):
        for base in (os.getcwd(), "/Users/hzq/paozhu"):
            cand = os.path.join(base, tempfile_path)
            if os.path.isfile(cand):
                abs_path = cand
                break
    if not os.path.isfile(abs_path):
        return abs_path, None, None
    with open(abs_path, "rb") as f:
        data = f.read()
    return abs_path, len(data), hashlib.md5(data).hexdigest()

# -------- arms --------
def arm_normal(status: int, resp: str) -> bool:
    size_str = grab(resp, "fileField:size")
    got_size = int(size_str) if size_str else None
    tempfile_path = grab(resp, "fileField:tempfile")

    print(f"[NORMAL]  HTTP              {status}    want 200")
    print(f"[NORMAL]  fileField:size    {got_size}    want {WANT_SIZE}")
    print(f"[NORMAL]  fileField:error   {grab(resp, 'fileField:error')}"
          "    (hardcoded 0 on every path, reading only)")

    disk_size = disk_md5 = None
    if tempfile_path:
        disk_path, disk_size, disk_md5 = resolve_on_disk(tempfile_path)
        print(f"[NORMAL]  tempfile on disk   {disk_path} size={disk_size} md5={disk_md5}")
    else:
        print("[NORMAL]  tempfile on disk   not reported by the page")

    ok = (status == 200 and got_size == WANT_SIZE and disk_size == WANT_SIZE
          and disk_md5 == WANT_MD5)
    print()
    if ok:
        print("[NORMAL]  VERDICT: PASS — writable upload still lands 16 bytes on disk")
        return True
    print("[NORMAL]  VERDICT: FAIL — 200 / size / disk size / md5 must all hold")
    if status == 400:
        print("[NORMAL]  400 on a writable temppath means the new refusal fires too wide"
              f" (Error Code: {grab(resp, 'Error Code')})")
    return False


def arm_notemp(status: int, resp: str) -> bool:
    size_str = grab(resp, "fileField:size")
    got_size = int(size_str) if size_str else None
    code_str = grab(resp, "Error Code")
    got_code = int(code_str) if code_str else None
    tempfile_path = grab(resp, "fileField:tempfile")

    print(f"[NO-TEMP] HTTP              {status}    want 400")
    print(f"[NO-TEMP] Error Code        {got_code}    want {FIXED_CODE}")
    print(f"[NO-TEMP] fileField:size    {got_size}    want absent")
    print(f"[NO-TEMP] fileField:tempfile {tempfile_path}    want absent")
    print()

    if status != 400:
        print("[NO-TEMP] VERDICT: FAIL — expected 400")
        if status == 200 and got_size == 0:
            print(f"[NO-TEMP] 200 + size=0 is the pre-{FIXED_CODE} behaviour: fopen failed,")
            print("[NO-TEMP] the entry was published anyway and business never learned.")
        elif status == 200 and got_size == WANT_SIZE:
            print("[NO-TEMP] the upload succeeded, so this instance CAN write its temppath —")
            print("[NO-TEMP] wrong target or conf; this arm needs an unwritable temppath.")
        return False
    if got_code != FIXED_CODE:
        print(f"[NO-TEMP] VERDICT: FAIL — 400 came from code {got_code}, not {FIXED_CODE}")
        print("[NO-TEMP] another parse error fired; this arm proves nothing about fopen.")
        return False
    if got_size is not None or tempfile_path is not None:
        print("[NO-TEMP] VERDICT: FAIL — reported 40226 but still published a fileField entry")
        return False
    print("[NO-TEMP] VERDICT: PASS — fopen failure refused the request, no bogus entry")
    return True


# -------- main --------
def main():
    target = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1:80"
    mode = sys.argv[2] if len(sys.argv) > 2 else "normal"
    if mode not in ("normal", "no-temp"):
        print(f"[REPRO] unknown mode {mode!r}; expected normal | no-temp")
        sys.exit(2)

    url = f"http://{target}/tfilepost"
    boundary = b"PZ-SIZE0-REPRO-" + str(os.getpid()).encode()
    body = build_multipart(boundary, FILE_CONTENT)

    print(f"[REPRO] mode            = {mode}")
    print(f"[REPRO] target          = {url}")
    print(f"[REPRO] file part bytes = {WANT_SIZE}")
    print(f"[REPRO] file part md5   = {WANT_MD5}")
    print()

    try:
        status, resp = http_post(url, body, boundary)
    except Exception as e:
        print(f"[REPRO] SEND FAIL: {e}")
        sys.exit(2)

    ok = arm_normal(status, resp) if mode == "normal" else arm_notemp(status, resp)
    sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
