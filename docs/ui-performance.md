# UI performance and embedded budget

Measurements were taken from the generated files and the final Pico W Release build on
13 August 2026. `tools/build_ui.py --check` is the source of the reproducible asset figures.

## Budgets and result

| Metric | Budget | Final result |
|---|---:|---:|
| Embedded single-file HTML | ≤ 128 KiB | 90,941 B |
| Local preview | ≤ 160 KiB | 103,503 B |
| Production CSS | ≤ 48 KiB | 37,992 B |
| Production JavaScript | ≤ 48 KiB | 38,159 B |
| Theoretical gzip size | reporting only | 24,805 B |
| External libraries | 0 B preferred | 0 B |
| Additional asset requests | 0 | 0 |
| Thermal-field draw calls | 1 / frame | 1 / frame |
| Thermal-field frame rate | ≤ 30 FPS | capped at 30 FPS |
| Browser history horizon | 30 min | 30 min |

The Pico server intentionally sends the 90,941-byte page uncompressed. At a conservative
1 Mbit/s payload rate its raw transfer component is about 0.73 s; normal local hotspot rates
are considerably faster. The existing asynchronous raw-lwIP sender splits the current page
into 61 writes bounded by `tcp_sndbuf()` while `TCP_SND_BUF` remains 29,200 bytes. No new
response-sized RAM buffer exists: HTML remains a `static const` flash asset.

## Runtime behavior

- The custom WebGL fragment shader uses one full-screen triangle and one draw call.
- Device pixel ratio is capped at 1.35 for the ambient field.
- Drawing is capped at 30 FPS and browser `requestAnimationFrame` naturally pauses in a
  hidden document.
- Low-Motion cancels the recurring shader frame completely and redraws only on state change.
- Canvas fallback is automatic; a static CSS field remains if neither graphics context can
  be created.
- Chart data arrives at 2 Hz. Rendering is coalesced into a single animation frame and keeps
  at most 30 minutes of browser-local samples.
- Hardware inspection and technical values use the API data already received; they create no
  additional network traffic.

## Browser measurements

The first completed render exposes `data-ui-ready-ms`, measured from navigation start. These
figures use the local `file:` preview and include HTML parse, shader initialization, simulated
status normalization and the first full UI render; they do not pretend to measure hotspot
latency.

| Browser / scenario | Result |
|---|---:|
| Chrome, HEATING | 94.5 ms |
| Chrome, READY | 80.6 ms |
| Chrome, HOLDING | 89.3 ms |
| Chrome, ERROR | 94.9 ms |
| Chrome, DISCONNECT | 158.5 ms |
| Chrome, RECOVERY | 78.0 ms |
| Microsoft Edge, HEATING | 122.5 ms |

Chrome and Edge were executed headlessly against the final generated file. Android Chrome is
covered by the same Blink/WebGL implementation plus real 430×932 and 390×844 renders. Safari
was reviewed logically because no Safari runtime is available on Windows: the build uses
standard WebGL 1, Canvas 2D, SVG, Fetch/AbortController, requestAnimationFrame and modern CSS;
failure to obtain WebGL enters the tested fallback instead of leaving a black page.

## Firmware footprint

| Release artifact / section | Size |
|---|---:|
| ELF `text` (flash code + constants) | 460,124 B |
| ELF `data` | 0 B |
| ELF `bss` | 92,744 B |
| ELF total (`text + data + bss`) | 552,868 B |
| Raw `.bin` | 460,128 B |
| Flashable `.uf2` | 920,576 B |

The web redesign changes flash-resident constants and browser-side code; it introduces no
response-sized Pico allocation. Browser Canvas/WebGL/history memory exists on the viewing
phone or computer, not in RP2040 RAM. The UF2 remains at
`build/vscode/temperature_control.uf2`.
