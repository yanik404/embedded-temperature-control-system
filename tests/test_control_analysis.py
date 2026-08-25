from pathlib import Path


index = Path("ui-v3/src/index.html").read_text(encoding="utf-8")
css = Path("ui-v3/src/experience.css").read_text(encoding="utf-8")
script = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")

required_ids = (
    "controllerAnalysis",
    "analysisRegulatorState",
    "analysisError",
    "analysisP",
    "analysisI",
    "analysisRaw",
    "analysisOutput",
    "analysisLimitState",
    "analysisAntiWindup",
    "analysisCycle",
    "controllerTrendCanvas",
    "stepResponseCard",
    "stepRiseTime",
    "stepT63",
    "stepOvershoot",
    "stepSettlingTime",
    "stepSteadyError",
    "stepModelHint",
)
for element_id in required_ids:
    assert index.count(f'id="{element_id}"') == 1
    assert f'id="{element_id}"' in production

for token in (
    "REGLERANALYSE",
    "ECHTE FIRMWARE",
    "PT1 + Totzeit",
    "PT2",
    "P, PI und PID im Vergleich",
    "LERNMODELL · NICHT LIVE",
    "PID · NUR VERGLEICH",
    "1 Integrator",
    "Reglersignale über der Zeit",
    "Sprungantwort des Temperierlaufs",
    "Anstiegszeit t10–90",
    "T63 Orientierung",
    "für eine eindeutige Bestimmung",
):
    assert token in index

for element_id in required_ids[1:]:
    assert (
        f'text("{element_id}"' in script
        or element_id in ("analysisLimitState", "controllerTrendCanvas", "stepResponseCard")
    )
for meter_id in ("analysisPMeter", "analysisIMeter", "analysisOutputMeter"):
    assert f'meter("{meter_id}"' in script

assert ".control-analysis-grid" in css
assert ".controller-trend-card" in css and ".step-response-card" in css
assert "drawControllerTrend" in script
assert "stepResponseMetrics" in script
assert 'Math.abs(amplitude)<=.5' in script
assert "validController" in script
assert "body.presentation-mode .control-analysis{display:none!important}" in css
assert 'analysis:"controllerAnalysis"' in script
assert "/api/pid" not in production and "/api/controller" not in production

print("control analysis checks passed: live PI trend, step-response metrics, PT1/PT2 model and P/PI/PID comparison")
