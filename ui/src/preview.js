(function () {
  "use strict";

  document.body.classList.add("preview-mode");

  const nominal = {
    state: "AUFHEIZEN",
    fault: "KEIN FEHLER",
    fault_description: "Keine aktive Störung",
    temperature: 42.6,
    temperature1: 42.6,
    temperature2: 42.1,
    setpoint: 50,
    error: 7.4,
    power: 68,
    fan_rpm: 1850,
    fan_percent: 73,
    current1: 2.8,
    current2: 2.7,
    light_level: .71,
    sensor_ok: true,
    temp1_ok: true,
    temp2_ok: true,
    current_ok: true,
    current1_ok: true,
    current2_ok: true,
    tla2024_ok: true,
    light_ok: true,
    display_initialized: true,
    leds_initialized: true,
    cup: true,
    power_good: true,
    wifi: true,
    webserver_ready: true,
    wifi_ssid: "Obi W-lan Kenobi",
    wifi_ip: "10.78.252.72",
    night: false,
    start_allowed: false,
    start_block_reason: "Heizvorgang ist bereits aktiv",
    kp: 8,
    ki: .12,
    p_term: 59.2,
    i_term: 8.8,
    output_limited: false,
    anti_windup: false,
    control_period_ms: 250,
    max_safe_temperature: 65,
    uptime_ms: 267000
  };
  let demo = { ...nominal };
  let scenario = "heating";
  let automatic = true;
  let simulatedMinutes = 8.7;
  let previousTick = performance.now();

  function updateDerived() {
    demo.error = demo.sensor_ok ? demo.setpoint - demo.temperature : NaN;
    demo.temperature1 = demo.sensor_ok ? demo.temperature : NaN;
    if (demo.temp2_ok) demo.temperature2 = demo.temperature - .45 + Math.sin(simulatedMinutes * .7) * .08;
    demo.uptime_ms += 500;
    demo.p_term = demo.state === "AUFHEIZEN" || demo.state === "HALTEN" ? Math.max(0, demo.error * demo.kp) : 0;
    demo.i_term = Math.max(0, demo.power - demo.p_term);
    demo.fan_percent = demo.power ? 38 + demo.power * .52 : 0;
    demo.fan_rpm = demo.power ? Math.round(640 + demo.fan_percent * 16.2 + Math.sin(simulatedMinutes) * 18) : 0;
    demo.current1 = demo.power ? demo.power * .0412 : 0;
    demo.current2 = demo.power ? demo.power * .0397 : 0;
    demo.start_allowed = demo.state === "BEREIT" && demo.cup && demo.power_good && demo.sensor_ok;
    demo.start_block_reason = demo.start_allowed ? "Alle Sicherheitsbedingungen erfüllt" : demo.state === "AUFHEIZEN" || demo.state === "HALTEN" ? "Heizprozess ist bereits aktiv" : demo.fault_description;
  }

  function applyScenario(name) {
    scenario = name;
    automatic = name === "demo30";
    Object.assign(demo, nominal);
    if (name === "ready") Object.assign(demo, { state: "BEREIT", temperature: 24.2, temperature1: 24.2, temperature2: 23.8, power: 0, fan_rpm: 0, fan_percent: 0, current1: 0, current2: 0, start_allowed: true, start_block_reason: "Alle Sicherheitsbedingungen erfüllt" });
    if (name === "holding") Object.assign(demo, { state: "HALTEN", temperature: 49.9, temperature1: 49.9, temperature2: 49.5, error: .1, power: 23, fan_rpm: 1190, fan_percent: 43, current1: .94, current2: .91, p_term: .8, i_term: 22.2 });
    if (name === "error") Object.assign(demo, { state: "FEHLER", fault: "TEMPERATURSENSOR", fault_description: "Temperatursensor 1 nicht verfügbar", temperature: NaN, temperature1: NaN, temperature2: 41.8, error: NaN, power: 0, fan_rpm: 0, fan_percent: 0, current1: 0, current2: 0, sensor_ok: false, temp1_ok: false, start_allowed: false, start_block_reason: "Temperatursensor nicht verfügbar" });
    if (name === "disconnected") Object.assign(demo, { state: "BEREIT", power: 0, wifi: false, webserver_ready: false, wifi_ip: "—", start_allowed: false });
    if (name === "recovery") Object.assign(demo, { state: "BEREIT", temperature: 31.8, temperature1: 31.8, temperature2: 31.4, power: 0, start_allowed: true, start_block_reason: "Verbindung wiederhergestellt · Start freigegeben" });
    if (name === "demo30") {
      simulatedMinutes = 0;
      Object.assign(demo, { state: "BEREIT", temperature: 23.7, temperature1: 23.7, temperature2: 23.4, setpoint: 50, power: 0, fan_rpm: 0, current1: 0, current2: 0, start_allowed: true });
      if (window.ThermalUI) window.ThermalUI.history.splice(0);
    }
    updateDerived();
    document.querySelectorAll("[data-scenario]").forEach(button => button.classList.toggle("active", button.dataset.scenario === name));
    if (window.ThermalUI) {
      window.ThermalUI.addEvent(`Szenario ${name.toUpperCase()} aktiviert`, name === "error" || name === "disconnected" ? "bad" : "info");
      window.ThermalUI.acceptStatus(demo, name !== "disconnected");
    }
  }

  function simulateThirtyMinutes(deltaSeconds) {
    simulatedMinutes += deltaSeconds * .13;
    if (simulatedMinutes < .7) {
      demo.state = "BEREIT"; demo.power = 0;
    } else if (simulatedMinutes < 22) {
      demo.state = "AUFHEIZEN";
      const progress = (simulatedMinutes - .7) / 21.3;
      const targetTemperature = 23.7 + 26.5 * (1 - Math.exp(-progress * 3.15));
      demo.temperature += (targetTemperature - demo.temperature) * .065;
      demo.power = Math.max(24, 96 - progress * 83 + Math.sin(simulatedMinutes * .5) * 1.5);
    } else {
      demo.state = "HALTEN";
      demo.temperature = 50 + Math.sin(simulatedMinutes * 1.3) * .14 - Math.cos(simulatedMinutes * .43) * .05;
      demo.power = 21.5 + Math.sin(simulatedMinutes * .72) * 2.4;
    }
    if (simulatedMinutes >= 30) simulatedMinutes = 22;
  }

  function tick(now) {
    const deltaSeconds = Math.min(1, (now - previousTick) / 1000);
    previousTick = now;
    if (automatic) simulateThirtyMinutes(deltaSeconds);
    else if (scenario === "heating") {
      simulatedMinutes += deltaSeconds / 60;
      demo.temperature += Math.max(0, demo.setpoint - demo.temperature) * .00042 + Math.sin(now * .001) * .0005;
    } else if (scenario === "holding") {
      demo.temperature = demo.setpoint - .08 + Math.sin(now * .0007) * .08;
      demo.power = 23 + Math.sin(now * .00045) * 1.7;
    }
    updateDerived();
    if (window.ThermalUI) window.ThermalUI.acceptStatus(demo, scenario !== "disconnected");
    setTimeout(() => requestAnimationFrame(tick), 440);
  }

  function command(path) {
    if (path === "start") {
      if (demo.start_allowed) {
        scenario = "heating"; automatic = false; demo.state = "AUFHEIZEN"; demo.power = 92;
        window.ThermalUI.addEvent("Demo-START bestätigt · Heizsimulation aktiv", "good");
      }
    } else if (path === "stop") {
      scenario = "ready"; automatic = false; demo.state = "BEREIT"; demo.power = 0;
      window.ThermalUI.addEvent("Demo-STOP bestätigt · Leistung sicher aus", "warn");
    } else if (path.startsWith("setpoint?value=")) {
      demo.setpoint = Math.max(20, Math.min(60, Number(new URLSearchParams(path.split("?")[1]).get("value")) || demo.setpoint));
      window.ThermalUI.addEvent(`Demo-Sollwert ${demo.setpoint.toFixed(1).replace(".", ",")} °C`, "good");
    }
    updateDerived();
    window.ThermalUI.acceptStatus(demo, true);
  }

  window.PreviewDriver = {
    start() {
      document.querySelectorAll("[data-scenario]").forEach(button => button.addEventListener("click", () => applyScenario(button.dataset.scenario)));
      updateDerived();
      if (window.ThermalUI) window.ThermalUI.acceptStatus(demo, true);
      requestAnimationFrame(tick);
    },
    command,
    applyScenario
  };
}());
