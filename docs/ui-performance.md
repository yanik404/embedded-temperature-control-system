# V3 UI performance and embedded budget

Measurements were taken from the generated V3 files and a clean Pico W Release build on
13 August 2026. `tools/build_ui.py --check` is the reproducible source for asset figures.

## Budgets and result

| Metric | Budget | V3 result |
|---|---:|---:|
| Embedded single-file HTML | ≤ 128 KiB | 74,871 B |
| Local preview | ≤ 160 KiB | 82,787 B |
| Production CSS | ≤ 48 KiB | 31,654 B |
| Production JavaScript | ≤ 48 KiB | 31,633 B |
| Theoretical gzip size | reporting only | 21,632 B |
| External production libraries | 0 B | 0 B |
| Additional asset requests | 0 | 0 |
| Product-scene draw calls | 1 / frame | 1 / frame |
| Product / thermal frame cap | Preview 60, Pico 30 FPS | 60 / 30 FPS |
| Browser history horizon | 30 min | 30 min |

The Pico intentionally sends the 74,871-byte page uncompressed. At a conservative 1 Mbit/s
payload rate the raw transfer component is about 0.60 seconds. The existing asynchronous raw
lwIP sender reconstructs the response in 52 MSS-bounded writes in the constrained host model;
`TCP_SND_BUF` remains 29,200 bytes. No response-sized RAM buffer exists: HTML remains a
`static const` flash asset.

## Runtime behavior

- The custom SDF/WebGL product shader uses one full-screen triangle and one draw call.
- Product resolution is capped at DPR 0.82 on desktop and 1.0 below 1,000 CSS pixels; the sparse
  thermal overlay is capped at DPR 1.25.
- Product and thermal drawing target 60 FPS in the local preview and are independently capped
  at 30 FPS in the embedded page. `requestAnimationFrame` naturally pauses in hidden documents.
- Reduced motion freezes particles/parallax, applies exploded state directly and limits the
  scene refresh to 4 FPS for static state coherence.
- The SVG fallback is already in the DOM and becomes visible if WebGL creation/compilation fails.
- Chart data arrives at 2 Hz, is coalesced into animation frames and retains at most 30 minutes
  in browser memory.
- Preview simulation, Three.js and postprocessing addons are not present in production.

## Browser and responsive validation

Local Chromium DevTools Protocol captures use exact CSS device metrics rather than Windows'
minimum headless-window width. The final matrix contains 1920×1080, 1440×900, 1366×768,
1024×768, 768×1024, 430×932 and 390×844. Image dimensions were read back and matched all seven
targets. Overview, Thermal, Engineering, Presentation and ERROR were also captured separately.

Google Chrome and Microsoft Edge both rendered the generated single-file preview with WebGL,
Canvas, controls and charts. Safari cannot run on the Windows build host; it was reviewed
logically against standard WebGL 1, Canvas 2D, SVG, Fetch/AbortController, requestAnimationFrame
and CSS. A missing WebGL context enters the explicit SVG fallback rather than a black screen.

## Firmware footprint

| Release artifact / section | Size |
|---|---:|
| ELF `text` (flash code + constants) | 443,668 B |
| ELF `data` | 0 B |
| ELF `bss` | 92,744 B |
| ELF total (`text + data + bss`) | 536,412 B |
| Raw `.bin` | 443,672 B |
| Flashable `.uf2` | 887,808 B |

Browser Canvas/WebGL/history memory exists on the viewing phone or computer, not in RP2040 RAM.
The final artifact is `build/vscode/temperature_control.uf2`.
