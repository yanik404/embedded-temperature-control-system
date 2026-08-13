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

## Round 2 — pending

The second capture, persona evaluation and anti-generic-design audit are recorded here after
the responsive and production pipeline review.

## Round 3 — pending

The final product, presentation, performance and embedded-stability polish is recorded here
after the second screenshot set.
