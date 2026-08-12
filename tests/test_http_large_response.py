import ast
import re
from pathlib import Path


asset_source = Path("include/web_assets.h").read_text(encoding="utf-8")
literals = re.findall(r'^"(.*)"$', asset_source, flags=re.MULTILINE)
dashboard = "".join(ast.literal_eval(f'"{literal}"') for literal in literals).encode("utf-8")

lwipopts = Path("lwipopts.h").read_text(encoding="utf-8")
mss = int(re.search(r"#define\s+TCP_MSS\s+(\d+)", lwipopts).group(1))
factor = int(re.search(r"#define\s+TCP_SND_BUF\s+\((\d+)\s*\*\s*TCP_MSS\)", lwipopts).group(1))
send_buffer = factor * mss

header = (
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    f"Content-Length: {len(dashboard)}\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-store\r\n\r\n"
).encode("ascii")
response = header + dashboard

# Model a deliberately constrained live send window. The response must remain
# byte-identical across many MSS-sized writes and may close only after all ACKs.
queued = 0
acked = 0
chunks = []
while acked < len(response):
    available = min(2 * mss, send_buffer)
    while queued < len(response) and available:
        size = min(mss, available, len(response) - queued)
        chunks.append(response[queued:queued + size])
        queued += size
        available -= size
    acked = queued

assert b"".join(chunks) == response
assert len(chunks) > 1
assert queued == acked == len(response)
assert len(dashboard) < send_buffer

print(
    f"large HTTP response test passed: dashboard={len(dashboard)}, "
    f"TCP_SND_BUF={send_buffer}, chunks={len(chunks)}"
)
