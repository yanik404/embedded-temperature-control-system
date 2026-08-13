# Visual review log

The dashboard was rendered from the real `preview.html`, not from design mock-ups. The
screenshots live below `build/ui-review/` and remain deliberately untracked build evidence.
The seven reference sizes are 1920×1080, 1440×900, 1366×768, 1024×768, 768×1024,
430×932 and 390×844.

## Round 1 — spatial composition

Baseline: first complete **Thermal Product Observatory** implementation.

1. The cinematic intro covered the screenshot review state — a deterministic `?review=1`
   bypass was added without removing the real 1.35 s intro.
2. The local scenario console crossed the safety rail — preview builds now reserve their
   own lower spatial band; the Pico build contains neither controls nor that extra spacing.
3. The 430 px header expanded past the viewport due to intrinsic network text widths — its
   grid now uses explicit `minmax(0, …)` tracks and clipped live labels.
4. The mobile connection row could force horizontal scrolling — all live network children
   now have zero minimum width and ellipsis behavior.
5. The cup cutaway consumed too much vertical space on phones — the mobile assembly is
   independently scaled and shortened rather than merely stacking the desktop scene.
6. Long component annotations escaped the phone viewport — mobile keeps values while
   suppressing redundant labels already conveyed by the drawing.
7. The system state and setpoint control started too far below the first phone viewport —
   process spacing was tightened by 70 px.
8. The tablet/phone presentation assembly retained a desktop minimum height — presentation
   mode now has a dedicated mobile stage height.
9. The live chart legend did not match its actual plotted colors — actual, target,
   secondary sensor and power semantics now match exactly.
10. SVG heat contours stayed faint because the power custom property was never updated —
    every live render now drives `--thermal-power` from `u(t)`.
11. Fan motion used a mismatched custom-property name — tachometer/PWM data now drives the
    single shared fan rotation period.
12. Preview actions used the production fetch path — START, STOP and SETPOINT now route to
    the simulation driver only in preview; production still uses the Pico API.
13. Error-free safety states used CSS names different from the renderer — the rail now uses
    the intended `ok`/`bad` visual language.
14. Hardware inspection nodes used class names not styled by the engineering drawer — all
    19 nodes now share the explicit component state grammar.
15. Technical values used markup inconsistent with their layout rules — semantic rows and
    source styles now align without presentation-only DOM workarounds.

Round-one outcome: the desktop hero reads as one product/process scene, tablet retains a
two-focus composition, and mobile becomes a vertical technical narrative with no classic
card stack.

## Round 2 — product and persona review

The second complete render was followed by the five-persona and anti-generic-design audit in
`ui-persona-review.md`. Ten additional findings were implemented:

1. Short 1366×768 screens cut through the heater/fan assembly — hero, process orbit and
   rotary dimensions now react to viewport height as well as width.
2. German values were shown with decimal commas but the control used a decimal point — the
   input now presents a comma and accepts both comma and point.
3. Browser-native number spinners duplicated the authored ± interaction — the control is
   now a numeric touch text field with explicit bounds and accessible naming.
4. English chapter labels weakened the project identity — all main section markers are now
   concise German engineering language.
5. The graph began as an empty instrument in a local presentation — preview boot now seeds
   five minutes of physically plausible history.
6. The reduced-motion shader still scheduled empty animation frames — it now cancels the
   loop and renders only when live state changes.
7. Review captures included a developer scenario bar — deterministic review mode removes
   that bar without altering the normal preview.
8. Phone header live/IP tracks still relied on intrinsic text width — they now use explicit
   viewport-bounded flex and grid geometry.
9. Mobile annotations repeated physical labels and escaped the scene — only the current pair
   remains adjacent; sensor, fan and plate meaning is carried by the drawing and live block.
10. Preview URLs could not deterministically open a non-default state — `scenario=` now
    selects READY, HOLDING, ERROR, DISCONNECT, RECOVERY or the 30-minute run at startup.

The deliberate non-change is equally important: the mobile interface was not converted into
a tabbed card application. It remains a vertical spatial narrative with control immediately
after the product/state scene.

## Round 3 — final product polish

The third seven-viewport set was rendered after the persona pass. The final review found and
resolved ten product-level issues:

1. The narrow phone header still clipped the IPv4 value. Live network identity and IP are
   now two explicit rows, so the actual DHCP address remains readable at 390 px.
2. The fifth state marker could leave the phone viewport. The state spine now becomes a
   five-column instrument at phone width, guaranteeing OFF through ERROR stay visible.
3. The remaining mobile current annotation had lost its physical meaning after label
   reduction. A compact `PELTIER 1 / 2` label is retained next to the currents.
4. API strings entered several engineering rows through HTML templates. All dynamic values
   are now escaped before insertion, including SSID, IP and fault detail.
5. An open Engineering drawer left controls behind it keyboard-focusable. Main scene and
   header now become inert until the drawer is closed.
6. A WebGL compile failure after context creation could also deny a 2D context. The final
   fallback adds a static CSS thermal field even in that rare dual-context failure.
7. Presentation mode could preserve an arbitrary previous scroll position. It now returns
   to the product scene before entering the full-screen composition.
8. Presentation and engineering review states were difficult to reproduce automatically.
   Deterministic query switches now support screenshot/product inspection without changing
   the user-facing controls.
9. Deep-section screenshot positioning used an offset relative to `main`, not the document.
   It now resolves document coordinates from the live bounding rectangle.
10. No objective browser-ready marker existed. The first completed real/simulated render now
    records `data-ui-ready-ms`, allowing repeatable local load measurements without a
    framework or telemetry service.

### Final review questions

- **Visible beyond a normal dashboard?** Yes: the first viewport is one instrumented
  physical thermal system and permission horizon, not a card grid.
- **Credible in a high-end product?** Yes: state color and motion carry process meaning,
  controls are restrained, technical details disclose progressively, and failure/offline
  behavior is deliberate.
- **Strong on a projector?** Yes: presentation mode preserves only the large product scene,
  essential control values, signal chain and chart with high-contrast tabular numerals.
- **Stable as a Pico asset?** Confirmed by byte synchronization, 128 KiB asset budget,
  lwIP pbuf ownership test, 61-chunk large-response test and full Pico W link.
