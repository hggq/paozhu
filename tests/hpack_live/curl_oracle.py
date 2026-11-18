import socket, subprocess, threading, sys
srv = socket.socket(); srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", 18999)); srv.listen(1); srv.settimeout(8)
def run():
    subprocess.run(["curl","--http2-prior-knowledge","-s","--max-time","3",
                    "-H","x-test: custom-key","-H","x-two: custom-header",
                    "-H","x-three: www.example.com","http://127.0.0.1:18999/"],
                   capture_output=True)
threading.Thread(target=run, daemon=True).start()
conn,_ = srv.accept(); conn.settimeout(5)
data = b""
try:
    while True:
        chunk = conn.recv(65536)
        if not chunk: break
        data += chunk
        if len(data) > 4096: break
except socket.timeout:
    pass
conn.close(); srv.close()
preface = b"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
assert data.startswith(preface), data[:30]
p = len(preface)
while p + 9 <= len(data):
    ln = int.from_bytes(data[p:p+3],"big"); typ = data[p+3]; flags=data[p+4]
    sid = int.from_bytes(data[p+5:p+9],"big") & 0x7FFFFFFF
    payload = data[p+9:p+9+ln]
    if typ == 1:
        print("HEADERS stream", sid, "flags", hex(flags), "len", ln)
        print(" ".join(f"{b:02x}" for b in payload))
    p += 9 + ln
